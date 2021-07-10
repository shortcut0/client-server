#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CrySystem/ICryPak.h"
#include "CryCommon/CryAction/IGameFramework.h"
#include "CryCommon/CryAction/ILevelSystem.h"
#include "Library/Util.h"

#include "MapDownloader.h"
#include "MapExtractor.h"
#include "Client.h"
#include "FileDownloader.h"
#include "FileRedirector.h"

void MapDownloader::DownloadMap(MapDownloaderRequest && request)
{
	// convert "multiplayer/ps/mymap" to "mymap.zip"
	const std::string zipFileName = std::filesystem::path(request.map).filename().string() + ".zip";

	FileDownloaderRequest download;
	download.url = request.mapURL;
	download.filePath = m_downloadDir / zipFileName;

	CryLogAlways("$3[CryMP] [MapDownloader] Downloading map $6%s$3", request.map.c_str());
	CryLogAlways("$3[CryMP] [MapDownloader] From $6%s$3", download.url.c_str());
	CryLogAlways("$3[CryMP] [MapDownloader] To $6%s$3", download.filePath.string().c_str());

	download.onProgress = [callback = request.onProgress](FileDownloaderProgress & progress) -> bool
	{
		if (callback)
			return callback("Downloading map " + progress.ToString());
		else
			return true;
	};

	download.onComplete = [request = std::move(request), this](FileDownloaderResult & result)
	{
		bool success = false;

		if (result.error)
		{
			CryLogAlways("$4[CryMP] [MapDownloader] Download error: %s", result.error.what());
		}
		else if (result.canceled)
		{
			CryLogAlways("$6[CryMP] [MapDownloader] Download canceled!");
		}
		else if (result.statusCode != HTTP::STATUS_OK)
		{
			const int code = result.statusCode;
			const char *codeName = HTTP::StatusCodeToString(code);

			CryLogAlways("$4[CryMP] [MapDownloader] Bad status code %d (%s)!", code, codeName);
		}
		else if (result.downloadedBytes == 0)
		{
			CryLogAlways("$4[CryMP] [MapDownloader] Empty file!");
		}
		else if (result.contentLength && result.contentLength != result.downloadedBytes)
		{
			CryLogAlways("$4[CryMP] [MapDownloader] Incomplete file!");
		}
		else
		{
			CryLogAlways("$3[CryMP] [MapDownloader] Download finished, extracting...");
			if (request.onProgress)
				request.onProgress("Extracting map...");

			success = UnpackMap(result.filePath, request.map);

			if (success)
			{
				CryLogAlways("$3[CryMP] [MapDownloader] Map extracted, checking map folder...");
				if (request.onProgress)
					request.onProgress("Checking map folder...");

				RescanMaps();

				gClient->GetFileRedirector()->AddDownloadedMap(request.map);

				CryLogAlways("$3[CryMP] [MapDownloader] Map downloaded successfully");
			}
		}

		RemoveZip(result.filePath);

		CompleteRequest(request, success);
	};

	gClient->GetFileDownloader()->Request(std::move(download));
}

bool MapDownloader::UnpackMap(const std::filesystem::path & zipPath, const std::string_view & mapName)
{
	try
	{
		MapExtractor(zipPath, mapName).Extract();
	}
	catch (const std::exception & ex)
	{
		CryLogAlways("$4[CryMP] [MapDownloader] Extract error: %s", ex.what());
		return false;
	}

	return true;
}

void MapDownloader::RemoveZip(const std::filesystem::path & zipPath)
{
	CryLog("$3[CryMP] [MapDownloader] Removing %s", zipPath.string().c_str());

	// no exceptions
	std::error_code code;
	std::filesystem::remove(zipPath, code);
}

bool MapDownloader::CheckMapExists(const std::string_view & mapName)
{
	ILevelSystem *pLevelSystem = gClient->GetGameFramework()->GetILevelSystem();

	const int levelCount = pLevelSystem->GetLevelCount();

	for (int i = 0; i < levelCount; i++)
	{
		const ILevelInfo *pLevelInfo = pLevelSystem->GetLevelInfo(i);

		if (Util::EqualNoCase(mapName, pLevelInfo->GetName()))
		{
			return true;
		}
	}

	return false;
}

void MapDownloader::RescanMaps()
{
	gClient->GetGameFramework()->GetILevelSystem()->Rescan("%USER%/Downloads/Levels");
}

void MapDownloader::RegisterDownloadedMaps()
{
	ILevelSystem *pLevelSystem = gClient->GetGameFramework()->GetILevelSystem();
	FileRedirector *pFileRedirector = gClient->GetFileRedirector();

	const std::string_view userDir = gEnv->pCryPak->GetAlias("%USER");
	const int levelCount = pLevelSystem->GetLevelCount();

	for (int i = 0; i < levelCount; i++)
	{
		const ILevelInfo *pLevelInfo = pLevelSystem->GetLevelInfo(i);

		if (Util::StartsWith(userDir, pLevelInfo->GetPath()))
		{
			pFileRedirector->AddDownloadedMap(pLevelInfo->GetName());
		}
	}
}

void MapDownloader::CompleteRequest(const MapDownloaderRequest & request, bool success)
{
	if (request.onComplete)
	{
		MapDownloaderResult result;
		result.success = success;
		result.map = request.map;

		request.onComplete(result);
	}
}

MapDownloader::MapDownloader()
{
	m_downloadDir = std::filesystem::canonical(gEnv->pCryPak->GetAlias("%USER%")) / "Downloads";

	const std::filesystem::path mapDir = m_downloadDir / "Levels";

	// make sure the map directory exists
	std::filesystem::create_directories(mapDir);

	gClient->GetFileRedirector()->SetDownloadedMapsPrefix(mapDir.string());

	RescanMaps();
	RegisterDownloadedMaps();
}

MapDownloader::~MapDownloader()
{
}

void MapDownloader::Request(MapDownloaderRequest && request)
{
	bool success = false;

	if (request.map.empty())
	{
		CryLogAlways("$4[CryMP] [MapDownloader] Unknown map!");
	}
	else if (CheckMapExists(request.map))
	{
		CryLogAlways("$3[CryMP] [MapDownloader] Map $6%s$3 found, download skipped", request.map.c_str());

		// nothing to do
		success = true;
	}
	else if (request.mapURL.empty())
	{
		CryLogAlways("$4[CryMP] [MapDownloader] Missing download link for map $6%s$4!", request.map.c_str());
	}
	else
	{
		DownloadMap(std::move(request));

		return;
	}

	// no map is being downloaded, so execute the callback immediately
	CompleteRequest(request, success);
}
