#pragma once

#ifndef  __CVAR_CALLBACKS__
#define  __CVAR_CALLBACKS__

#include "Server.h"
#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CrySystem/IConsole.h"

// -----------------------------------------
// sv_maxplayers

void CCALLBACK_MaxPlayers(ICVar* pCVar) {
	int num = pCVar->GetIVal();
	if (num > 99)
		gServer->LogWarning("Max Players >99 won't be displayed correctl.");

	gServer->UpdateGameSpyServerReport(EGameSpyUpdateType::eGSUpdate_Server, "maxplayers", pCVar->GetString());
}

// -----------------------------------------
// sv_servername

void CCALLBACK_ServerName(ICVar* pCVar) {
	gServer->UpdateGameSpyServerReport(EGameSpyUpdateType::eGSUpdate_Server, "hostname", pCVar->GetString());
}

// -----------------------------------------
// sv_servername

void CCALLBACK_Map(ICVar* pCVar) {
	gServer->GetEvents()->Call("ServerRPC.Callbacks.OnMapCommand", pCVar->GetString());
}

#endif // ! __CVAR_CALLBACKS__
