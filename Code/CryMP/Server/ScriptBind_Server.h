#pragma once
#include "CryCommon/CryScriptSystem/IScriptSystem.h"

class ScriptBind_Server : public CScriptableBase
{
public:
	ScriptBind_Server();
	~ScriptBind_Server();

	// CryMP
	int GetMapName(IFunctionHandler* pH);
	int Random(IFunctionHandler* pH);
	int Request(IFunctionHandler* pH, SmartScriptTable params, HSCRIPTFUNCTION callback);
	int SetCallback(IFunctionHandler* pH, int callback, HSCRIPTFUNCTION handler);
	int SHA256(IFunctionHandler* pH, const char* text);
	int URLEncode(IFunctionHandler* pH, const char* text);
	int GetMasters(IFunctionHandler* pH);
	int GetModelFilePath(IFunctionHandler* pH, ScriptHandle entityId, int slot);
	int GetLP(IFunctionHandler* pH);
	int GetNumVars(IFunctionHandler* pH);
	int GetVars(IFunctionHandler* pH);

	// New
	int GetRoot(IFunctionHandler* pH);
	int GetWorkingDir(IFunctionHandler* pH);
	int GetMasterServerAPI(IFunctionHandler* pH);
	int SetChannelNick(IFunctionHandler* pH, int channelId, const char* name);
	int GetChannelNick(IFunctionHandler* pH, int channelId);
	int GetChannelIP(IFunctionHandler* pH, int channelId);
	int GetChannelName(IFunctionHandler* pH, int channelId);
	int KickChannel(IFunctionHandler* pH, int type, int channelId, const char* reason);
	int UpdateGameSpyReport(IFunctionHandler* pH, int type, const char* key, const char* val);

	// Server
	int IsDedicated(IFunctionHandler* pH);
	int IsMultiplayer(IFunctionHandler* pH);
	int IsServer(IFunctionHandler* pH);
	int IsClient(IFunctionHandler* pH);
	int SetDedicated(IFunctionHandler* pH, bool mode);
	int SetMultiplayer(IFunctionHandler* pH, bool mode);
	int SetServer(IFunctionHandler* pH, bool mode);
	int SetClient(IFunctionHandler* pH, bool mode);
	int SetScriptErrorLog(IFunctionHandler* pH, bool mode);
	int GetItemCategory(IFunctionHandler* pH, const char* item);
	int GetLevels(IFunctionHandler* pH);
	int GetEntityClasses(IFunctionHandler* pH);
	int GetItemClasses(IFunctionHandler* pH);
	int GetVehicleClasses(IFunctionHandler* pH);
	int IsValidEntityClass(IFunctionHandler* pH, const char* name);
	int IsValidItemClass(IFunctionHandler* pH, const char* name);
	int GetScriptPath(IFunctionHandler* pH, const char* sClass);
	int GetProjectileOwnerId(IFunctionHandler* pH, ScriptHandle id);
	int SetProjectileOwnerId(IFunctionHandler* pH, ScriptHandle id, ScriptHandle ownerId);
	int GetProjectilePos(IFunctionHandler* pH, ScriptHandle id);
	int SetProjectilePos(IFunctionHandler* pH, ScriptHandle id, Vec3 pos);
	int ExplodeProjectile(IFunctionHandler* pH, ScriptHandle id);
	int SetEntityScriptUpdateRate(IFunctionHandler* pH, ScriptHandle id, float rate);

	int RayWorldIntersection(IFunctionHandler* pH);

	int GetMemUsage(IFunctionHandler* pH);
	int GetMemPeak(IFunctionHandler* pH);
	int GetCPUUsage(IFunctionHandler* pH);
	int GetCPUName(IFunctionHandler* pH);

	int FSetCVar(IFunctionHandler* pH, const char* cvar, const char* value);
};
