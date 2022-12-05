#include <stdexcept>
#include <string>
#include <system_error>

#include "Library/WinAPI.h"

#include "Launcher.h"

////////////////////////////////////////////////////////////////////////////////
// Request fast graphics card
////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

////////////////////////////////////////////////////////////////////////////////

static std::string RuntimeErrorToString(const std::runtime_error& error)
{
	std::string message = error.what();
	std::string::size_type pos = 0;

	while ((pos = message.find(": ", pos)) != std::string::npos)
	{
		message.replace(pos, 2, "\n");
	}

	return message;
}

static std::string SystemErrorToString(const std::system_error& error)
{
	std::string message = RuntimeErrorToString(error);

	message += "\n\nError code ";
	message += std::to_string(error.code().value());

	return message;
}

int __stdcall WinMain(void*, void*, char*, int)
{
	Launcher launcher;
	gLauncher = &launcher;

	try
	{
		launcher.Run();
	}
	catch (const std::system_error& error)
	{
		WinAPI::ErrorBox(SystemErrorToString(error).c_str());
		return 1;
	}
	catch (const std::runtime_error& error)
	{
		WinAPI::ErrorBox(RuntimeErrorToString(error).c_str());
		return 1;
	}
	catch (const std::exception& ex)
	{
		WinAPI::ErrorBox(ex.what());
		return 1;
	}

	return 0;
}
