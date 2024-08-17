#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CrySystem/ICryPak.h"
#include "CryCommon/CrySystem/IConsole.h"
#include "CryCommon/Cry3DEngine/IMaterial.h"
#include "CryCommon/CryEntitySystem/IEntitySystem.h"
#include "CryCommon/CryAction/IVehicleSystem.h"
#include "CrySystem/LocalizationManager.h"
#include "CrySystem/RandomGenerator.h"
#include "Library/StringTools.h"
#include "Library/Util.h"
#include "Library/WinAPI.h"

// -------------------------------------
#include "ScriptBind_Server.h"
#include "Server.h"
#include "ServerEvents.h"
#include "CryGame/GameActions.h"
#include "CryGame/Actors/Actor.h"

// -------------------------------------
ScriptBind_Server::ScriptBind_Server()
{
	Init(gEnv->pScriptSystem, gEnv->pSystem);
	SetGlobalName("ServerDLL");

#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &ScriptBind_Server::

	SCRIPT_REG_GLOBAL(SV_EVENT_ON_UPDATE);
	SCRIPT_REG_GLOBAL(SV_EVENT_ON_SPAWN);
	SCRIPT_REG_GLOBAL(SV_EVENT_ON_GAME_RULES_CREATED);
	// Client
	//SCRIPT_REG_GLOBAL(SCRIPT_CALLBACK_ON_LOADING_START);
	//SCRIPT_REG_GLOBAL(SCRIPT_CALLBACK_ON_DISCONNECT);
	//SCRIPT_REG_GLOBAL(SCRIPT_CALLBACK_ON_BECOME_LOCAL_ACTOR);
	//SCRIPT_REG_GLOBAL(SV_EVENT_ON_MASTER_RESOLVED);

	// New
	RegisterGlobal("eGSUpdate_Server", (int)EGameSpyUpdateType::eGSUpdate_Server);
	RegisterGlobal("eGSUpdate_Player", (int)EGameSpyUpdateType::eGSUpdate_Player);
	RegisterGlobal("eGSUpdate_Team",   (int)EGameSpyUpdateType::eGSUpdate_Team);

	// CryMP
	SCRIPT_REG_TEMPLFUNC(GetMapName, "");
	SCRIPT_REG_TEMPLFUNC(Random, "");
	SCRIPT_REG_TEMPLFUNC(SetCallback, "callback, handler");
	SCRIPT_REG_TEMPLFUNC(SHA256, "text");
	SCRIPT_REG_TEMPLFUNC(URLEncode, "text");
	SCRIPT_REG_TEMPLFUNC(GetMasters, "");
	SCRIPT_REG_TEMPLFUNC(GetModelFilePath, "entityId, slot");
	SCRIPT_REG_FUNC(GetLP);
	SCRIPT_REG_FUNC(GetNumVars);
	SCRIPT_REG_FUNC(GetVars);

	// SS
	SCRIPT_REG_TEMPLFUNC(SetScriptErrorLog, "mode");

	// New
	SCRIPT_REG_FUNC(GetRoot);
	SCRIPT_REG_FUNC(GetWorkingDir);

	// Sv
	SCRIPT_REG_FUNC(IsDedicated);
	SCRIPT_REG_FUNC(IsMultiplayer);
	SCRIPT_REG_FUNC(IsClient);
	SCRIPT_REG_TEMPLFUNC(SetDedicated, "mode");
	SCRIPT_REG_TEMPLFUNC(SetMultiplayer, "mode");
	SCRIPT_REG_TEMPLFUNC(SetClient, "mode");
	SCRIPT_REG_TEMPLFUNC(SetServer, "mode");
	SCRIPT_REG_FUNC(IsServer);
	SCRIPT_REG_FUNC(GetMemUsage);
	SCRIPT_REG_FUNC(GetMemPeak);
	SCRIPT_REG_FUNC(GetCPUUsage);
	SCRIPT_REG_FUNC(GetCPUName);
	SCRIPT_REG_TEMPLFUNC(GetItemCategory, "item");
	SCRIPT_REG_TEMPLFUNC(GetLevels, "");   // Returns all available levels
	SCRIPT_REG_TEMPLFUNC(IsValidEntityClass, "class");   // Returns true if specified entity class is valid (exists)
	SCRIPT_REG_TEMPLFUNC(IsValidItemClass, "class");   // Returns true if specified entity class is valid (exists)
	SCRIPT_REG_FUNC(GetEntityClasses);   // Returns true if specified entity class is valid (exists)
	SCRIPT_REG_FUNC(GetItemClasses);   // Returns true if specified entity class is valid (exists)
	SCRIPT_REG_FUNC(GetVehicleClasses);   // Returns true if specified entity class is valid (exists)

	// Network
	SCRIPT_REG_TEMPLFUNC(Request, "params, callback");
	SCRIPT_REG_FUNC(GetMasterServerAPI);
	SCRIPT_REG_TEMPLFUNC(SetChannelNick, "channel, name");
	SCRIPT_REG_TEMPLFUNC(GetChannelNick, "channel");
	SCRIPT_REG_TEMPLFUNC(GetChannelIP, "channel");
	SCRIPT_REG_TEMPLFUNC(GetChannelName, "channel");
	SCRIPT_REG_TEMPLFUNC(KickChannel, "type, channel,reason");
	SCRIPT_REG_TEMPLFUNC(UpdateGameSpyReport, "type, key, val");

	SCRIPT_REG_TEMPLFUNC(FSetCVar, "cvar, value");
}

// -------------------------------------
ScriptBind_Server::~ScriptBind_Server()
{
}

// --------------------------------------------------------------------------------
// Describe this
int ScriptBind_Server::FSetCVar(IFunctionHandler* pH, const char* cvar, const char* value)
{
	ICVar* pCVar = gEnv->pConsole->GetCVar(cvar);
	if (!pCVar)
	{
		return pH->EndFunction(false);
	}

	const std::string previousVal = pCVar->GetString();

	if (previousVal != std::string_view(value))
	{
		pCVar->ForceSet(value);

		// CVar still the same, is it synced?
		if (previousVal == std::string_view(pCVar->GetString()))
		{
			const int previousFlags = pCVar->GetFlags();

			// disable sync
			pCVar->SetFlags(VF_NOT_NET_SYNCED);

			// 2nd attempt
			pCVar->ForceSet(value);

			// now restore the flags
			pCVar->SetFlags(previousFlags);
			// CVar value won't change untill server changes it to something else

			if (std::string_view(value) != std::string_view(pCVar->GetString()))
			{
				CryLogAlways("$4[CryMP] Failed to change CVar %s - value still %s", cvar, pCVar->GetString());
				return pH->EndFunction(false);
			}
		}
	}

	// all good!
	return pH->EndFunction(true);
}

// --------------------------------------------------------------------------------
// Describe this
int ScriptBind_Server::GetItemClasses(IFunctionHandler* pH) {

	SmartScriptTable pClasses = gEnv->pScriptSystem->CreateTable();
	IEntityClass* pClass = NULL;

	IItemSystem* pItemSystem = gEnv->pGame->GetIGameFramework()->GetIItemSystem();
	if (!pItemSystem)
		return pH->EndFunction();


	IEntityClassRegistry* pEntityRegistry = gEnv->pEntitySystem->GetClassRegistry();
	if (!pEntityRegistry)
		return pH->EndFunction();

	for (pEntityRegistry->IteratorMoveFirst(); pClass = pEntityRegistry->IteratorNext();)
	{
		if (pClass != NULL) {
			if (pItemSystem->IsItemClass(pClass->GetName()))
				pClasses->PushBack(pClass->GetName());

		}
	}

	return pH->EndFunction(pClasses);
}


// --------------------------------------------------------------------------------
// Describe this
int ScriptBind_Server::GetVehicleClasses(IFunctionHandler* pH) {

	SmartScriptTable pClasses = gEnv->pScriptSystem->CreateTable();
	IEntityClass* pClass = NULL;

	IVehicleSystem* pVehicleSystem = gEnv->pGame->GetIGameFramework()->GetIVehicleSystem();
	if (!pVehicleSystem)
		return pH->EndFunction();


	IEntityClassRegistry* pEntityRegistry = gEnv->pEntitySystem->GetClassRegistry();
	if (!pEntityRegistry)
		return pH->EndFunction();

	for (pEntityRegistry->IteratorMoveFirst(); pClass = pEntityRegistry->IteratorNext();)
	{
		if (pClass != NULL) {
			if (pVehicleSystem->IsVehicleClass(pClass->GetName()))
				pClasses->PushBack(pClass->GetName());

		}
	}

	return pH->EndFunction(pClasses);
}

// --------------------------------------------------------------------------------
// Describe this
int ScriptBind_Server::GetEntityClasses(IFunctionHandler* pH) {

	SmartScriptTable pClasses = gEnv->pScriptSystem->CreateTable();
	IEntityClass* pClass = NULL;

	IEntityClassRegistry* pEntityRegistry = gEnv->pEntitySystem->GetClassRegistry();
	if (!pEntityRegistry)
		return pH->EndFunction();

	for (pEntityRegistry->IteratorMoveFirst(); pClass = pEntityRegistry->IteratorNext();)
	{
		if (pClass != NULL) {
			pClasses->PushBack(pClass->GetName());
		}
		else
			break;
	}

	return pH->EndFunction(pClasses);
}

// --------------------------------------------------------------------------------
// Describe this
int ScriptBind_Server::IsValidEntityClass(IFunctionHandler* pH, const char* name) {

	if (!name)
		return pH->EndFunction(false);

	IEntityClass* pEntityClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(name);
	return pH->EndFunction((pEntityClass != 0));
}

// --------------------------------------------------------------------------------
// Describe this
int ScriptBind_Server::IsValidItemClass(IFunctionHandler* pH, const char* name) {

	if (!name)
		return pH->EndFunction(false);
	;

	return pH->EndFunction(gEnv->pGame->GetIGameFramework()->GetIItemSystem()->IsItemClass(name));
}

// --------------------------------------------------------------------------------
// Describe this
int ScriptBind_Server::GetLevels(IFunctionHandler* pH) {

	ILevelSystem* pLevelSystem = gEnv->pGame->GetIGameFramework()->GetILevelSystem();
	const int levelCount = pLevelSystem->GetLevelCount();
	SmartScriptTable lvlTable(gEnv->pScriptSystem->CreateTable());

	for (int i = 0; i < levelCount; i++)
	{
		const ILevelInfo* pLevelInfo = pLevelSystem->GetLevelInfo(i);

		SmartScriptTable lvTable(gEnv->pScriptSystem->CreateTable());
		lvTable->PushBack(pLevelInfo->GetName());

		for (int j = 0; j < pLevelInfo->GetGameTypeCount(); j++)
			lvTable->PushBack(pLevelInfo->GetGameType(j)->name.c_str());
		//	lvTable->PushBack(pLevelInfo->GetGameType(j));

		lvlTable->PushBack(lvTable);
		// ...
	}
	return pH->EndFunction(lvlTable);
}

// -------------------------------------
int ScriptBind_Server::GetItemCategory(IFunctionHandler* pH, const char* item)
{
	IItemSystem* pItemSystem = gEnv->pGame->GetIGameFramework()->GetIItemSystem();
	if (!pItemSystem)
		return pH->EndFunction("");

	return pH->EndFunction(pItemSystem->GetItemCategory(item));
}

// -------------------------------------
int ScriptBind_Server::GetMemUsage(IFunctionHandler* pH)
{
	return pH->EndFunction(gServer->GetStats()->GetMemUsage());
}

// -------------------------------------
int ScriptBind_Server::GetMemPeak(IFunctionHandler* pH)
{
	return pH->EndFunction(gServer->GetStats()->GetMemPeak());
}

// -------------------------------------
int ScriptBind_Server::GetCPUUsage(IFunctionHandler* pH)
{
	return pH->EndFunction(gServer->GetStats()->GetCPUUsage());
}

// -------------------------------------
int ScriptBind_Server::GetCPUName(IFunctionHandler* pH)
{
	return pH->EndFunction(gServer->GetStats()->GetCPUName().c_str());
}

// -------------------------------------
int ScriptBind_Server::SetScriptErrorLog(IFunctionHandler* pH, bool mode)
{
	gScriptSystem->m_print_errors = mode;
	return pH->EndFunction();
}

// -------------------------------------
int ScriptBind_Server::IsDedicated(IFunctionHandler* pH)
{
	return pH->EndFunction(gEnv->pSystem->IsDedicated());
}

// -------------------------------------
int ScriptBind_Server::IsMultiplayer(IFunctionHandler* pH)
{
	return pH->EndFunction(gEnv->bMultiplayer);
}

// -------------------------------------
int ScriptBind_Server::IsServer(IFunctionHandler* pH)
{
	return pH->EndFunction(gEnv->bServer);
}

// -------------------------------------
int ScriptBind_Server::IsClient(IFunctionHandler* pH)
{
	return pH->EndFunction(gEnv->bClient);
}

// -------------------------------------
int ScriptBind_Server::SetDedicated(IFunctionHandler* pH, bool mode)
{
	return pH->EndFunction();
}

// -------------------------------------
int ScriptBind_Server::SetMultiplayer(IFunctionHandler* pH, bool mode)
{
	gEnv->bMultiplayer = mode;
	return pH->EndFunction();
}

// -------------------------------------
int ScriptBind_Server::SetServer(IFunctionHandler* pH, bool mode)
{
	gEnv->bServer = mode;
	return pH->EndFunction();
}

// -------------------------------------
int ScriptBind_Server::SetClient(IFunctionHandler* pH, bool mode)
{
	gEnv->bClient = mode;
	return pH->EndFunction();
}

// -------------------------------------
int ScriptBind_Server::GetMapName(IFunctionHandler* pH)
{
	return pH->EndFunction(gServer->GetGameFramework()->GetLevelName());
}

// -------------------------------------
int ScriptBind_Server::Random(IFunctionHandler* pH)
{
	float min = 0;
	if (pH->GetParamCount() >= 1)
		min = pH->GetParam(1, min);

	float max = 1;
	if (pH->GetParamCount() >= 2)
		min = pH->GetParam(2, max);

	return pH->EndFunction(RandomGenerator::GenerateFloat(min, max));
}

// -------------------------------------
int ScriptBind_Server::Request(IFunctionHandler* pH, SmartScriptTable params, HSCRIPTFUNCTION callback)
{
	HTTPClientRequest request;

	{
		CScriptSetGetChain chain(params);

		const char* url = "";
		const char* method = "GET";
		const char* body = "";
		SmartScriptTable headers;
		int timeout = 4000;

		chain.GetValue("url", url); // 
		chain.GetValue("method", method); // POS
		chain.GetValue("body", body); // a=??&b=??
		chain.GetValue("headers", headers); // Content-Type = application/x-www-form-urlencoded; charset=utf-8
		chain.GetValue("timeout", timeout); // 30000

		request.url = url;
		request.method = method;
		request.data = body;
		request.timeout = timeout;

		if (headers)
		{
			auto it = headers->BeginIteration();
			while (headers->MoveNext(it))
			{
				if (it.sKey && it.value.GetVarType() == ScriptVarType::svtString)
				{
					request.headers[it.sKey] = it.value.str;
				}
			}
			headers->EndIteration(it);
		}
	}

	if (request.url.empty())
	{
		m_pSS->ReleaseFunc(callback);
		return pH->EndFunction(false, "url not provided");
	}

	request.callback = [callback, pSS = m_pSS](HTTPClientResult& result)
	{
		if (pSS->BeginCall(callback))
		{
			if (result.error.empty())
			{
				pSS->PushFuncParam(false);
			}
			else
			{
				pSS->PushFuncParam(result.error.c_str());
			}

			pSS->PushFuncParam(result.response.c_str());
			pSS->PushFuncParam(result.code);
			pSS->EndCall();
		}

		pSS->ReleaseFunc(callback);
	};

	gServer->HttpRequest(std::move(request));

	return pH->EndFunction(true);
}

// -------------------------------------
int ScriptBind_Server::SetCallback(IFunctionHandler* pH, int callback, HSCRIPTFUNCTION handler)
{
	bool success = gServer->GetEvents()->SetHandler(static_cast<EServerEvents>(callback), handler);

	if (!success)
		m_pSS->ReleaseFunc(handler);

	return pH->EndFunction(success);
}

// -------------------------------------
int ScriptBind_Server::SHA256(IFunctionHandler* pH, const char* text)
{
	return pH->EndFunction(Util::SHA256(text).c_str());
}

int ScriptBind_Server::URLEncode(IFunctionHandler* pH, const char* text)
{
	return pH->EndFunction(HTTP::URLEncode(text).c_str());
}

// -------------------------------------
int ScriptBind_Server::GetMasters(IFunctionHandler* pH)
{
	SmartScriptTable masters(m_pSS);

	for (const std::string& master : gServer->GetMasters())
	{
		masters->PushBack(master.c_str());
	}

	return pH->EndFunction(masters);
}

// -------------------------------------
int ScriptBind_Server::GetMasterServerAPI(IFunctionHandler* pH)
{
	return pH->EndFunction(gServer->GetMasterServerAPI(std::string("")).c_str());
}

// -------------------------------------
int ScriptBind_Server::GetModelFilePath(IFunctionHandler* pH, ScriptHandle entityId, int slot)
{
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityId.n);
	if (!pEntity)
		return pH->EndFunction();

	if (ICharacterInstance* pCharacter = pEntity->GetCharacter(slot))
	{
		return pH->EndFunction(pCharacter->GetFilePath());
	}
	if (IStatObj* pObject = pEntity->GetStatObj(slot))
	{
		return pH->EndFunction(pObject->GetFilePath());
	}
	return pH->EndFunction();
}

// -------------------------------------
int ScriptBind_Server::GetLP(IFunctionHandler* pH)
{
	SmartScriptTable pks(m_pSS);
	ICryPak::PakInfo* pPakInfo = gEnv->pCryPak->GetPakInfo();
	size_t openPakSize = 0;
	for (uint32 pak = 0; pak < pPakInfo->numOpenPaks; pak++)
	{
		const auto& c = pPakInfo->arrPaks[pak];
		std::string path = StringTools::ToLower(c.szFilePath);
		pks->SetValue(path.c_str(), static_cast<int>(c.nUsedMem));
	}
	gEnv->pCryPak->FreePakInfo(pPakInfo);
	return pH->EndFunction(pks);
}

// -------------------------------------
int ScriptBind_Server::GetNumVars(IFunctionHandler* pH)
{
	return pH->EndFunction(gEnv->pConsole->GetNumVars());
}

// -------------------------------------
int ScriptBind_Server::GetVars(IFunctionHandler* pH)
{
	std::vector<const char*> cmds;

	cmds.resize(gEnv->pConsole->GetNumVars());
	gEnv->pConsole->GetSortedVars(&cmds[0], cmds.size());

	SmartScriptTable vars(m_pSS);
	for (const char* vName : cmds)
	{
		vars->PushBack(vName);
	}
	return pH->EndFunction(vars);
}


// -------------------------------------
int ScriptBind_Server::GetRoot(IFunctionHandler* pH)
{
	return pH->EndFunction(gServer->GetRoot().c_str());
}


// -------------------------------------
int ScriptBind_Server::GetWorkingDir(IFunctionHandler* pH)
{
	return pH->EndFunction(gServer->GetWorkingDir().c_str());
}

// -------------------------------------
int ScriptBind_Server::SetChannelNick(IFunctionHandler* pH, int channelId, const char* name)
{
	if (INetChannel* pChannel = g_pGame->GetIGameFramework()->GetNetChannel(channelId)) {
		pChannel->SetNickname(name);
		return pH->EndFunction(true);
	}

	return pH->EndFunction(false);
}

// -------------------------------------
int ScriptBind_Server::GetChannelNick(IFunctionHandler* pH, int channelId)
{
	if (INetChannel* pChannel = g_pGame->GetIGameFramework()->GetNetChannel(channelId)) {
		return pH->EndFunction(pChannel->GetNickname());
	}

	return pH->EndFunction();
}


// -------------------------------------
int ScriptBind_Server::GetChannelIP(IFunctionHandler* pH, int channelId)
{
	if (INetChannel* pChannel = g_pGame->GetIGameFramework()->GetNetChannel(channelId)) {
		return pH->EndFunction(gServer->GetUtils()->CryChannelToIP(channelId).c_str());
	}

	return pH->EndFunction();
}

// -------------------------------------
int ScriptBind_Server::GetChannelName(IFunctionHandler* pH, int channelId)
{
	if (INetChannel* pChannel = g_pGame->GetIGameFramework()->GetNetChannel(channelId)) {
		return pH->EndFunction(pChannel->GetName());
	}

	return pH->EndFunction();
}

// -------------------------------------
int ScriptBind_Server::KickChannel(IFunctionHandler* pH, int type, int channelId, const char*reason)
{
	if (INetChannel* pChannel = g_pGame->GetIGameFramework()->GetNetChannel(channelId)) {
		pChannel->Disconnect((EDisconnectionCause)type, reason);
	}

	return pH->EndFunction();
}


// -------------------------------------
int ScriptBind_Server::UpdateGameSpyReport(IFunctionHandler* pH, int type, const char* key, const char* val)
{
	int index = 0;
	if (pH->GetParamCount() >= 4)
		pH->GetParam(4, index);

	gServer->UpdateGameSpyServerReport((EGameSpyUpdateType)type, key, val, index);
	return pH->EndFunction();
}
