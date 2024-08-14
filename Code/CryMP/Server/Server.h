#pragma once

#ifndef __SERVER_H__
#define __SERVER_H__

// Windows
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>

#include "Library/WinAPI.h"

// Listeners
#include "CryCommon/CryAction/IGameFramework.h"
#include "CryCommon/CryAction/ILevelSystem.h"
#include "CryCommon/CryEntitySystem/IEntitySystem.h"
#include "CryCommon/CryScriptSystem/IScriptSystem.h"

// Server
#include "ServerStats.h"

#include "ServerCVars.h"
#include "ServerUtils.h"
#include "ServerEvents.h"
#include "ServerPublisher.h"
#include "ScriptBind_Server.h"
#include "LuaFileSystem.h"
#include "CryMP/Common/HTTPClient.h"

#include "CryCommon/CryNetwork/INetworkService.h"

#include "CryScriptSystem/ScriptSystem.h"

class Executor;
class HTTPClient;

enum class EGameSpyUpdateType {
	eGSUpdate_Server = 0,
	eGSUpdate_Player,
	eGSUpdate_Team
};
//ServerPublisher *gServerPbl;


// ---------------------------------
class Server : public IGameFrameworkListener, public ILevelSystemListener, public IEntitySystemSink
{
	// ------------------------------
	static void m_pExitHandler();;

	// ------------------------------
	IGameFramework* m_pGameFramework = nullptr;
	INetworkService* m_pGSMaster = nullptr;

	// ------------------------------
	std::unique_ptr<Executor> m_pExecutor;
	std::unique_ptr<HTTPClient> m_pHttpClient;
	//std::unique_ptr<ServerPublisher> m_pServerPublisher;
	std::unique_ptr<ServerEvents> m_pEventsCallback;
	std::unique_ptr<ScriptBind_Server> m_pScriptBindServer;
	std::unique_ptr<LuaFileSystem> m_pLuaFileSystem;
	std::unique_ptr<ServerUtils> m_pServerUtils;
	std::unique_ptr<ServerStats> m_pServerStats;

	// ------------------------------

	std::vector<std::pair<EntityId, float>> m_scheduledEntityRemovals;
	void UpdateEntityRemoval();
	//void ScheduleEntityRemoval();

	std::vector<std::string> m_masters;

	std::filesystem::path m_rootDir;
	std::filesystem::path m_workingDir;

	// ------------------------------

	std::unique_ptr<ServerCVars> m_pCVars;

	ServerCVars* GetCVars() { 
		return m_pCVars.get(); 
	}

	// ------------------------------

	void InitGameOverwrites(const std::string& filename);
	std::vector<std::pair<std::string, std::string>> m_pOverwriteSF;

	// ------------------------------

	IScriptSystem* m_pSS = nullptr;
	std::string m_serverScriptPath;
	SmartScriptTable m_ATOMLua;
	SmartScriptTable m_ServerRPCLua;
	SmartScriptTable m_ServerRPCCallbackLua;
	bool m_ATOMLuaInitialized = false;

public:

	// ------------------------------
	Server();
	~Server();

	// ---------------------------------
	// Overwrites input script path 
	bool OverwriteScriptPath(std::string &output, const std::string& input);

	// ---------------------------------
	void Log(const char* format, ...); //
	void LogError(const char* format, ...); // Error:
	void LogWarning(const char* format, ...); // Warning:

	// ------------------------------
	void Init(IGameFramework* pGameFramework);
	void InitLua();
	void OnGameStart(IGameObject *pGameRules);
	void UpdateLoop();

	// ------------------------------
	EntityId m_lastSpawnId = 0;

	// ------------------------------
	void InitCVars(IConsole* pConsole);
	void InitCommands(IConsole* pConsole);
	static void OnInitLuaCmd(IConsoleCmdArgs* pArgs);

	// ------------------------------
	void InitMasters();
	void HttpGet(const std::string_view& url, std::function<void(HTTPClientResult&)> callback);
	void HttpRequest(HTTPClientRequest&& request);
	std::string GetMasterServerAPI(const std::string& master);

	// ------------------------------
	IGameFramework* GetGameFramework()
	{
		return m_pGameFramework;
	}

	Executor* GetExecutor()
	{
		return m_pExecutor.get();
	}

	ServerUtils* GetUtils()
	{
		return m_pServerUtils.get();
	}

	ServerStats* GetStats()
	{
		return m_pServerStats.get();
	}

	HTTPClient* GetHTTPClient()
	{
		return m_pHttpClient.get();
	}

	//ServerPublisher* GetServerPublisher()
	//{
	//	return m_pServerPublisher.get();
	//}

	ServerEvents* GetEvents() const
	{
		return m_pEventsCallback.get();
	}

	LuaFileSystem* GetLFS() const
	{
		return m_pLuaFileSystem.get();
	}

	const std::vector<std::string>& GetMasters() const
	{
		return m_masters;
	}

	EntityId GetLastSpawnId() const
	{
		return m_lastSpawnId;
	}

	SmartScriptTable GetATOMLua() const
	{
		return m_ATOMLua;
	}

	SmartScriptTable GetLuaServerRPC() const
	{
		return m_ServerRPCLua;
	}

	bool IsLuaReady() const
	{
		bool error = false;
		if (m_pSS)
			m_pSS->GetGlobalValue("SCRIPT_ERROR", error);

		if (!m_ServerRPCCallbackLua)
			return false;

		if (!m_ServerRPCLua)
			return false;

			if (!m_ATOMLua)
				return false;

		return !error && m_ATOMLuaInitialized;
	}

	std::string GetRoot() const
	{
		return m_rootDir.string();
	}

	std::string GetWorkingDir() const
	{
		return m_workingDir.string();
	}

	INetworkService *GetGSMaster() const
	{
		return m_pGSMaster;
	}

	bool UpdateGameSpyServerReport(EGameSpyUpdateType type, const char* key, const char* value, int index = 0);


	bool m_bErrorHandledFrame = false;
	void HandleScriptError(const std::string &error);
	

private:
	// IGameFrameworkListener
	void OnPostUpdate(float deltaTime) override;
	void OnSaveGame(ISaveGame* saveGame) override;
	void OnLoadGame(ILoadGame* loadGame) override;
	void OnLevelEnd(const char* nextLevel) override;
	void OnActionEvent(const SActionEvent& event) override;

	// ILevelSystemListener
	void OnLevelNotFound(const char* levelName) override;
	void OnLoadingStart(ILevelInfo* pLevel) override;
	void OnLoadingComplete(ILevel* pLevel) override;
	void OnLoadingError(ILevelInfo* pLevel, const char* error) override;
	void OnLoadingProgress(ILevelInfo* pLevel, int progressAmount) override;

	// IEntitySystemSink
	bool OnBeforeSpawn(SEntitySpawnParams& params) override;
	void OnSpawn(IEntity* pEntity, SEntitySpawnParams& params) override;
	bool OnRemove(IEntity* pEntity) override;
	void OnEvent(IEntity* pEntity, SEntityEvent& event) override;

	//
	bool ReadCfg(const std::string &filepath);
	
	//
	bool m_Initialized = false;
	float m_tickCounter = 0;
	float m_tickGoal = 0;
	float m_minGoal = 0;
	float m_hourGoal = 0;

	int m_lastChannel = 0;
};

///////////////////////
inline Server* gServer;
///////////////////////

#endif // __SERVER_H__
