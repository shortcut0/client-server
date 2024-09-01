// -------------------------------------------------------------------------
// Crytek Source File.
// Copyright (C) Crytek GmbH, 2001-2008.
// -------------------------------------------------------------------------
#include "CryCommon/CrySystem/ISystem.h"
#include "NetPlayerInput.h"
#include "Player.h"
#include "CryGame/Game.h"
#include "CryGame/GameCVars.h"

// Server
#include "CryMP/Server/ServerCVars.h"

CNetPlayerInput::CNetPlayerInput(CPlayer* pPlayer) : m_pPlayer(pPlayer)
{
}

void CNetPlayerInput::PreUpdate()
{
	/*
	CryLogAlways("m_curInput.bodyDirection = {x=%f, y=%f, z=%f}", m_curInput.bodyDirection.x, m_curInput.bodyDirection.y, m_curInput.bodyDirection.z);
	CryLogAlways("m_curInput.deltaMovement = {x=%f, y=%f, z=%f}", m_curInput.deltaMovement.x, m_curInput.deltaMovement.y, m_curInput.deltaMovement.z);
	CryLogAlways("m_curInput.lookDirection = {x=%f, y=%f, z=%f}", m_curInput.lookDirection.x, m_curInput.lookDirection.y, m_curInput.lookDirection.z);
	CryLogAlways("m_curInput.sprint        = %s", m_curInput.sprint ? "true" : "false");
	CryLogAlways("m_curInput.leanl         = %s", m_curInput.leanl ? "true" : "false");
	CryLogAlways("m_curInput.leanr         = %s", m_curInput.leanr ? "true" : "false");*/

	//if (true) return;


	const IPhysicalEntity *pPhysEnt = m_pPlayer->GetEntity()->GetPhysics();
	if (!pPhysEnt)
		return;

	CMovementRequest moveRequest;
	SMovementState moveState;
	m_pPlayer->GetMovementController()->GetMovementState(moveState);
	Quat worldRot = m_pPlayer->GetBaseQuat(); 
	Vec3 deltaMovement = worldRot.GetInverted().GetNormalized() * m_curInput.deltaMovement;
	// absolutely ensure length is correct
	deltaMovement = deltaMovement.GetNormalizedSafe(ZERO) * m_curInput.deltaMovement.GetLength();
	moveRequest.AddDeltaMovement(deltaMovement);

	Vec3 lookDirection(m_curInput.lookDirection);
	m_pPlayer->m_netAimDir = lookDirection;

	// --------------------------------------------------
	//CryMP: FP Spectator uses interpolated lookDirection
	if (m_pPlayer->IsFpSpectatorTarget())
	{
		lookDirection = m_pPlayer->GetNetAimDirSmooth();
	}

	const Vec3 distantTarget = moveState.eyePosition + 1000.0f * lookDirection;
	moveRequest.SetLookTarget(distantTarget);
	moveRequest.SetAimTarget(distantTarget);

	if (m_curInput.deltaMovement.GetLengthSquared() > sqr(0.02f)) // 0.2f is almost stopped
		moveRequest.SetBodyTarget(distantTarget);
	else
		moveRequest.ClearBodyTarget();

	moveRequest.SetAllowStrafing(true);

	float pseudoSpeed = 0.0f;
	if (m_curInput.deltaMovement.len2() > 0.0f)
	{
		pseudoSpeed = m_pPlayer->CalculatePseudoSpeed(m_curInput.sprint);
	}
	moveRequest.SetPseudoSpeed(pseudoSpeed);

	float lean = 0.0f;
	if (m_curInput.leanl)
		lean -= 1.0f;
	if (m_curInput.leanr)
		lean += 1.0f;

	if (fabsf(lean) > 0.01f)
		moveRequest.SetLean(lean);
	else
		moveRequest.ClearLean();
	/*
	moveRequest.ClearActorTarget();
	moveRequest.SetDesiredSpeed(0.f);
	moveRequest.SetMoveTarget(Vec3(0, 0, 0));
	moveRequest.ClearMoveTarget();
	moveRequest.ClearDesiredSpeed();
	moveRequest.ClearFireTarget();
	moveRequest.ClearJump();
	moveRequest.ClearActorTarget();
	moveRequest.ClearAimTarget();
	moveRequest.ClearBodyTarget();	
	*/

	if (m_curInput.sprint)
		m_pPlayer->m_actions |= ACTION_SPRINT;
	else
		m_pPlayer->m_actions &= ~ACTION_SPRINT;

	if (m_curInput.leanl)
		m_pPlayer->m_actions |= ACTION_LEANLEFT;
	else
		m_pPlayer->m_actions &= ~ACTION_LEANLEFT;

	if (m_curInput.leanr)
		m_pPlayer->m_actions |= ACTION_LEANRIGHT;
	else
		m_pPlayer->m_actions &= ~ACTION_LEANRIGHT;


	// Server: this somehow for some reason "resets" movement to register that the player is no longer moving!
	// Esentially fixing the bug where the server wont recognize when the player stops moving while in spectator mode..
	if (g_pServerCVars->server_fix_spectatorDesync > 0 && m_pPlayer && m_pPlayer->GetSpectatorMode() > 0) {

		bool moving = m_curInput.deltaMovement.GetLength() > 0;
		if (moving && m_moveReset) {
			//CryLogAlways("move start..");
			m_moveReset = false;
		}

		
		if (!moving) {

			if (!m_moveReset) {
				Vec3 o_look = m_curInput.lookDirection;
				Vec3 t_look = m_curInput.lookDirection;
				t_look.x = o_look.x + 0.1;
				t_look.y = o_look.y + 0.1;
				t_look.z = o_look.z + 0.1;

				CMovementRequest mr;
				mr.AddDeltaMovement(Vec3(0, 0, 0));
				mr.SetLookTarget(t_look);
				mr.SetAimTarget(t_look);
				m_pPlayer->GetMovementController()->RequestMovement(mr);
				//mr.SetLookTarget(o_look);
				//mr.SetAimTarget(o_look);
				//m_pPlayer->GetMovementController()->RequestMovement(mr);

				m_moveReset = true;
				m_spectatorPos = m_pPlayer->GetEntity()->GetWorldPos() + deltaMovement * max(1.f, pseudoSpeed);

				//CryLogAlways("move reset..");
			}
			else {


				if (m_spectatorPos.GetDistance(m_pPlayer->GetEntity()->GetWorldPos()) > g_pServerCVars->server_spectatorFix_ResetThreshold) {
					m_spectatorPos = m_pPlayer->GetEntity()->GetWorldPos();
				}

				Quat rot = m_pPlayer->GetViewRotation();
				m_pPlayer->GetEntity()->SetWorldTM(Matrix34::Create(Vec3(m_spectatorPos), rot, m_spectatorPos));


				// Experimentals..

				moveRequest.ClearActorTarget();
				moveRequest.SetDesiredSpeed(0.f);
				moveRequest.SetMoveTarget(Vec3(0, 0, 0));
				moveRequest.ClearMoveTarget();
				moveRequest.ClearDesiredSpeed();
				moveRequest.ClearFireTarget();
				moveRequest.ClearJump();
				moveRequest.ClearActorTarget();
				moveRequest.ClearAimTarget();
				moveRequest.ClearBodyTarget();
			}

			/*
				CryLogAlways("m_curInput.bodyDirection = {x=%f, y=%f, z=%f}", m_curInput.bodyDirection.x, m_curInput.bodyDirection.y, m_curInput.bodyDirection.z);
				CryLogAlways("m_curInput.deltaMovement = {x=%f, y=%f, z=%f}", m_curInput.deltaMovement.x, m_curInput.deltaMovement.y, m_curInput.deltaMovement.z);
				CryLogAlways("m_curInput.lookDirection = {x=%f, y=%f, z=%f}", m_curInput.lookDirection.x, m_curInput.lookDirection.y, m_curInput.lookDirection.z);
				CryLogAlways("m_curInput.sprint        = %s", m_curInput.sprint ? "true" : "false");
				CryLogAlways("m_curInput.leanl         = %s", m_curInput.leanl ? "true" : "false");
				CryLogAlways("m_curInput.leanr         = %s", m_curInput.leanr ? "true" : "false");
				*/
		}
	}




	m_pPlayer->GetMovementController()->RequestMovement(moveRequest);
	m_deltaLast = deltaMovement;
}

void CNetPlayerInput::Update()
{
	if (gEnv->bServer && (g_pGameCVars->sv_input_timeout > 0) && ((gEnv->pTimer->GetFrameStartTime() - m_lastUpdate).GetMilliSeconds() >= g_pGameCVars->sv_input_timeout))
	{
		m_curInput.deltaMovement.zero();
		m_curInput.sprint = m_curInput.leanl = m_curInput.leanr = false;
		m_curInput.stance = STANCE_NULL;

		m_pPlayer->GetGameObject()->ChangedNetworkState(INPUT_ASPECT);
	}
	/*
	// Server test
	if (m_curInput.deltaMovement.GetLength() == 0) {
		if (m_pPlayer)
		{
			CMovementRequest mr;
			mr.ClearLookTarget();
			m_pPlayer->GetMovementController()->RequestMovement(mr);

			CryLogAlways("stop.!");
		}
	}*/

	if (m_pPlayer)
		m_pPlayer->m_netLookDirection = m_curInput.lookDirection;

}

void CNetPlayerInput::PostUpdate()
{
}

void CNetPlayerInput::SetState(const SSerializedPlayerInput& input)
{
	DoSetState(input);

	m_lastUpdate = gEnv->pTimer->GetCurrTime();
}

void CNetPlayerInput::GetState(SSerializedPlayerInput& input)
{
	input = m_curInput;
}

void CNetPlayerInput::Reset()
{
	SSerializedPlayerInput i(m_curInput);
	i.leanl = i.leanr = i.sprint = false;
	i.deltaMovement.zero();

	DoSetState(i);

	m_pPlayer->GetGameObject()->ChangedNetworkState(IPlayerInput::INPUT_ASPECT);
}

void CNetPlayerInput::DisableXI(bool disabled)
{
}


void CNetPlayerInput::DoSetState(const SSerializedPlayerInput& input)
{

	//CryLogAlways("test > ", input.deltaMovement.x, input.deltaMovement.y, input.deltaMovement.z);

	m_curInput = input;
	m_pPlayer->GetGameObject()->ChangedNetworkState(INPUT_ASPECT);

	CMovementRequest moveRequest;
	moveRequest.SetStance((EStance)m_curInput.stance);

	if (IsDemoPlayback())
	{
		Vec3 localVDir(m_pPlayer->GetViewQuatFinal().GetInverted() * m_curInput.lookDirection);
		Ang3 deltaAngles(asin(localVDir.z), 0, cry_atan2f(-localVDir.x, localVDir.y));
		moveRequest.AddDeltaRotation(deltaAngles * gEnv->pTimer->GetFrameTime());
	}
	//else
	{
		moveRequest.SetLookTarget(m_pPlayer->GetEntity()->GetWorldPos() + 10.0f * m_curInput.lookDirection);
		moveRequest.SetAimTarget(moveRequest.GetLookTarget());
	}

	float pseudoSpeed = 0.0f;
	if (m_curInput.deltaMovement.len2() > 0.0f)
	{
		pseudoSpeed = m_pPlayer->CalculatePseudoSpeed(m_curInput.sprint);
	}
	moveRequest.SetPseudoSpeed(pseudoSpeed);
	moveRequest.SetAllowStrafing(true);

	float lean = 0.0f;
	if (m_curInput.leanl)
		lean -= 1.0f;
	if (m_curInput.leanr)
		lean += 1.0f;
	moveRequest.SetLean(lean);

	m_pPlayer->GetMovementController()->RequestMovement(moveRequest);

	// debug..
	if (g_pGameCVars->g_debugNetPlayerInput & 1)
	{
		IPersistantDebug* pPD = gEnv->pGame->GetIGameFramework()->GetIPersistantDebug();
		pPD->Begin((string("net_player_input_") + m_pPlayer->GetEntity()->GetName()).c_str(), true);
		pPD->AddSphere(moveRequest.GetLookTarget(), 0.5f, ColorF(1, 0, 1, 1), 1.0f);
		//			pPD->AddSphere( moveRequest.GetMoveTarget(), 0.5f, ColorF(1,1,0,1), 1.0f );

		Vec3 wp(m_pPlayer->GetEntity()->GetWorldPos() + Vec3(0, 0, 2));
		pPD->AddDirection(wp, 1.5f, m_curInput.deltaMovement, ColorF(1, 0, 0, 1), 1.0f);
		pPD->AddDirection(wp, 1.5f, m_curInput.lookDirection, ColorF(0, 1, 0, 1), 1.0f);
	}
}
