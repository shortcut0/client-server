#include "CryGame/Actors/Actor.h"
#include "ServerEvents.h"

#include <cmath>  // For sqrt and pow functions

class ServerAnticheat 
{

private:

	ServerEvents* m_pEvents;
	IGameFramework* m_pGameFramework;
	bool m_status = true;

	CActor* GetActor(EntityId actorId)
	{
		return static_cast<CActor*>(m_pGameFramework->GetIActorSystem()->GetActor(actorId));
	}

	CActor* GetActorByChannel(uint16 actorId)
	{
		return static_cast<CActor*>(m_pGameFramework->GetIActorSystem()->GetActorByChannelId(actorId));
	}

public:

	// ------------------------------
	ServerAnticheat() {};
	~ServerAnticheat() {};

	void Init(IGameFramework* pGameFramework, ServerEvents* pEvents) { 
		m_pGameFramework = pGameFramework; 
		m_pEvents = pEvents; 
	};

	// -----------------
	void ToggleStatus(bool mode) { m_status = mode; };

	// -----------------
	template<class... Params>
	bool Distance(INetChannel* pNetChannel, Vec3 from, Vec3 to, float threshold, const char* sCheatName, const char* sFunctionPtr, const Params &... params) {
		float distance = std::sqrt(std::pow(to.x - from.x, 2) +
			std::pow(to.y - from.y, 2) +
			std::pow(to.z - from.z, 2));
		
		if (!pNetChannel || distance <= threshold)
			return true;

		// try to handle cheater.. if they exist..
		if (uint16 pChannelId = m_pGameFramework->GetGameChannelId(pNetChannel)) {
			if (CActor* pNetActor = GetActorByChannel(pChannelId))
			{
				EntityId pNetId = pNetActor->GetEntityId();
				m_pEvents->Call("ServerRPC.Callbacks.OnCheat", pChannelId, sCheatName, sFunctionPtr, ScriptHandle(pNetId), ScriptHandle(pNetId), false, params...);
			}
		}

		return false;
	}

	// -----------------
	template<class... Params>
	bool CheckDistance(INetChannel* pNetChannel, const Params &... params) { return Distance(pNetChannel, params...); } // ?!

	// -----------------

	template<class... Params>
	bool CheckOwnerRequest(INetChannel* pNetChannel, EntityId pOwnerId, const char* sCheatName, const char* sFunctionPtr, bool positive, const Params &... params) {
		if (!m_status)
		{
			return true; // ok
		}

		uint16 pChannelId = m_pGameFramework->GetGameChannelId(pNetChannel);
		if (!pChannelId) 
		{
			return false; // channel not found, drop request
		}

		CActor* pNetActor = GetActorByChannel(pChannelId);
		if (!pNetActor) 
		{
			return false; // no actor associated with this channel, drop request
		}

		CActor* pOwnerActor = GetActor(pOwnerId);
		if (!pOwnerActor) 
		{
			return false; // owner actor not found, drop request
		}

		EntityId pNetId = pNetActor->GetEntityId();
		EntityId pActorId = pOwnerActor->GetEntityId();

		// Special case for laggy players, might send RMI upon death, causing a false positive
		if (pNetId != pActorId && (pActorId == NULL || pNetId == NULL))
		{
			return false;
		}

		// We only call this if the ID has been spoofed!
		if (pActorId != pNetId && pOwnerActor->GetChannelId() != pChannelId) {
			m_pEvents->Call("ServerRPC.Callbacks.OnCheat", pChannelId, sCheatName, sFunctionPtr, ScriptHandle(pNetId), ScriptHandle(pActorId), positive, params...);
			return false; // owner does not match channel owner, probable cheat!!
		}

		return true; // request ok!
	}

	bool CheckLongpoke(CActor* pActor, int seq) {
		if (!m_status) { 
			return false;  // no longpoke
		} 
		return seq == 1; // SEQ == 1 means longpoke ??
	};
};