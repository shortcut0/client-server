#include "CryMP/Common/Executor.h"
#include "CryMP/Common/HTTPClient.h"
#include "CryCommon/CrySystem/IConsole.h"
#include "CryCommon/CrySystem/ISystem.h"
#include "CryGame/Game.h"

#include "Server.h"

Server::Server()
{
}

Server::~Server()
{
}

void Server::Init(IGameFramework* pGameFramework)
{
	this->pGameFramework = pGameFramework;

	this->pExecutor = std::make_unique<Executor>();
	this->pHttpClient = std::make_unique<HTTPClient>(*this->pExecutor);

	pGameFramework->RegisterListener(this, "crymp-server", FRAMEWORKLISTENERPRIORITY_DEFAULT);

	// initialize the game
	// mods are not supported
	g_pGame = new CGame();
	g_pGame->Init(pGameFramework);
}

void Server::UpdateLoop()
{
	gEnv->pConsole->ExecuteString("exec autoexec.cfg");

	const bool haveFocus = true;
	const unsigned int updateFlags = 0;

	while (g_pGame->Update(haveFocus, updateFlags))
	{
	}
}

void Server::OnPostUpdate(float deltaTime)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	this->pExecutor->OnUpdate();
}

void Server::OnSaveGame(ISaveGame* saveGame)
{
}

void Server::OnLoadGame(ILoadGame* loadGame)
{
}

void Server::OnLevelEnd(const char* nextLevel)
{
}

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
