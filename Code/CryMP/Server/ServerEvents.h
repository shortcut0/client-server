#pragma once
/*
#include <map>
#include <vector>*/

#include <regex>
#include <array>

#include <iostream>
#include <sstream>  // Include this header for stringstream
#include <vector>
#include <string>
#include <map>

//#include "CryCommon/CryAction/IWeapon.h"
#include "CryGame/Items/Weapons/Weapon.h"
#include "CryCommon/CryScriptSystem/IScriptSystem.h"


struct IEntity;
enum EServerEvents
{
	SV_EVENT_ON_UPDATE,
	SV_EVENT_ON_DISCONNECT,
	SV_EVENT_ON_SPAWN,
	SV_EVENT_ON_LOADING_START,
	SV_EVENT_ON_GAME_RULES_CREATED,

	// must be last
	SV_EVENT_COUNT
};

class ServerEvents : public IWeaponEventListener
{
	IScriptSystem* m_pSS = nullptr;
	std::array<HSCRIPTFUNCTION, SV_EVENT_COUNT> m_handlers = {};

	template<class... Params>
	void Call(EServerEvents callback, const Params &... params)
	{
		HSCRIPTFUNCTION handler = m_handlers[callback];

		if (handler && m_pSS->BeginCall(handler))
		{
			(m_pSS->PushFuncParam(params), ...);
			m_pSS->EndCall();
		}
	}

	// --------------------
	std::pair<SmartScriptTable, HSCRIPTFUNCTION> GetFunc(const std::string& host);
	
private:

	// --------------------
	bool Regexp(const std::string& itemName, const std::string& filter) {
		std::regex pattern(filter);
		return std::regex_match(itemName, pattern);
	}

	// --------------------
	std::vector<std::string> SplitString(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);
		std::string token;

		while (std::getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}

		return tokens;
	}

public:

	ServerEvents();
	~ServerEvents();

	bool SetHandler(EServerEvents callback, HSCRIPTFUNCTION handler);

	void OnUpdate(float deltaTime);
	void OnSpawn(IEntity* pEntity);
	void OnGameRulesCreated(EntityId gameRulesId);

	// Client Stuff
	//void OnBecomeLocalActor(EntityId localActorId);
	//void OnLoadingStart();
	//void OnDisconnect(int reason, const char* message);
	//void OnMasterResolved();

	template<class... Params>
	bool Call(HSCRIPTFUNCTION handle, const Params &... params)
	{
		if (handle && m_pSS->BeginCall(handle))
		{
			(m_pSS->PushFuncParam(params), ...);
			m_pSS->EndCall();
		}
		else
			return false;

		return true;
	}

	template<class Ret, class... Params>
	bool Get(const std::string& handle, Ret &ret, const Params &... params)
	{

		std::pair<SmartScriptTable, HSCRIPTFUNCTION> map = GetFunc(handle);

		HSCRIPTFUNCTION pFunc = map.second;
		SmartScriptTable pHost = map.first;

		if (pFunc == nullptr) {return false;}

		bool bOk = true;
		if (pFunc && m_pSS->BeginCall(pFunc))
		{
			if (pHost != nullptr)
				m_pSS->PushFuncParam(pHost); // push host (self in lua)

			(m_pSS->PushFuncParam(params), ...);
			m_pSS->EndCall(ret);
		}
		else
			bOk = false;

		m_pSS->ReleaseFunc(pFunc);
		return bOk;
	}

	template<class... Params>
	bool Call(const std::string& handle, const Params &... params)
	{

		std::pair<SmartScriptTable, HSCRIPTFUNCTION> map = GetFunc(handle);

		HSCRIPTFUNCTION pFunc = map.second;
		SmartScriptTable pHost = map.first;

		if (pFunc == nullptr) {
			CryLogAlways("Function handle is nullptr");
			return false;
		}

		bool bOk = true;
		if (pFunc && m_pSS->BeginCall(pFunc))
		{
			if (pHost != nullptr)
				m_pSS->PushFuncParam(pHost); // push host (self in lua)

			(m_pSS->PushFuncParam(params), ...);
			m_pSS->EndCall();
		}
		else
			bOk = false;

		m_pSS->ReleaseFunc(pFunc);
		return bOk;
	}

	private:

		// IWeaponListener
		virtual void OnShoot(IWeapon* pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType,
			const Vec3& pos, const Vec3& dir, const Vec3& vel) override;
		virtual void OnStartFire(IWeapon* pWeapon, EntityId shooterId) override;
		virtual void OnStopFire(IWeapon* pWeapon, EntityId shooterId) override;
		virtual void OnStartReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) override;
		virtual void OnEndReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) override;
		virtual void OnOutOfAmmo(IWeapon* pWeapon, IEntityClass* pAmmoType) override;
		virtual void OnReadyToFire(IWeapon* pWeapon) override;
		virtual void OnPickedUp(IWeapon* pWeapon, EntityId actorId, bool destroyed) override;
		virtual void OnDropped(IWeapon* pWeapon, EntityId actorId) override;
		virtual void OnMelee(IWeapon* pWeapon, EntityId shooterId) override;
		virtual void OnStartTargetting(IWeapon* pWeapon) override;
		virtual void OnStopTargetting(IWeapon* pWeapon) override;
		virtual void OnSelected(IWeapon* pWeapon, bool selected) override;
};
