#include "CryCommon/CrySystem/ISystem.h"
#include "Library/WinAPI.h"

#include "QuitHook.h"
#include "Server.h"

struct CSystemHook
{
    void Quit();
};

using QuitFunc = decltype(&CSystemHook::Quit);
static QuitFunc g_originalQuit = nullptr;

void CSystemHook::Quit()
{
    CryLogAlways("%s", __FUNCTION__);
    gServer->Quit();

   (this->*g_originalQuit)();
}

void QuitHook::Init()
{
    void** pSystemVTable = *reinterpret_cast<void***>(gEnv->pSystem);

    g_originalQuit = reinterpret_cast<QuitFunc&>(pSystemVTable[14]);

    // vtable hook
    QuitFunc newQuit = &CSystemHook::Quit;
    WinAPI::FillMem(&pSystemVTable[14], &reinterpret_cast<void*&>(newQuit), sizeof(void*));
}