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

#include "..\Server\Server.h"
#include "ServerPublisher.h"
#include "CryMP/Common/HTTPClient.h"

#include "CryCommon/CrySystem/IConsole.h"
#include "CryGame/Game.h"
#include "CryGame/GameRules.h"

#include <string>
#include <format>

// -------------------------------------
void Callback_Expose(HTTPClientResult& result) {

	/*ServerPublisher* pSvPbl = gServer->GetServerPublisher();

	if (result.code != 200) {
		pSvPbl->LogError("Expose Server failed. Code %d (%s)", result.code, result.error.c_str());
		return;
	}

	std::string response = result.response;
	pSvPbl->Log("HTTP Response: ", result.response.c_str());

	if (response == "FAIL") {
		pSvPbl->LogError("Expose Failed.");
		return;
	}

	std::string cookie = pSvPbl->ExtractCookie(response);
	if (cookie.empty()) {
		pSvPbl->LogError("Expose Failed. Unable to exctract the cookie");
		return;
	}

	pSvPbl->Log("Cookie for current Session is %s", cookie.c_str());
	pSvPbl->SetCookie(cookie);
	pSvPbl->OnExposed(true);*/
}

// ------------------------------------------------------------
void ServerPublisher::Init(ISystem *pSystem) {

	// ----------------
	// Log
	Log("Init()");

	// ----------------
	if (m_pTimer == nullptr)
		m_pTimer = pSystem->GetITimer();
}

// ------------------------------------------------------------
void ServerPublisher::ExposeServer() {
	
	// -----------------------
	if (m_bExposed == true && m_bExposedSuccess == false)
		return; // Currently trying!

	/*
	IConsole* pConsole = gEnv->pConsole;

	// -----------------------
	std::string url = gServer->GetMasterServerAPI(std::string("")) + "/reg.php?";
	HTTPClientRequest request;

	// -----------------------
	bool svpass = (strlen(gEnv->pConsole->GetCVar("sv_password")->GetString()) > 0);
	const char* level = g_pGame->GetIGameFramework()->GetLevelName();

	// -----------------------
	int         port   = pConsole->GetCVar("sv_port")->GetIVal();			// PORT
	int         maxpl  = pConsole->GetCVar("sv_maxplayers")->GetIVal();     // Max Players
	int         numpl  = 0;													// Num Players
	std::string name   = pConsole->GetCVar("sv_servername")->GetString();   // Server Name
	std::string pass   = (svpass ? "true" : "");							// Server Pass (1, 0)
	std::string map    = (level ? level : "");								// Map
	int         timel  = (int)g_pGame->GetGameRules()->GetRemainingGameTime();   // Time Limit
	std::string mapdl  = GetMapDownloadLink();   // Map Download
	std::string ver    = "6156";				 // Version
	int         ranked = pConsole->GetCVar("sv_Ranked")->GetIVal();    // Ranked (1, 0)
	std::string local  = GetLocalIP();   // Local IP
	std::string desc   = GetServerDescription();   // Server Description

	std::string body = std::format(
		"port={}&maxpl={}&numpl={}&name={}&pass={}&map={}&timel={}&mapdl={}&ver={}&ranked={}&local={}&desc={}",
		port,
		maxpl,
		numpl,
		HTTP::URLEncode(name), 
		HTTP::URLEncode(pass), 
		HTTP::URLEncode(map), 
		timel,
		HTTP::URLEncode(mapdl),
		HTTP::URLEncode(ver), 
		ranked,
		HTTP::URLEncode(local), 
		HTTP::URLEncode(desc)
	);

	// -----------------------
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "application/x-www-form-urlencoded; charset=utf-8";

	// -----------------------
	request.url = url;
	request.method = "GET";
	request.headers = headers;
	request.data = body;
	request.timeout = 12000;
	request.callback = Callback_Expose;

	// -----------------------
	Log("Announcing Server at Master Server..");
	gServer->HttpRequest(std::move(request));*/

	Script::CallMethod(m_pLuaPart, "ExposeServer");

	m_bExposed = true;
	m_bExposedSuccess = false; // set to true in the callback
}

// ------------------------------------------------------------
std::string ServerPublisher::ExtractCookie(std::string inputCookie) {
	const std::string startM = "<<Cookie>>";
	const std::string endM = "<<";

	std::size_t startPos = inputCookie.find(startM);
	if (startPos == std::string::npos) {
		return "";
	}

	startPos += startM.length();

	std::size_t endPos = inputCookie.find(endM, startPos);
	if (endPos == std::string::npos) {
		return "";
	}

	std::string cookie = inputCookie.substr(startPos, endPos - startPos);
	Log("inputCookie=%s", inputCookie.c_str());
	Log("startPos=%d", startPos);
	Log("endPos=%d", endPos);
	Log("cookie=%s", cookie.c_str());
	return cookie;
}

// ------------------------------------------------------------
void ServerPublisher::Update(float frametime) {
	
	if (m_bExposed && m_bExposedSuccess) {
		if (!m_cookie.empty()) {
			if (m_lastUpdate - m_pTimer->GetCurrTime() > m_updateRate) {
				UpdateNow(frametime);
				m_lastUpdate += m_updateRate;
			}
		}
	}
	else if (g_pGame->GetGameRules() != nullptr) {
		if (gServer->IsLuaReady())
			ExposeServer();
	}
}

// ------------------------------------------------------------
void ServerPublisher::UpdateNow(float frametime) {
	
	if (!m_bExposed) {
		return;
	}

	Log("Update now!!");
}
// ------------------------------------------------------------
std::string ServerPublisher::GetMapDownloadLink() {
	
	std::string link;
	return link;
}

// ------------------------------------------------------------
std::string ServerPublisher::GatherPlayers() {

	std::string players;
	return players;
}

// ------------------------------------------------------------
std::string ServerPublisher::GetLocalIP() {

	std::string ip = "127.0.0.1";
	return ip;
}

// ------------------------------------------------------------
std::string ServerPublisher::GetServerDescription() {

	std::string description = "ATOM Configuration failed to load";

	if (SmartScriptTable pATOMLua = gServer->GetATOMLua()) {
		// Implementation missing
	}

	return description;
}

// ------------------------------------------------------------
void ServerPublisher::Log(const char* format, ...) {
	va_list args;
	va_start(args, format);
	gServer->Log(("$9($3ServerPublisher$9) " + std::string(format)).c_str(), args);
	va_end(args);
}

// ------------------------------------------------------------
void ServerPublisher::LogError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	gServer->LogError(("$9($3ServerPublisher$9) " + std::string(format)).c_str(), args);
	va_end(args);
}

// ------------------------------------------------------------
void ServerPublisher::LogWarning(const char* format, ...) {
	va_list args;
	va_start(args, format);
	gServer->LogWarning(("$9($3ServerPublisher$9) " + std::string(format)).c_str(), args);
	va_end(args);
}