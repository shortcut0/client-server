// ------------------------------------------------------------
// File: <File>
// 
// Description:
//  <None>
// 
// Authors:
//   Marisa
//
// ------------------------------------------------------------

#include "CryMP/Common/Executor.h"
#include "CryMP/Common/HTTPClient.h"
#include "CryCommon/CrySystem/IConsole.h"
#include "CryCommon/CrySystem/ISystem.h"
#include "CryGame/Game.h"

#include "Server.h"
#include "ServerLog.h"
#include "ServerCVars.h"

#include "Library/StringTools.h"
#include "Library/Util.h"
#include "Library/WinAPI.h"
#include "Launcher/Resources.h"
#include "config.h"
#include "Version.h"

// File System
#include <iostream>
#include <fstream>
#include <string>


// ----------
ServerCVars* g_pServerCVars = 0;

// -------------------------------------
Server::Server()
{
}

// -------------------------------------
Server::~Server()
{
}

// -------------------------------------
void Server::Init(IGameFramework* pGameFramework)
{
	IConsole* pConsole = gEnv->pConsole;
	IScriptSystem* pScriptSystem = gEnv->pScriptSystem;
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

	m_rootDir = std::filesystem::canonical(gEnv->pSystem->GetRootFolder());
	m_workingDir = std::filesystem::current_path();

	if (m_rootDir.empty())
		m_rootDir = m_workingDir;

	// -----------
	// Log
	Log("****************************************************************");
	Log("Server::Init()");

	// -----------
	this->m_pGSMaster			= GetISystem()->GetINetwork()->GetService("GameSpy");
	this->m_pGameFramework		= pGameFramework;
	this->m_pSS					= gEnv->pScriptSystem;


	// -----------
	this->m_pExecutor			= std::make_unique<Executor>();
	this->m_pHttpClient			= std::make_unique<HTTPClient>(*this->m_pExecutor);
	//this->m_pServerPublisher	= std::make_unique<ServerPublisher>(); // moved to lua for unknown reasons
	this->m_pEventsCallback		= std::make_unique<ServerEvents>();
	this->m_pScriptBindServer	= std::make_unique<ScriptBind_Server>();
	this->m_pLuaFileSystem		= std::make_unique<LuaFileSystem>();
	this->m_pServerUtils		= std::make_unique<ServerUtils>();
	this->m_pCVars				= std::make_unique<ServerCVars>();
	this->m_pServerStats		= std::make_unique<ServerStats>();

	// -----------
	pGameFramework->RegisterListener(this, "crymp-server", FRAMEWORKLISTENERPRIORITY_DEFAULT);
	pGameFramework->GetILevelSystem()->AddListener(this);
	pEntitySystem->AddSink(this);

	// -----------
	g_pGame = new CGame();
	g_pGame->Init(pGameFramework);

	// -----------
	m_pServerUtils->Init();
	InitCommands(pConsole);
	InitCVars(pConsole);
	InitMasters(); // Must be done before network stuff
	//m_pServerPublisher->Init(gEnv->pSystem);

	// -----------
	m_serverScriptPath = m_rootDir.string() + std::string("/Scripts/Main.lua");
	m_Initialized = true;

	m_tickGoal = 1.f;
	m_minGoal = 60.f;
	m_hourGoal = 3600.f;

	m_lastChannel = 1;
}

// -------------------------------------
void Server::UpdateLoop()
{
	ReadCfg(std::string("ATOM - 2024\\Config\\ATOM.cfg"));

	const bool haveFocus = true;
	const unsigned int updateFlags = 0;

	while (g_pGame->Update(haveFocus, updateFlags))
	{

		float frametime = gEnv->pTimer->GetFrameTime();
		bool newChan = false;

		if (INetChannel* pNextChannel = g_pGame->GetIGameFramework()->GetNetChannel(m_lastChannel)) {
			newChan = true;
		}

		if (m_Initialized) {
		
			if (IsLuaReady()) {

				m_tickCounter += gEnv->pTimer->GetFrameTime();
				Script::CallMethod(m_ServerRPCCallbackLua, "OnUpdate");

				if (m_tickCounter >= m_tickGoal) {
					Script::CallMethod(m_ServerRPCCallbackLua, "OnTimer", 1);
					m_tickGoal += (1.0f);
				}

				if (m_tickCounter >= m_minGoal) {
					Script::CallMethod(m_ServerRPCCallbackLua, "OnTimer", 2);
					m_minGoal += (60.0f);
				}

				if (m_tickCounter >= m_hourGoal) {
					Script::CallMethod(m_ServerRPCCallbackLua, "OnTimer", 3);
					m_hourGoal += (3600.f);
				}


				if (newChan) {
					std::string ip = GetUtils()->CryChannelToIP(m_lastChannel);
					GetEvents()->Call("ServerRPC.Callbacks.OnConnection", m_lastChannel, ip.c_str());
				}
				
			}
		}
		// Update always in case of script errors to prevent the channel queue from getting stuck!
		if (newChan)
			m_lastChannel++;

		m_bErrorHandledFrame = false;

		if (m_pSS)
			m_pSS->SetGlobalValue("DLL_ERROR", false);
	
		//GetStats()->Update(gEnv->pTimer->GetAsyncTime().GetMilliSeconds());
		//CryLogAlways("%.2f", GetStats()->GetCPUUsage());
	}

}

// -------------------------------------
void Server::InitCommands(IConsole *pConsole)
{

	pConsole->AddCommand("server_reloadScript", OnInitLuaCmd, VF_NOT_NET_SYNCED, "Reload the main Server Initializer Script");
}


// -------------------------------------
void Server::InitCVars(IConsole *pConsole)
{

	if (m_pCVars) {
		m_pCVars->InitCVars(pConsole);
		g_pServerCVars = GetCVars();
	}
}

// -------------------------------------
bool Server::ReadCfg(std::string filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open()) {
		LogError("Could not open the config file \"%s\"", filepath.c_str());
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		gEnv->pConsole->ExecuteString(line.c_str());
	}

	file.close();

	if (file.fail() && !file.eof()) {
		LogError("An error occurred while reading the config file \"%s\"", filepath.c_str());
		return false;
	}

	return true;
}

// -------------------------------------
void Server::InitMasters()
{
	std::string content;

	WinAPI::File file("masters.txt", WinAPI::FileAccess::READ_ONLY);  // Crysis main directory
	if (file)
	{
		CryLogAlways("$6[CryMP] Using local masters.txt as the master server list provider");

		try
		{
			content = file.Read();
		}
		catch (const std::exception& ex)
		{
			CryLogAlways("$4[CryMP] Failed to read the masters.txt file: %s", ex.what());
		}
	}
	else
		content = WinAPI::GetDataResource(nullptr, RESOURCE_MASTERS);

	for (const std::string_view& master : Util::SplitWhitespace(content))
		m_masters.emplace_back(master);

	if (m_masters.empty())
		m_masters.emplace_back("crymp.org");
}

// -------------------------------------
std::string Server::GetMasterServerAPI(const std::string& master)
{
	if (master.empty())
	{
		return "https://" + m_masters[0] + "/api";
	}
	else
	{
		int a = 0, b = 0, c = 0, d = 0;
		// in case it is IP, don't use HTTPS
		if (sscanf(master.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) == 4 || master == "localhost")
			return "http://" + master + "/api";
		else
			return "https://" + master + "/api";
	}
}

// -------------------------------------
void Server::HttpGet(const std::string_view& url, std::function<void(HTTPClientResult&)> callback)
{
	HTTPClientRequest request;
	request.method = "GET";
	request.url = url;
	request.callback = std::move(callback);

	HttpRequest(std::move(request));
}

// -------------------------------------
void Server::HttpRequest(HTTPClientRequest&& request)
{
	for (const std::string& master : m_masters)
	{
		if (Util::StartsWith(request.url, GetMasterServerAPI(master)))
		{
			break;
		}
	}

	//Log("HTTP(%s): %s", request.url.c_str(), request.data.c_str());
	m_pHttpClient->Request(std::move(request));
}

// -------------------------------------
void Server::OnInitLuaCmd(IConsoleCmdArgs* pArgs) {
	gServer->InitLua();
}

// -------------------------------------
void Server::InitLua() {

	// -----
	m_pSS->SetGlobalValue("CRYMP_SERVER_EXE", CRYMP_SERVER_EXE_NAME);
	m_pSS->SetGlobalValue("CRYMP_SERVER_BITS", CRYMP_SERVER_BITS);
	m_pSS->SetGlobalValue("CRYMP_SERVER_VERSION", CRYMP_SERVER_VERSION);
	m_pSS->SetGlobalValue("CRYMP_SERVER_VERSION_STRING", CRYMP_SERVER_VERSION_STRING);
	m_pSS->SetGlobalValue("CRYMP_SERVER_BUILD_TYPE", CRYMP_SERVER_BUILD_TYPE);
	m_pSS->SetGlobalValue("CRYMP_SERVER_COMPILER", CRYMP_SERVER_COMPILER);

	// -----
	if (m_serverScriptPath.empty()) {
		LogError("Server Script path empty!");
		return;
	}

	// -----
	Log("Loading Server Script %s...", m_serverScriptPath.c_str());

	// -----
	if (!m_pSS->ExecuteFile(m_serverScriptPath.c_str(), true, true)) {
		LogError("Failed to load the Server Script %s", m_serverScriptPath.c_str());
		return;
	}

	// -----
	if (!m_pSS->GetGlobalValue("Server", m_ATOMLua)) {
		LogError("Server Global not found (Server Script: %s)", m_serverScriptPath.c_str());
		return;
	}

	// -----
	if (!m_pSS->GetGlobalValue("ServerRPC", m_ServerRPCLua)) {
		LogError("ServerRPC Global not found (Server Script: %s)", m_serverScriptPath.c_str());
		return;
	}

	// -----
	if (!m_ServerRPCLua->GetValue("Callbacks", m_ServerRPCCallbackLua)) {
		LogError("ServerRPC.Callbacks not found (Server Script: %s)", m_serverScriptPath.c_str());
		return;
	}

	Log("Server Script Loaded");
	m_ATOMLuaInitialized = true;
	return;
}

// -------------------------------------
void Server::OnPostUpdate(float deltaTime)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);
	this->m_pExecutor->OnUpdate();
}

// -------------------------------------
void Server::OnGameStart(IGameObject* pGameRules)
{

}

// -------------------------------------
void Server::OnSaveGame(ISaveGame* saveGame)
{
}

// -------------------------------------
void Server::OnLoadGame(ILoadGame* loadGame)
{
}

// -------------------------------------
void Server::OnLevelEnd(const char* nextLevel)
{
}

// -------------------------------------
void Server::OnActionEvent(const SActionEvent& event)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	switch (event.m_event)
	{
		case eAE_channelCreated:
		case eAE_channelDestroyed:
		case eAE_connectFailed:
		case eAE_connected:
		case eAE_disconnected:
		case eAE_clientDisconnected:
		case eAE_resetBegin:
		case eAE_resetEnd:
		case eAE_resetProgress:
		case eAE_preSaveGame:
		case eAE_postSaveGame:
		case eAE_inGame:
		case eAE_serverName:
		case eAE_serverIp:
		case eAE_earlyPreUpdate:
		{
			break;
		}
	}
}

// -------------------------------------
void Server::OnLevelNotFound(const char* levelName)
{
}

// -------------------------------------
void Server::OnLoadingStart(ILevelInfo* pLevel)
{
	gEnv->pScriptSystem->ForceGarbageCollection();
}

// -------------------------------------
void Server::OnLoadingComplete(ILevel* pLevel)
{
	InitLua();
}

// -------------------------------------
void Server::OnLoadingError(ILevelInfo* pLevel, const char* error)
{
}

// -------------------------------------
void Server::OnLoadingProgress(ILevelInfo* pLevel, int progressAmount)
{
}


// -------------------------------------
bool Server::OnBeforeSpawn(SEntitySpawnParams& params)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	//CryMP: Archetype Loader
	if (params.sName && params.sName[0] == '*')
	{
		const char* name = params.sName + 1;
		const char* archetypeEnd = strchr(name, '|');

		if (archetypeEnd)
		{
			const std::string archetypeName(name, archetypeEnd - name);

			if (!gEnv->bServer)
			{
				params.sName = archetypeEnd + 1;
			}
			params.pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(archetypeName.c_str());

			if (!params.pArchetype && params.pClass)
			{
				Log("Archetype '%s' not found for entity %s",
					archetypeName.c_str(),
					params.pClass->GetName()
				);

				if (gEnv->bServer) //If archetype loading failed, don't proceed spawn on the server
					return false;
			}
		}
	}

	return true;
}

// -------------------------------------
void Server::OnSpawn(IEntity* pEntity, SEntitySpawnParams& params)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);
	m_lastSpawnId = pEntity->GetId();
}

// -------------------------------------
bool Server::OnRemove(IEntity* pEntity)
{
	return true;
}

// -------------------------------------
void Server::OnEvent(IEntity* pEntity, SEntityEvent& event)
{
}

// -------------------------------------
void Server::HandleScriptError(const std::string &error)
{
	if (IsLuaReady()) {
		if (m_bErrorHandledFrame)
			return;

		m_pSS->SetGlobalValue("DLL_ERROR", true);
		GetEvents()->Call("HandleError", error.c_str());
	}
}

// -------------------------------------
void Server::Log(const char* format, ...) {
	va_list args;
	va_start(args, format);
	ServerLog::Log("$9[$4ATOM$9] ", format, args);
	va_end(args);
}

// -------------------------------------
void Server::LogError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	ServerLog::Log("$9[$4ATOM$9] Error: ", format, args);
	va_end(args);
}

// -------------------------------------
void Server::LogWarning(const char* format, ...) {
	va_list args;
	va_start(args, format);
	ServerLog::Log("$9[$4ATOM$9] Warning: ", format, args);
	va_end(args);
}