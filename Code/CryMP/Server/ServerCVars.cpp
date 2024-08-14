/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$

 -------------------------------------------------------------------------
  History:
  - 11:8:2004   10:50 : Created by Márcio Martins

*************************************************************************/
#include <cstring>

#include "ServerCVars.h"
#include "ServerCVarCallbacks.h"

ServerCVars::ServerCVars()
{
	std::memset(this, 0, sizeof(ServerCVars));
}

ServerCVars::~ServerCVars()
{
	this->ReleaseCVars();
}

void ServerCVars::InitCVars(IConsole* pConsole)
{
	
	// ---------------------------------------------------------
	// Callbacks


	// ----------------
	if (ICVar* pMaxPlayers = pConsole->GetCVar("sv_maxPlayers"))
		pMaxPlayers->SetOnChangeCallback(CCALLBACK_MaxPlayers);


	// ----------------
	if (ICVar* pServerName = pConsole->GetCVar("sv_serverName"))
		pServerName->SetOnChangeCallback(CCALLBACK_ServerName);


	// ---------------- doesnt work!!
	//if (ICVar* pMapCommand = pConsole->GetCVar("map"))
	//	pMapCommand->SetOnChangeCallback(CCALLBACK_Map);

	// ...
	// ---------------------------------------------------------
	

	// Flags
	const int OPTIONAL_SYNC = 0;

	// ---------------
	// Server CVars 
	pConsole->Register("server_ghostbug_fix", &server_ghostbug_fix, 1, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_use_hit_queue", &server_use_hit_queue, 0, 0, "Enable/Disable the hit queue");
	pConsole->Register("server_use_explosion_queue", &server_use_explosion_queue, 1, 0, "Enable/Disable the explosion queue");
	pConsole->Register("server_classic_chat", &server_classic_chat, 0, 0, "Enable/Disable the default chat system");

	// Lag Fix
	pConsole->Register("server_lag_resetmovement", &server_lag_resetmovement, 0, 0, "Enables/Disables Resetting movement on network lag");
	
	// ---------------
	// Commands
	server_mapTitle = pConsole->RegisterString("server_maptitle", "", 0, "Server Map Title");

	// ---------------
	// CryMP CVars 
	/*
	pConsole->Register("mp_crymp", &mp_crymp, 0, OPTIONAL_SYNC, "Enable high precision look direction serialization");
	pConsole->Register("mp_circleJump", &mp_circleJump, 0.0f, OPTIONAL_SYNC, "Enable circle jumping as in 5767");
	pConsole->Register("mp_wallJump", &mp_wallJump, 1.0f, OPTIONAL_SYNC, "WallJump multiplier");
	pConsole->Register("mp_flyMode", &mp_flyMode, 0, OPTIONAL_SYNC, "Enable FlyMode", NULL); // OnChangeFlyMode
	pConsole->Register("mp_pickupObjects", &mp_pickupObjects, 0, OPTIONAL_SYNC, "Allow pickup and throw objects in DX10");
	pConsole->Register("mp_pickupVehicles", &mp_pickupVehicles, 0, OPTIONAL_SYNC, "Allow pickup and throw vehicles (requires mp_pickupObjects 1)");
	pConsole->Register("mp_weaponsOnBack", &mp_weaponsOnBack, 0, OPTIONAL_SYNC, "Attach weapons to back as in SP");
	pConsole->Register("mp_thirdPerson", &mp_thirdPerson, 1, OPTIONAL_SYNC, "Allow ThirdPerson mode (F1)", NULL); // OnChangeThirdPerson
	pConsole->Register("mp_animationGrenadeSwitch", &mp_animationGrenadeSwitch, 0, OPTIONAL_SYNC, "Enable FP animations for grenade switching");
	pConsole->Register("mp_ragdollUnrestricted", &mp_ragdollUnrestricted, 1, OPTIONAL_SYNC);
	pConsole->Register("mp_killMessages", &mp_killMessages, 1, OPTIONAL_SYNC);
	pConsole->Register("mp_rpgMod", &mp_rpgMod, 0, OPTIONAL_SYNC);
	pConsole->Register("mp_aaLockOn", &mp_aaLockOn, 0, OPTIONAL_SYNC, "enables lockon air for AARocketLauncher");
	pConsole->Register("mp_C4StrengthThrowMult", &mp_C4StrengthThrowMult, 1.0f, OPTIONAL_SYNC, "Strength throw mult for C4s");

	//CryMP CVars (un-synced)
	pConsole->Register("mp_newSpectator", &mp_newSpectator, 1, VF_NOT_NET_SYNCED, "");
	pConsole->Register("mp_usePostProcessAimDir", &mp_usePostProcessAimDir, 1, VF_NOT_NET_SYNCED, "");
	pConsole->Register("mp_messageCenterColor", &mp_messageCenterColor, 1, VF_NOT_NET_SYNCED);
	pConsole->Register("mp_animationWeaponMult", &mp_animationWeaponMult, 1.5f, VF_NOT_NET_SYNCED);
	pConsole->Register("mp_animationWeaponMultSpeed", &mp_animationWeaponMultSpeed, 3.0f, VF_NOT_NET_SYNCED);
	pConsole->Register("mp_animationModelMult", &mp_animationModelMult, 1.0f, VF_NOT_NET_SYNCED);
	pConsole->Register("mp_animationModelMultSpeed", &mp_animationModelMultSpeed, 1.0f, VF_NOT_NET_SYNCED);
	pConsole->Register("mp_menuSpeed", &mp_menuSpeed, 3.0f, VF_NOT_NET_SYNCED);
	pConsole->Register("mp_hitIndicator", &mp_hitIndicator, 1, VF_NOT_NET_SYNCED, "Enables hit indicator from Wars");
	pConsole->Register("mp_chatHighResolution", &mp_chatHighResolution, 0, VF_NOT_NET_SYNCED);
	pConsole->Register("mp_spectatorSlowMult", &mp_spectatorSlowMult, 0.15f, VF_NOT_NET_SYNCED, "Speed mult for spectating while holding Ctrl");
	pConsole->Register("mp_buyPageKeepTime", &mp_buyPageKeepTime, 30, VF_NOT_NET_SYNCED, "The time in sec it will remember your last buy page");
	pConsole->Register("mp_attachBoughtEquipment", &mp_attachBoughtEquipment, 0, VF_NOT_NET_SYNCED, "Automatically attach bought weapon attachments");
	*/
	//pConsole->Register("mp_netAimLerpFactor", &mp_netAimLerpFactor, 20.f, VF_NOT_NET_SYNCED/*VF_CHEAT*/, "set aim smoothing for other clients (1-50, 0:off)");
	//pConsole->Register("mp_netAimLerpFactorCrymp", &mp_netAimLerpFactorCrymp, 42.f, VF_NOT_NET_SYNCED/*VF_CHEAT*/, "set aim smoothing for other clients when mp_crymp 1 (1-50, 0:off)");
	//pConsole->Register("mp_explosiveSilhouettes", &mp_explosiveSilhouettes, 0, VF_NOT_NET_SYNCED/*VF_CHEAT*/, "enables new indicators for explosives");
	
}

//------------------------------------------------------------------------
void ServerCVars::ReleaseCVars()
{
	// ------------
	IConsole* pConsole = gEnv->pConsole;

	// ------------
	pConsole->UnregisterVariable("server_use_hit_queue", true);
	pConsole->UnregisterVariable("server_use_explosion_queue", true);
}
