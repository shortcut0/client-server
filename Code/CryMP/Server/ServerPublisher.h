#include "CryCommon/CrySystem/ITimer.h" // TImer
#include "CryCommon/CryScriptSystem/IScriptSystem.h"
#include "Server.h"

#ifndef _ATOM_SERVERPBL_
#define _ATOM_SERVERPBL_


struct ServerPublisher
{

private:

	// -------------------------------------
	// Timer
	ITimer *m_pTimer;

	// -------------------------------------
	// Updates
	int m_timeout = 1000;
	float m_lastUpdate;
	bool m_bExposed = false;
	bool m_bExposedSuccess = false;

	SmartScriptTable m_pLuaPart;
	
public:

	// -------------------------------------
	std::string m_master; // Master Server address
	float m_updateRate = 30.0f; // Time between updates sent to master server

	std::string m_cookie;

	// -------------------------------------
	void Init(ISystem* pSystem);
	void ExposeServer(); // Expose server to master server
	void ExposeStatus() {};// Expose players to master server

	// -------------------------------------
	void SetMasterServer(const char* server);
	void SetUpdateInterval(float interval) { m_updateRate = interval; };
	std::string ExtractCookie(std::string inputCookie);
	void SetCookie(std::string cookie) { m_cookie = cookie; };
	void OnExposed(bool success) { m_bExposedSuccess = success; };

	// -------------------------------------
	void UpdateNow(float frametime);
	void Update(float frametime);

	// -------------------------------------
	std::string GatherPlayers(); // Collect players for server report
	std::string GetMapDownloadLink(); // Collect players for server report
	std::string GetLocalIP(); // Collect players for server report
	std::string GetServerDescription(); // Collect players for server report

	// -------------------------------------
	void Log(const char* format, ...);
	void LogError(const char* format, ...);
	void LogWarning(const char* format, ...);
};

#endif // ! _ATOM_SERVERPBL_