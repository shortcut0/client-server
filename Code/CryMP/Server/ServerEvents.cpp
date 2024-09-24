#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CryEntitySystem/IEntity.h"

#include "CryGame/Items/Weapons/Weapon.h"
#include "CryGame/GameRules.h"
#include "CryGame/Items/Weapons/WeaponSystem.h"
#include "CryGame/Items/Weapons/Projectile.h"

#include "ServerEvents.h"
#include "Server.h"

// --------------------------------------------------------
ServerEvents::ServerEvents()
{
	m_pSS = gEnv->pScriptSystem;
}

// --------------------------------------------------------
ServerEvents::~ServerEvents()
{
	for (HSCRIPTFUNCTION handler : m_handlers)
	{
		if (handler)
			m_pSS->ReleaseFunc(handler);
	}
}


// --------------------------------------------------------
std::pair<SmartScriptTable, SmartScriptFunction> ServerEvents::GetFunc(const std::string& handle)
{

	HSCRIPTFUNCTION pFunc_r = nullptr;
	SmartScriptTable pHost = nullptr;

	// ------------
	std::string delims(".");
	std::vector<std::string> tokens = SplitString(handle, *delims.c_str());
	int c(0);

	for (const auto& token : tokens) {

		c += 1;

		if (c >= tokens.size()) {
			if (c == 1) {
				if (!m_pSS->GetGlobalValue(token.c_str(), pFunc_r)) {
					gServer->LogError("ServerEvents::GetFunc Function %s Not found at index %d, Input Host was %s", token.c_str(), c, handle.c_str());
				}
			}
			else
			{
				if (pHost && !pHost->GetValue(token.c_str(), pFunc_r)) {
					gServer->LogError("ServerEvents::GetFunc Function %s Not found at index %d, Input Host was %s", token.c_str(), c, handle.c_str());
				}
			}
			break;
		}

		if (c == 1) {
			m_pSS->GetGlobalValue(token.c_str(), pHost);
			if (!pHost) {
				gServer->LogError("ServerEvents::GetFunc Global Host %s not found", token.c_str());
				break;
			}
			continue;
		}

		if (pHost && !pHost->GetValue(token.c_str(), pHost)) {
			pHost = nullptr;
			//pFunc = nullptr;
			gServer->LogError("ServerEvents::GetFunc Host Array %s not found at index %d, Input Host was %s", token.c_str(), c, handle.c_str());
			break;
		}
	}

	//if (pFunc_r != nullptr)
	//	pFunc = pFunc_r;

	//SmartScriptFunction smartFunc(m_pSS, pFunc_r);
	//return std::make_pair(pHost, smartFunc);
	return std::make_pair(pHost, SmartScriptFunction(m_pSS, pFunc_r));
}


// --------------------------------------------------------
bool ServerEvents::SetHandler(EServerEvents callback, HSCRIPTFUNCTION handler)
{
	if (callback < 0 || callback >= SV_EVENT_COUNT)
	{
		// invalid callback
		return false;
	}

	if (m_handlers[callback])
	{
		// handlers cannot be replaced
		return false;
	}

	m_handlers[callback] = handler;

	return true;
}

// --------------------------------------------------------
void ServerEvents::OnUpdate(float deltaTime)
{
	Call(SV_EVENT_ON_UPDATE, deltaTime);
}

// --------------------------------------------------------
void ServerEvents::OnSpawn(IEntity* pEntity)
{
	IScriptTable* pScript = pEntity->GetScriptTable();
	if (pScript)
	{
		Call(SV_EVENT_ON_SPAWN, pScript);
	}
}

// --------------------------------------------------------
void ServerEvents::OnGameRulesCreated(EntityId gameRulesId)
{
	ScriptHandle id;
	id.n = gameRulesId;
	Call(SV_EVENT_ON_GAME_RULES_CREATED, id);
}

// --------------------------------------------------------
// --------------------------------------------------------
// --------------------------------------------------------
// Weapon Listener
void ServerEvents::OnShoot(IWeapon* pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3& pos, const Vec3& dir, const Vec3& vel) {
	/*
	IScriptTable* pWeaponTable;
	IScriptTable* pShooterTable;
	const char* sAmmoClass = "";

	CWeapon* pWpn = static_cast<CWeapon*>(pWeapon);
	if (pWpn)
		pWeaponTable = pWpn->GetEntity()->GetScriptTable();

	IEntity* pShooter = gEnv->pEntitySystem->GetEntity(shooterId);
	if (pShooter)
		pShooterTable = pShooter->GetScriptTable();

	if (pAmmoType)
		sAmmoClass = pAmmoType->GetName();

	if (CWeaponSystem* pWeaponSystem = g_pGame->GetWeaponSystem()) {
		if (CProjectile* pProj = pWeaponSystem->GetProjectile(ammoId)) {
		}
	}

	Call("ServerRPC.Callbacks.OnShoot", pShooterTable, pWeaponTable, sAmmoClass, pos, pWeapon->GetDestination(), dir, vel);
	*/
}



void ServerEvents::OnStartFire(IWeapon* pWeapon, EntityId shooterId) {

}


void ServerEvents::OnStopFire(IWeapon* pWeapon, EntityId shooterId) {

}


void ServerEvents::OnStartReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {
	if (CWeapon* pWpn = static_cast<CWeapon*>(pWeapon)) {
		EntityId weaponId = pWpn->GetEntityId();
		Call("ServerRPC.Callbacks.OnStartReload", ScriptHandle(shooterId), ScriptHandle(weaponId));
	}
}


void ServerEvents::OnEndReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {
	if (CWeapon* pWpn = static_cast<CWeapon*>(pWeapon)) {
		EntityId weaponId = pWpn->GetEntityId();
		Call("ServerRPC.Callbacks.OnEndReload", ScriptHandle(shooterId), ScriptHandle(weaponId));
	}

}


void ServerEvents::OnOutOfAmmo(IWeapon* pWeapon, IEntityClass* pAmmoType) {

}


void ServerEvents::OnReadyToFire(IWeapon* pWeapon) {

}


void ServerEvents::OnPickedUp(IWeapon* pWeapon, EntityId actorId, bool destroyed) {

	/*
	IScriptTable* pWeaponTbl = 0;
	IScriptTable* pActorTbl = 0;
	const char* sAmmoClass = "";

	CWeapon* pWpn = static_cast<CWeapon*>(pWeapon);
	if (pWpn)
		pWeaponTbl = pWpn->GetEntity()->GetScriptTable();

	IEntity* pShooter = gEnv->pEntitySystem->GetEntity(actorId);
	if (pShooter)
		pActorTbl = pShooter->GetScriptTable();
		*/
	//Call("ServerRPC.Callbacks.OnWeaponPickedUp", pWeaponTbl, pActorTbl, destroyed);
}


void ServerEvents::OnDropped(IWeapon* pWeapon, EntityId actorId) {

}


void ServerEvents::OnMelee(IWeapon* pWeapon, EntityId shooterId) {

	if (CWeapon* pWpn = static_cast<CWeapon*>(pWeapon)) {
		EntityId weaponId = pWpn->GetEntityId();
		Call("ServerRPC.Callbacks.OnMelee", ScriptHandle(shooterId), ScriptHandle(weaponId));
	}
}


void ServerEvents::OnStartTargetting(IWeapon* pWeapon) {

}


void ServerEvents::OnStopTargetting(IWeapon* pWeapon) {

}


void ServerEvents::OnSelected(IWeapon* pWeapon, bool selected) {

}