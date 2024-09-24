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
#include "AntiCheat.h"

#include "ServerTimer.h"
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
	
	//IGameFramework* m_pGameFramework = nullptr;
	IGame* m_pGame = nullptr;

	// ------------------------------
	static void m_pExitHandler();

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
	std::unique_ptr<ServerAnticheat> m_pAC;
	std::unique_ptr<ServerTimer> m_pServerTimer;

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

public:

	// ------------------------------
	Server();
	~Server();

	// ---------------------------------
	void Quit() { m_pExitHandler(); };


	// ============== LOGS =============
	void Log(const char* format, ...); //
	void LogError(const char* format, ...); // Error:
	void LogWarning(const char* format, ...); // Warning:

	// ============== INIT =============
	void Init(IGameFramework* pGameFramework);
	void OnGameStart(IGameObject *pGameRules);
	void UpdateLoop();

	// ============== SCRIPTS =============

	ServerTimer m_ScriptSecondTimer;
	ServerTimer m_ScriptMinuteTimer;
	ServerTimer m_ScriptHourTimer;

	void InitScript();
	void RegisterScriptGlobals();
	void LoadScript(bool ForceInit = false);
	void HandleScriptError(const std::string& error);
	void OnScriptLoaded(const std::string& fileName);
	bool OverwriteScriptPath(std::string& output, const std::string& input);

	bool IsLuaReady() const
	{
		bool error = false;
		if (m_pSS)
		{
			if (m_pSS->GetGlobalValue("SCRIPT_ERROR", error) && error)
			{
				return false;
			}
		}

		return (m_ScriptLoaded && m_ScriptInitialized);
	}

	bool m_ScriptInitialized = false;
	bool m_ScriptLoaded = false;


	// ------------------------------
	EntityId m_lastSpawnId = 0;
	int m_spawnCounter = 0;
	int m_counter = 0;

	int GetCounter() { m_counter++; return m_counter; };

	/*
	std::vector<std::string> m_uniqueNames;
	const char* GetUniqueName(const char* name) { 
		int c=GetCounter();  
		m_uniqueNames.push_back(std::string(name)+"_"+std::to_string(c));
		return m_uniqueNames[m_uniqueNames.size()-1].c_str();
	};*/

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
	IGameFramework* GetGameFramework() { return m_pGameFramework; }
	Executor* GetExecutor() { return m_pExecutor.get(); }
	HTTPClient* GetHTTPClient() { return m_pHttpClient.get(); }
	INetworkService* GetGSMaster() const { return m_pGSMaster; }

	ServerEvents* GetEvents() const { return m_pEventsCallback.get(); }
	ServerUtils* GetUtils() { return m_pServerUtils.get(); }
	ServerAnticheat* GetAC() { return m_pAC.get(); }
	ServerStats* GetStats() { return m_pServerStats.get(); }
	LuaFileSystem* GetLFS() const { return m_pLuaFileSystem.get(); }
	ServerTimer* GetTimer() const { return m_pServerTimer.get(); }
	std::string GetRoot() const { return m_rootDir.string(); }
	std::string GetWorkingDir() const { return m_workingDir.string(); }

	const std::vector<std::string>& GetMasters() const { return m_masters; }
	EntityId GetLastSpawnId() const { return m_lastSpawnId; }

	bool UpdateGameSpyServerReport(EGameSpyUpdateType type, const char* key, const char* value, int index = 0);

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

	// Debug
	ServerTimer m_DebugTimer;
	void Debug();
};

///////////////////////
inline Server* gServer;
///////////////////////

#endif // __SERVER_H__
