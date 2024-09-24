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

#include "CryCommon/CryAction/IVehicleSystem.h"
#include "CryCommon/CryAction/IItemSystem.h"
#include "CryGame/Items/Weapons/WeaponSystem.h"

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

// Exit handler
#include "QuitHook.h"
#include <cstdlib>  // For std::atexit

// deug..
#include "CryCommon/CryAction/IGameRulesSystem.h"

// ----------
ServerCVars* g_pServerCVars = 0;

// -------------------------------------
Server::Server() :
	m_Initialized(false)
{
}

// -------------------------------------
Server::~Server()
{
}

// -------------------------------------
void Server::m_pExitHandler() {

	gServer->Log("Exiting...");

	if (gServer->m_pSS && gServer->IsLuaReady()) {
		gServer->Log("Trying to inform LUA...");
		if (ServerEvents* pEvents = gServer->GetEvents()) {
			gServer->Log("Calling lua...");
			pEvents->Call("ServerRPC.Callbacks.OnGameQuit");
		}
	}
}

// -------------------------------------
void Server::Init(IGameFramework* pGameFramework)
{
	
	// --------------------------------------
	Log("Setting atexit callback..");
	QuitHook::Init();
	//std::atexit(m_pExitHandler);

	// --------------------------------------
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
	this->m_pAC					= std::make_unique<ServerAnticheat>();
	this->m_pServerTimer		= std::make_unique<ServerTimer>();


	// -----------
	pGameFramework->RegisterListener(this, "crymp-server", FRAMEWORKLISTENERPRIORITY_DEFAULT);
	pGameFramework->GetILevelSystem()->AddListener(this);
	pEntitySystem->AddSink(this);

	// --------------
	// Init AntiCheat
	m_pAC->Init(pGameFramework, GetEvents());

	// --------------
	// Load Early init
	if (!m_pSS->ExecuteFile(std::string(m_rootDir.string() + "\\Scripts\\EarlyInit.lua").c_str()))
		Log("Failed to load EarlyInit.lua");
	else
		Log("EarlyInit.lua loaded..");

	// -----------
	// Init Game
	if (!WinAPI::CmdLine::HasArg("-oldgame"))
	{
		m_pGame = new CGame();
	}
	else
	{
		Log("***************** Using Old Game.DLL! *****************");
		void* pCryGame = WinAPI::DLL::Load("CryGame.dll");
		if (!pCryGame)
		{
			throw StringTools::SysErrorFormat("Failed to load the CryGame DLL!");
		}

		auto entry = static_cast<IGame::TEntryFunction>(WinAPI::DLL::GetSymbol(pCryGame, "CreateGame"));
		if (!entry)
		{
			throw StringTools::ErrorFormat("The CryGame DLL is not valid!");
		}

		m_pGame = entry(pGameFramework);
	}
	//g_pGame = new CGame();
	//g_pGame = m_pGame;
	//g_pGame = static_cast<CGame*>(g_pGame);
	//g_pGame = static_cast<CGame*>(g_pGame);
	m_pGame->Init(pGameFramework);

	// -----------
	// Init utils and cvars


	m_DebugTimer = ServerTimer(0.1f);
	m_ScriptSecondTimer = ServerTimer(1.f);
	m_ScriptMinuteTimer = ServerTimer(60.f);
	m_ScriptHourTimer = ServerTimer(3600.f);

	m_pServerUtils->Init();
	InitCommands(pConsole);
	InitCVars(pConsole);
	InitMasters(); 
	
	// Must be done before network stuff
	//m_pServerPublisher->Init(gEnv->pSystem);

	// -----------
	m_serverScriptPath = m_rootDir.string() + ("/Scripts/Main.lua");
	m_Initialized = true;

	m_tickGoal = 1.f;
	m_minGoal = 60.f;
	m_hourGoal = 3600.f;

	m_lastChannel = 1;

	// --------------------------------
	// OVERWRITE GAME SCRIPT FILES

	const std::string overwriteFile = m_rootDir.string() + "/Scripts/GameOverwrites.txt";
	InitGameOverwrites(overwriteFile);

	// -----------
	// Load Script File
	RegisterScriptGlobals();
	LoadScript();
}

// -------------------------------------
void Server::Debug()
{
	IEntityItPtr pIIt = gEnv->pEntitySystem->GetEntityIterator();
	IEntity* pEntity = 0;

	pIIt->MoveFirst();

	IGameRulesSystem* pGR = gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem();
	IGameRules* pGRR = pGR->GetCurrentGameRules();
	//CGameRules* pCGR = static_cast<CGameRules*>(pGRR);

	m_pGame->GetIGameFramework()->GetIGameRulesSystem();

	while (pEntity = pIIt->Next())
	{
		if (strcmp("Player", pEntity->GetClass()->GetName()) == 0)
		{
			GetEvents()->Call("ServerCommands.DebugEffect", pEntity->GetScriptTable(), "/flare");
			GetEvents()->Call("g_gameRules.game.ServerExplosion", ScriptHandle(EntityId(0)), ScriptHandle(EntityId(0)),0, pEntity->GetWorldPos(),Vec3(0,0,1),0,0,0,0,"explosions.flare.a",1,0, 0,0,0);
			
			/*
			IScriptTable* gr;
			IScriptTable* grg;
			gEnv->pScriptSystem->GetGlobalValue("g_gameRules", gr);
			if (!gr)
			{
				Log("not found yet");
				return;
			}
			gr->GetValue("game", grg);
			HSCRIPTFUNCTION spawn;
			grg->GetValue("ServerExplosion", spawn);

			//Script::Call(gEnv->pScriptSystem, spawn, 0, 0, 0, pEntity->GetWorldPos(), Vec3(0, 0, 1), 0, 0, 0, 0, "explosions.flare.a", 1, 0, 0, 0, 0);

			gEnv->pScriptSystem->BeginCall(spawn);
			gEnv->pScriptSystem->PushFuncParam(grg);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(pEntity->GetWorldPos());
			gEnv->pScriptSystem->PushFuncParam(Vec3(0,0,1));
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam("explosions.flare.a");
			gEnv->pScriptSystem->PushFuncParam(1);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);

			// 0, 0, 0, 0, "explosions.flare.a", 1, 0, 0, 0, 0
			gEnv->pScriptSystem->EndCall();

			gEnv->pScriptSystem->ReleaseFunc(spawn);
			
			// g_pGame:ServerExplosion(NULL_ENTITY, NULL_ENTITY, 0, (vPos), (vDir or vectors.up), 0, 0, 0, 0, sEffect, (iScale or 1), nil, 0, 0, 0)
			*/
			HitInfo sh;
			sh.shooterId = pEntity->GetId();
			sh.targetId = pEntity->GetId();
			sh.remote = true;
			sh.type = 0;
			sh.weaponId = pEntity->GetId();
			sh.pos = pEntity->GetWorldPos();
			sh.dir = Vec3(0, 0, 0);
			sh.fmId = 1;
			sh.bulletType = 1;
			sh.angle = 45;
			sh.damage = 999;
			sh.material = 1;
			sh.normal = Vec3(0, 0, 0);
			sh.seq = 1;
			sh.radius = 1;
			sh.projectileId = 0;
			pGRR->ClientHit(sh);
			Log("hit %s", pEntity->GetName());
		}
	}
}

// -------------------------------------
void Server::UpdateLoop()
{

	// ========== CFG =============
	std::string cfg ("CryMP-Server\\Config\\Init.cfg");
	if (WinAPI::CmdLine::HasArg("-headless"))
	{
		cfg = "CryMP-Server\\Config\\Init-Headless.cfg";
	}

	ReadCfg(cfg);

	// ========== UPDATE =============
	const bool haveFocus = true;
	const unsigned int updateFlags = 0;

	while (m_pGame->Update(haveFocus, updateFlags))
	{

		float frametime = gEnv->pTimer->GetFrameTime();
		bool NewChannel = false;

		bool ScriptReady = IsLuaReady();

		while (INetChannel* pNextChannel = m_pGame->GetIGameFramework()->GetNetChannel(m_lastChannel)) 
		{
			if (ScriptReady)
			{
				const std::string ip = GetUtils()->CryChannelToIP(m_lastChannel);
				GetEvents()->Call("ServerRPC.Callbacks.OnConnection", m_lastChannel, ip.c_str());
			}
			m_lastChannel++;
		}

		if (m_Initialized) {


			if (GetCVars()->server_debug_positions > 0 && m_DebugTimer.Expired_Refresh())
			{
				Debug();
			}
		
			if (ScriptReady) 
			{

				m_tickCounter += frametime;
				GetStats()->Update(m_tickCounter);
				
				// Every Frame
				GetEvents()->Call("ServerRPC.Callbacks.OnUpdate");
				
				// Every Second
				//Log("Diff= %f, since creation: %f", m_ScriptSecondTimer.Diff(), m_ScriptSecondTimer.Diff_C());
				if (m_ScriptSecondTimer.Expired_Refresh())
				{
					GetEvents()->Call("ServerRPC.Callbacks.OnTimer", 1);
				}

				// Every Minute
				if (m_ScriptMinuteTimer.Expired_Refresh())
				{
					GetEvents()->Call("ServerRPC.Callbacks.OnTimer", 2);
				}

				// Every Hour
				if (m_ScriptHourTimer.Expired_Refresh())
				{
					GetEvents()->Call("ServerRPC.Callbacks.OnTimer", 3);
				}

				/*if (m_ServerRPCCallbackLua && m_ServerRPCCallbackLua.GetPtr()) {

					Script::CallMethod(m_ServerRPCCallbackLua, "OnUpdate");

					if (m_tickCounter >= m_tickGoal) 
					{
						Script::CallMethod(m_ServerRPCCallbackLua, "OnTimer", 1);
						m_tickGoal += (1.0f);
					}

					if (m_tickCounter >= m_minGoal) 
					{
						Script::CallMethod(m_ServerRPCCallbackLua, "OnTimer", 2);
						m_minGoal += (60.0f);
					}

					if (m_tickCounter >= m_hourGoal) 
					{
						Script::CallMethod(m_ServerRPCCallbackLua, "OnTimer", 3);
						m_hourGoal += (3600.f);
					}
				}*/
				
			}
		}

		if (m_pSS)
		{
			m_pSS->SetGlobalValue("DLL_ERROR", false);
		}

		// exists already :s
		//UpdateEntityRemoval();
	}


	// FIXME: i don't think this is called
	// Save Data !!
	if (IsLuaReady()) 
	{
		GetEvents()->Call("ServerRPC.Callbacks.OnGameShutdown");
	}

}

// -------------------------------------
void Server::UpdateEntityRemoval()
{

	float frameTime = gEnv->pTimer->GetCurrTime();
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

	if (m_scheduledEntityRemovals.empty())
		return;

	// Iterate and remove EntityId 6789
	for (auto it = m_scheduledEntityRemovals.begin(); it != m_scheduledEntityRemovals.end(); /* no increment here */) {
		if (it->second >= frameTime) {  // Access the EntityId using it->first

			//FIXME
			//pEntitySystem->RemoveEntity(it->first);
			it = m_scheduledEntityRemovals.erase(it);  // Remove element and get new iterator
		}
		else {
			++it;  // Move to the next element
		}
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
bool Server::ReadCfg(const std::string &filepath)
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
	 // FIXME: Use server dir!

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
	gServer->LoadScript(true);
}

// -------------------------------------
void Server::RegisterScriptGlobals() {

	// -----
	m_pSS->SetGlobalValue("CRYMP_SERVER_EXE", CRYMP_SERVER_EXE_NAME);
	m_pSS->SetGlobalValue("CRYMP_SERVER_BITS", CRYMP_SERVER_BITS);
	m_pSS->SetGlobalValue("CRYMP_SERVER_VERSION", CRYMP_SERVER_VERSION);
	m_pSS->SetGlobalValue("CRYMP_SERVER_VERSION_STRING", CRYMP_SERVER_VERSION_STRING);
	m_pSS->SetGlobalValue("CRYMP_SERVER_BUILD_TYPE", CRYMP_SERVER_BUILD_TYPE);
	m_pSS->SetGlobalValue("CRYMP_SERVER_COMPILER", CRYMP_SERVER_COMPILER);

}

// -------------------------------------
void Server::LoadScript(bool ForceInit) {

	const char* ScriptFile = m_serverScriptPath.c_str();

	Log("Loading Server Script: %s", ScriptFile);
	m_ScriptLoaded = m_pSS->ExecuteFile(ScriptFile, true, true);
	
	if (!m_ScriptLoaded) 
	{
		return LogError("Failed to load the Server Script: %s", ScriptFile);
	}

	if (ForceInit)
	{
		InitScript();
	}

	Log("Server Script Loaded");
}

// -------------------------------------
void Server::InitScript() {

	Log("Initializing Server Script..");

	if (!m_ScriptLoaded)
	{
		return LogWarning("Script not loaded: Cannot Initialize!");
	}

	bool Success(false);
	m_ScriptInitialized = (GetEvents()->Get("ServerInit.InitServer", Success) && Success);

	if (!m_ScriptInitialized)
	{
		return LogWarning("Failed to Initialize the Script!");
	}

	Log("Server Script Initialized");
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
	// FIXME: Experimental!
	//gServer->LoadScript(true);
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
		Log("Channel created? event.m_val=%d", event.m_value);
		break;

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

	LoadScript(true);

	if (m_pSS) 
	{
		m_pSS->SetGlobalValue("MAP_START_TIME", gEnv->pTimer->GetCurrTime());
		if (IsLuaReady()) {
			GetEvents()->Call("ServerRPC.Callbacks.OnMapStarted");
		}
	}

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
	else if (IsLuaReady()) {


	}

	// Fixed entity names for respawning entities
	const char* respawn_name;
	IScriptTable* pProperties = params.pPropertiesTable;
	if (pProperties)
	{
		Log("found props..");
		if (pProperties->GetValue("sRespawnName", respawn_name)) 
		{
			params.sName = respawn_name;
			Log("Fixed name for Entity %s", respawn_name);
		}
	}

	return true;
}


// -------------------------------------
void Server::OnSpawn(IEntity* pEntity, SEntitySpawnParams& params)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	// For -oldgame
	if (!g_pGame)
	{
		return;
	}

	//Log("name=%s", params.sName);
	//Log("class=%s", params.pClass->GetName());
	/*if (strcmp(params.sName, "GameRules") == 0 && (strcmp(params.pClass->GetName(), "PowerStruggle") + strcmp(params.pClass->GetName(), "InstantAction")) > 0)
	{
		gServer->LoadScript(true);
	}*/

	// =====================================
	if (IsLuaReady())
	{

		bool vehicle = false;
		bool item = false;
		bool ignore = false;
		if (IEntityClass* pClass = params.pClass)
		{

			// Vehicle
			if (m_pGameFramework->GetIVehicleSystem()->IsVehicleClass(pClass->GetName()))
			{
				vehicle = true;
				//GetEvents()->Call("ServerRPC.Callbacks.OnVehicleSpawn", ScriptHandle(pEntity->GetId()));
			}

			// Items
			if (m_pGameFramework->GetIItemSystem()->IsItemClass(pClass->GetName())) 
			{
				item = true;
			}

			// Projectiles
			if (g_pGame->GetWeaponSystem()->GetProjectile(pEntity->GetId()) != 0)
			{
				ignore = true;
			}
		}

		if (!ignore)
		{
			if (IScriptTable* pScriptTable = pEntity->GetScriptTable())
			{
				GetEvents()->Call("ServerRPC.Callbacks.OnEntitySpawn", pScriptTable, ScriptHandle(pEntity->GetId()), vehicle, item);
			}
		}
	}

	// =====================================
	m_spawnCounter++;
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
	if (IsLuaReady()) 
	{
		m_pSS->SetGlobalValue("DLL_ERROR", true);
		GetEvents()->Call("HandleError", error.c_str());
	}
}

// -------------------------------------
bool Server::UpdateGameSpyServerReport(EGameSpyUpdateType type, const char* key, const char* value, int index) {

	INetworkService* pGS = GetGSMaster();
	if (!pGS || pGS == nullptr)
		return false;


	IServerReport* pGSReport = pGS->GetServerReport();
	if (!pGSReport)
		return false;

	switch (type) 
	{
	case EGameSpyUpdateType::eGSUpdate_Server:
		pGSReport->SetServerValue(key, value);
		break;

	case EGameSpyUpdateType::eGSUpdate_Player:
		pGSReport->SetPlayerValue(index, key, value);
		break;

	case EGameSpyUpdateType::eGSUpdate_Team:
		pGSReport->SetTeamValue(index, key, value);
		break;

	default:
		Log("Unknown type specified to UpdateGameSpyServerReport(): %d", type);
		break;
	}

	pGSReport->Update();
	return true;
}

// -------------------------------------
void Server::InitGameOverwrites(const std::string& filename) {

	std::ifstream file(filename);
	std::string line;

	if (!file.is_open()) {
		LogError( "Failed to open %s", filename.c_str());
		return;
	}

	std::string root = m_rootDir.filename().string();

	while (std::getline(file, line)) {
		// Find the '=' character
		size_t pos = line.find('=');
		if (pos != std::string::npos) {
			// Split the line into STRING1 and STRING2
			std::string game_original = line.substr(0, pos);
			std::string game_replace = line.substr(pos + 1);

			// Trim any whitespace around the strings (optional)
			game_original.erase(0, game_original.find_first_not_of(" \t"));
			game_original.erase(game_original.find_last_not_of(" \t") + 1);
			game_replace.erase(0, game_replace.find_first_not_of(" \t"));
			game_replace.erase(game_replace.find_last_not_of(" \t") + 1);

			// Add to the vector
			//Log("Added overwrite: %s = %s", game_original.c_str(), game_replace.c_str());
			//m_pOverwriteSF.push_back(std::make_pair(game_original, std::string(/*"../" +*/ root + "/" + game_replace)));
			m_pOverwriteSF.push_back(std::make_pair(game_original, std::string(m_rootDir.string() + "/" + game_replace))); // ned to make it a full path!
		}
	}

	file.close();
}

// -------------------------------------
void Server::OnScriptLoaded(const std::string& fileName)
{

	if (!IsLuaReady())
	{
		return;
	}

	GetEvents()->Call("ServerRPC.Callbacks.OnScriptLoaded", fileName.c_str());
}


// -------------------------------------
bool Server::OverwriteScriptPath( std::string &output, const std::string& input) {
	

	std::string lowerInput = input;
	std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);


	// Find the element with the matching key in the vector
	auto it = std::find_if(m_pOverwriteSF.begin(), m_pOverwriteSF.end(),
		[&lowerInput](const std::pair<std::string, std::string>& pair) {
			//CryLogAlways("compare %-30s->%s", pair.first.c_str(), lowerInput.c_str());

			std::string lowerFirst = pair.first;
			std::transform(lowerFirst.begin(), lowerFirst.end(), lowerFirst.begin(), ::tolower);


			return lowerFirst == lowerInput;
		});

	// If found, overwrite the path
	if (it != m_pOverwriteSF.end()) {
		output = it->second;  // Assign the new value as needed
		return true;
	}

	// If not found, you may want to handle it differently
	return false;

}

// -------------------------------------
void Server::Log(const char* format, ...) {
	va_list args;
	va_start(args, format);
	ServerLog::Log("$9[$4CryMP-Server$9] ", format, args);
	va_end(args);
}

// -------------------------------------
void Server::LogError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	ServerLog::Log("$9[$4CryMP-Server$9] Error: ", format, args);
	va_end(args);
}

// -------------------------------------
void Server::LogWarning(const char* format, ...) {
	va_list args;
	va_start(args, format);
	ServerLog::Log("$9[$4CryMP-Server$9] Warning: ", format, args);
	va_end(args);
}