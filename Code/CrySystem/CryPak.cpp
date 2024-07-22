#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include "Library/PathTools.h"

#include "CryLog.h"
#include "CryPak.h"
#include "Pak/FileOutsidePak.h"
#include "Pak/ZipPak.h"
#include "ResourceList.h"

// TODO: custom allocator for CryPak
// TODO: some dev-only command line argument to set a folder that overrides PAK in EXE
// TODO: nested paks

class CryPakWildcardMatcher
{
	enum class Wildcard
	{
		NONE, STEM, EXTENSION, FULL
	};

	Wildcard m_kind = Wildcard::NONE;
	std::string_view m_stem;
	std::string_view m_extension;

public:
	// | wildcard | kind      |
	// | :------: | :-------: |
	// | `xxx`    | NONE      |
	// | `*.xxx`  | STEM      |
	// | `xxx.*`  | EXTENSION |
	// | `*.*`    | FULL      |
	// | `*`      | FULL      |
	explicit CryPakWildcardMatcher(std::string_view wildcard)
	{
		std::tie(m_stem, m_extension) = PathTools::SplitNameIntoStemAndExtension(wildcard);

		if (!m_extension.empty())
		{
			// remove dot
			m_extension.remove_prefix(1);
		}

		if (!m_stem.empty() && m_stem.back() == '*')
		{
			m_stem.remove_suffix(1);
			m_kind = Wildcard::STEM;
		}

		if (!m_extension.empty() && m_extension.back() == '*')
		{
			m_extension.remove_suffix(1);
			m_kind = (m_kind == Wildcard::STEM) ? Wildcard::FULL : Wildcard::EXTENSION;
		}

		const auto hasWildcard = [](std::string_view x) {
			return x.find_first_of("*?") != std::string_view::npos;
		};

		if (hasWildcard(m_stem) || hasWildcard(m_extension))
		{
			CryLogErrorAlways("CryPakWildcardMatcher(\"%.*s\"): Unsupported wildcard!",
				static_cast<int>(wildcard.size()), wildcard.data());
		}
	}

	bool operator()(std::string_view name) const
	{
		auto [stem, extension] = PathTools::SplitNameIntoStemAndExtension(name);

		if (!extension.empty())
		{
			// remove dot
			extension.remove_prefix(1);
		}

		const auto check = [](std::string_view a, std::string_view b, bool prefix) -> bool
		{
			if (prefix)
			{
				return StringTools::StartsWithNoCase(a, b);
			}
			else
			{
				return StringTools::IsEqualNoCase(a, b);
			}
		};

		if (!check(stem, m_stem, m_kind == Wildcard::STEM || m_kind == Wildcard::FULL))
		{
			return false;
		}

		if (!check(extension, m_extension, m_kind == Wildcard::EXTENSION || m_kind == Wildcard::FULL))
		{
			return false;
		}

		return true;
	}
};

static std::vector<std::string> ExpandWildcardFilesystemPath(std::string_view wildcardPath)
{
	const auto [dirPath, wildcardName] = PathTools::SplitPathIntoDirAndFile(wildcardPath);
	const CryPakWildcardMatcher matcher(wildcardName);

	std::vector<std::string> foundPaths;

	std::error_code ec;
	for (const auto& entry : std::filesystem::directory_iterator(dirPath, ec))
	{
		std::string path = entry.path().generic_string();
		if (matcher(PathTools::FileName(path)))
		{
			// no need to adjust because generic_string uses forward slashes
			foundPaths.emplace_back(std::move(path));
		}
	}

	std::sort(foundPaths.begin(), foundPaths.end(), StringTools::ComparatorNoCase());

	return foundPaths;
}

CryPak::CryPak()
{
	m_resourceList_EngineStartup = ResourceList::Create();
	m_resourceList_NextLevel = ResourceList::Create();
	m_resourceList_Level = ResourceList::Create();
}

CryPak::~CryPak()
{
}

////////////////////////////////////////////////////////////////////////////////
// ICryPak
////////////////////////////////////////////////////////////////////////////////

const char* CryPak::AdjustFileName(const char* src, char* dst, unsigned int flags, bool* foundInPak)
{
	std::lock_guard lock(m_mutex);

	if (foundInPak)
	{
		CryLogWarningAlways("%s(\"%s\"): Ignoring foundInPak!", __FUNCTION__, src);
		*foundInPak = false;
	}

	const std::string adjusted = this->AdjustFileNameImpl(StringTools::SafeView(src), flags);

	// we don't know the actual size of the dst buffer
	// it's supposed to be ICryPak::g_nMaxPath, which is 2048, but it's not always the case!
	// let's hope all buffers out there are at least MAX_PATH, which is 260
	if (adjusted.length() < 260)
	{
		std::memcpy(dst, adjusted.c_str(), adjusted.length() + 1);
	}
	else
	{
		CryLogErrorAlways("%s(\"%s\"): Too long path!", __FUNCTION__, src);
		dst[0] = '\0';
	}

	return dst;
}

bool CryPak::Init(const char* basePath)
{
	return true;
}

void CryPak::Release()
{
}

bool CryPak::OpenPack(const char* name, unsigned int flags)
{
	std::lock_guard lock(m_mutex);

	const std::string adjustedName = this->AdjustFileNameImpl(StringTools::SafeView(name), flags);
	const std::string adjustedBindingRoot(PathTools::DirPath(adjustedName));

	return this->OpenPakImpl(adjustedName, adjustedBindingRoot);
}

bool CryPak::OpenPack(const char* bindingRoot, const char* name, unsigned int flags)
{
	std::lock_guard lock(m_mutex);

	const std::string adjustedName = this->AdjustFileNameImpl(StringTools::SafeView(name), flags);
	const std::string adjustedBindingRoot = this->AdjustFileNameImpl(StringTools::SafeView(bindingRoot), flags);

	PakSlot* pak = this->OpenPakImpl(adjustedName, adjustedBindingRoot);
	if (!pak)
	{
		return false;
	}

	return true;
}

bool CryPak::ClosePack(const char* name, unsigned int flags)
{
	std::lock_guard lock(m_mutex);

	const std::string adjustedName = this->AdjustFileNameImpl(StringTools::SafeView(name), flags);

	PakSlot* pak = this->FindLoadedPakByPath(adjustedName);
	if (!pak)
	{
		CryLogWarningAlways("%s(\"%s\", 0x%x): Not found!", __FUNCTION__, name, flags);
		return false;
	}

	this->ClosePakImpl(pak);

	CryLog("%s(\"%s\", 0x%x)", __FUNCTION__, name, flags);

	return true;
}

bool CryPak::OpenPacks(const char* wildcard, unsigned int flags)
{
	std::lock_guard lock(m_mutex);

	CryLog("%s(\"%s\", 0x%x): Opening paks...", __FUNCTION__, wildcard, flags);

	const std::string adjustedWildcard = this->AdjustFileNameImpl(StringTools::SafeView(wildcard), flags);
	const std::string adjustedBindingRoot(PathTools::DirPath(adjustedWildcard));

	bool success = false;

	for (const std::string& path : ExpandWildcardFilesystemPath(adjustedWildcard))
	{
		PakSlot* pak = this->OpenPakImpl(path, adjustedBindingRoot);
		if (!pak)
		{
			continue;
		}

		success = true;
	}

	return success;
}

bool CryPak::OpenPacks(const char* bindingRoot, const char* wildcard, unsigned int flags)
{
	std::lock_guard lock(m_mutex);

	CryLog("%s(\"%s\", \"%s\", 0x%x): Opening paks...", __FUNCTION__, bindingRoot, wildcard, flags);

	const std::string adjustedWildcard = this->AdjustFileNameImpl(StringTools::SafeView(wildcard), flags);
	const std::string adjustedBindingRoot = this->AdjustFileNameImpl(StringTools::SafeView(bindingRoot), flags);

	bool success = false;

	for (const std::string& path : ExpandWildcardFilesystemPath(adjustedWildcard))
	{
		PakSlot* pak = this->OpenPakImpl(path, adjustedBindingRoot);
		if (!pak)
		{
			continue;
		}

		success = true;
	}

	return success;
}

bool CryPak::ClosePacks(const char* wildcard, unsigned int flags)
{
	std::lock_guard lock(m_mutex);

	CryLog("%s(\"%s\", 0x%x): Closing paks...", __FUNCTION__, wildcard, flags);

	const std::string adjustedWildcard = this->AdjustFileNameImpl(StringTools::SafeView(wildcard), flags);

	bool success = false;

	for (const std::string& path : ExpandWildcardFilesystemPath(adjustedWildcard))
	{
		PakSlot* pak = this->FindLoadedPakByPath(path);
		if (!pak)
		{
			continue;
		}

		this->ClosePakImpl(pak);

		CryLog("%s(\"%s\", 0x%x): Closed \"%s\"", __FUNCTION__, wildcard, flags, path.c_str());

		success = true;
	}

	return success;
}

void CryPak::AddMod(const char* mod)
{
	CryLogErrorAlways("%s(\"%s\"): Not implemented!", __FUNCTION__, mod);
}

void CryPak::RemoveMod(const char* mod)
{
	CryLogErrorAlways("%s(\"%s\"): Not implemented!", __FUNCTION__, mod);
}

void CryPak::ParseAliases(const char* commandLine)
{
	CryLogErrorAlways("%s: Not implemented!", __FUNCTION__);
}

void CryPak::SetAlias(const char* name, const char* alias, bool add)
{
	std::lock_guard lock(m_mutex);

	const std::string_view aliasName = StringTools::SafeView(name);
	if (aliasName.length() < 3 || aliasName.front() != '%' || aliasName.back() != '%')
	{
		CryLogErrorAlways("%s: Invalid alias name \"%s\"", __FUNCTION__, name);
		return;
	}

	if (add)
	{
#ifdef __cpp_lib_associative_heterogeneous_insertion
		const auto [it, added] = m_aliases.try_emplace(aliasName);
#else
		const auto [it, added] = m_aliases.try_emplace(std::string(aliasName));
#endif

		if (added)
		{
			CryLog("%s: Adding \"%s\" -> \"%s\"", __FUNCTION__, name, alias);
		}
		else
		{
			CryLog("%s: Changing \"%s\" -> \"%s\" to \"%s\"", __FUNCTION__,
				name, it->second.c_str(), alias);
		}

		it->second = StringTools::SafeView(alias);
	}
	else
	{
		const auto it = m_aliases.find(aliasName);
		if (it == m_aliases.end())
		{
			CryLog("%s: \"%s\" does not exist", __FUNCTION__, name);
			return;
		}

		CryLog("%s: Removing \"%s\" -> \"%s\"", __FUNCTION__, name, it->second.c_str());

		m_aliases.erase(it);
	}
}

const char* CryPak::GetAlias(const char* name, bool returnSame)
{
	std::lock_guard lock(m_mutex);

	const auto it = m_aliases.find(StringTools::SafeView(name));
	if (it == m_aliases.end())
	{
		return returnSame ? name : nullptr;
	}

	return it->second.c_str();
}

ICryPak::PakInfo* CryPak::GetPakInfo()
{
	std::lock_guard lock(m_mutex);

	const unsigned int pakCount = m_paks.GetActiveCount();

	PakInfo* info = static_cast<PakInfo*>(std::calloc(1, sizeof(PakInfo) + (sizeof(PakInfo::Pak) * pakCount)));
	info->numOpenPaks = pakCount;

	const auto my_strdup = [](std::string_view str) -> char*
	{
		char* res = static_cast<char*>(std::malloc(str.length() + 1));
		std::memcpy(res, str.data(), str.length());
		res[str.length()] = '\0';
		return res;
	};

	PakSlot* pak = m_paks.GetFirstActive();

	for (unsigned int i = 0; i < pakCount; ++i)
	{
		info->arrPaks[i].szFilePath = my_strdup(pak->path);
		info->arrPaks[i].szBindRoot = my_strdup(pak->bindingRoot);
		info->arrPaks[i].nUsedMem = 0;

		pak = m_paks.GetNextActive(pak);
	}

	return info;
}

void CryPak::FreePakInfo(PakInfo* pakInfo)
{
	// no need to lock m_mutex

	const unsigned int pakCount = pakInfo->numOpenPaks;

	for (unsigned int i = 0; i < pakCount; ++i)
	{
		std::free(const_cast<char*>(pakInfo->arrPaks[i].szFilePath));
		std::free(const_cast<char*>(pakInfo->arrPaks[i].szBindRoot));
	}

	std::free(pakInfo);
}

FILE* CryPak::FOpen(const char* name, const char* mode, unsigned int flags)
{
	std::lock_guard lock(m_mutex);

	std::string adjustedName = this->AdjustFileNameImpl(StringTools::SafeView(name), flags);
	const FileModeFlags modeFlags = this->FileModeFromString(StringTools::SafeView(mode));
	bool foundInPak = true;

	OpenFileSlot* file = this->OpenFileInPakImpl(std::move(adjustedName), modeFlags);
	if (!file)
	{
		foundInPak = false;
		file = this->OpenFileOutsideImpl(std::move(adjustedName), modeFlags);
		if (!file)
		{
			CryLogComment("%s(\"%s\", \"%s\", 0x%x): Not found!", __FUNCTION__, name, mode, flags);
			return nullptr;
		}
	}

	FILE* handle = reinterpret_cast<FILE*>(m_files.SlotToHandle(file));

	if (foundInPak)
	{
		PakSlot* pak = m_paks.HandleToSlot(file->pakHandle);
		CryLogComment("%s(\"%s\", \"%s\", 0x%x): 0x%p Found in pak \"%s\"", __FUNCTION__, name, mode, flags,
			handle, pak ? pak->path.c_str() : "");
	}
	else
	{
		CryLogComment("%s(\"%s\", \"%s\", 0x%x): 0x%p Found outside \"%s\"", __FUNCTION__, name, mode, flags,
			handle, file->path.c_str());
	}

	return handle;
}

FILE* CryPak::FOpen(const char* name, const char* mode, char* fileGamePath, int length)
{
	CryLogErrorAlways("%s(\"%s\", \"%s\", 0x%p, %d): Not implemented!", __FUNCTION__, name, mode,
		fileGamePath, length);
	return nullptr;
}

size_t CryPak::FReadRaw(void* data, size_t elementSize, size_t elementCount, FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return 0;
	}

	return file->impl->FRead(data, elementSize, elementCount);
}

size_t CryPak::FReadRawAll(void* data, size_t fileSize, FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return 0;
	}

	if (file->impl->GetSize() != fileSize)
	{
		return 0;
	}

	if (file->impl->FSeek(0, SEEK_SET) < 0)
	{
		return 0;
	}

	return file->impl->FRead(data, 1, fileSize);
}

void* CryPak::FGetCachedFileData(FILE* handle, size_t& fileSize)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return nullptr;
	}

	return file->impl->GetCachedFileData(fileSize);
}

size_t CryPak::FWrite(const void* data, size_t elementSize, size_t elementCount, FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return 0;
	}

	return file->impl->FWrite(data, elementSize, elementCount);
}

int CryPak::FPrintf(FILE* handle, const char* format, ...)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return -1;
	}

	va_list args;
	va_start(args, format);
	const int length = file->impl->VFPrintF(format, args);
	va_end(args);

	return length;
}

char* CryPak::FGets(char* buffer, int bufferSize, FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return nullptr;
	}

	return file->impl->FGetS(buffer, bufferSize);
}

int CryPak::Getc(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return EOF;
	}

	return file->impl->FGetC();
}

size_t CryPak::FGetSize(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return 0;
	}

	return static_cast<size_t>(file->impl->GetSize());
}

int CryPak::Ungetc(int ch, FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return EOF;
	}

	return file->impl->FUnGetC(ch);
}

bool CryPak::IsInPak(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return false;
	}

	return file->pakHandle != 0;
}

bool CryPak::RemoveFile(const char* name)
{
	std::lock_guard lock(m_mutex);

	const std::string adjustedName = this->AdjustFileNameImpl(StringTools::SafeView(name), 0);

	CryLog("%s(\"%s\"): Removing \"%s\"", __FUNCTION__, name, adjustedName.c_str());

	std::error_code ec;
	return std::filesystem::remove(adjustedName, ec);
}

bool CryPak::RemoveDir(const char* name, bool recurse)
{
	std::lock_guard lock(m_mutex);

	const std::string adjustedName = this->AdjustFileNameImpl(StringTools::SafeView(name), 0);

	CryLog("%s(\"%s\", recurse=%d): Removing \"%s\"", __FUNCTION__, name, static_cast<int>(recurse),
		adjustedName.c_str());

	if (recurse)
	{
		std::error_code ec;
		return std::filesystem::remove_all(adjustedName, ec) > 0 && !ec;
	}
	else
	{
		std::error_code ec;
		return std::filesystem::remove(adjustedName, ec);
	}
}

bool CryPak::IsAbsPath(const char* path)
{
	// no need to lock m_mutex

	return PathTools::IsAbsolutePath(StringTools::SafeView(path));
}

size_t CryPak::FSeek(FILE* handle, long seek, int mode)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return static_cast<size_t>(-1);
	}

	return static_cast<size_t>(file->impl->FSeek(seek, mode));
}

long CryPak::FTell(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return -1;
	}

	return static_cast<long>(file->impl->FTell());
}

int CryPak::FClose(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return EOF;
	}

	this->CloseFileImpl(file);

	CryLogComment("%s(0x%p)", __FUNCTION__, handle);

	return 0;
}

int CryPak::FEof(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return 0;
	}

	return file->impl->FEof();
}

int CryPak::FError(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return 0;
	}

	return file->impl->FError();
}

int CryPak::FGetErrno()
{
	return -1;
}

int CryPak::FFlush(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return 0;
	}

	return file->impl->FFlush();
}

void* CryPak::PoolMalloc(size_t size)
{
	return std::malloc(size);
}

void CryPak::PoolFree(void* pool)
{
	std::free(pool);
}

intptr_t CryPak::FindFirst(const char* wildcard, struct _finddata_t* fd, unsigned int flags)
{
	std::lock_guard lock(m_mutex);

	std::string adjustedWildcard = this->AdjustFileNameImpl(StringTools::SafeView(wildcard), flags);

	FindSlot* find = this->OpenFindImpl(std::move(adjustedWildcard));
	if (!find)
	{
		return -1;
	}

	this->FillFindData(fd, find->entries[0]);
	find->pos++;

	return static_cast<intptr_t>(m_finds.SlotToHandle(find));
}

int CryPak::FindNext(intptr_t handle, struct _finddata_t* fd)
{
	std::lock_guard lock(m_mutex);

	FindSlot* find = m_finds.HandleToSlot(static_cast<SlotVectorHandle>(handle));
	if (!find)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, reinterpret_cast<void*>(handle));
		return -1;
	}

	if (find->pos >= find->entries.size())
	{
		return -1;
	}

	this->FillFindData(fd, find->entries[find->pos]);
	find->pos++;

	return 0;
}

int CryPak::FindClose(intptr_t handle)
{
	std::lock_guard lock(m_mutex);

	FindSlot* find = m_finds.HandleToSlot(static_cast<SlotVectorHandle>(handle));
	if (!find)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, reinterpret_cast<void*>(handle));
		return -1;
	}

	this->CloseFindImpl(find);

	return 0;
}

ICryPak::FileTime CryPak::GetModificationTime(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return 0;
	}

	return file->impl->GetModificationTime();
}

bool CryPak::IsFileExist(const char* name)
{
	std::lock_guard lock(m_mutex);

	const std::string adjustedName = this->AdjustFileNameImpl(StringTools::SafeView(name), 0);

	FileTreeNode* fileNode = m_tree.FindNode(adjustedName);
	if (fileNode)
	{
		CryLogComment("%s(\"%s\"): Found in a pak", __FUNCTION__, name);
		return true;
	}

	std::error_code ec;
	if (std::filesystem::exists(adjustedName, ec))
	{
		CryLogComment("%s(\"%s\"): Found outside \"%s\"", __FUNCTION__, name, adjustedName.c_str());
		return true;
	}

	CryLogComment("%s(\"%s\"): Not found!", __FUNCTION__, name);

	return false;
}

bool CryPak::MakeDir(const char* path)
{
	std::lock_guard lock(m_mutex);

	const std::string adjustedPath = this->AdjustFileNameImpl(StringTools::SafeView(path), 0);

	std::error_code ec;
	const bool created = std::filesystem::create_directories(adjustedPath, ec);

	if (ec)
	{
		CryLogErrorAlways("%s(\"%s\"): Error %d (%s)", __FUNCTION__, path, ec.value(), ec.message().c_str());
		return false;
	}
	else if (!created)
	{
		CryLog("%s(\"%s\"): Already exists", __FUNCTION__, path);
		return true;
	}
	else
	{
		CryLog("%s(\"%s\"): Success", __FUNCTION__, path);
		return true;
	}
}

ICryArchive* CryPak::OpenArchive(const char* path, unsigned int flags)
{
	CryLogErrorAlways("%s(\"%s\", 0x%x): Not implemented!", __FUNCTION__, path, flags);
	return nullptr;
}

const char* CryPak::GetFileArchivePath(FILE* handle)
{
	std::lock_guard lock(m_mutex);

	OpenFileSlot* file = m_files.HandleToSlot(reinterpret_cast<SlotVectorHandle>(handle));
	if (!file)
	{
		CryLogErrorAlways("%s(0x%p): Invalid handle!", __FUNCTION__, handle);
		return nullptr;
	}

	PakSlot* pak = m_paks.HandleToSlot(file->pakHandle);
	if (!pak)
	{
		return nullptr;
	}

	return pak->path.c_str();
}

int CryPak::RawCompress(const void* uncompressed, unsigned long* dstSize, void* compressed, unsigned long srcSize, int level)
{
	CryLogErrorAlways("%s: Not implemented!", __FUNCTION__);
	return -1;
}

int CryPak::RawUncompress(void* uncompressed, unsigned long* dstSize, const void* compressed, unsigned long srcSize)
{
	CryLogErrorAlways("%s: Not implemented!", __FUNCTION__);
	return -1;
}

void CryPak::RecordFileOpen(const ERecordFileOpenList list)
{
	CryLogErrorAlways("%s(%d): Not implemented!", __FUNCTION__, static_cast<int>(list));
}

void CryPak::RecordFile(FILE* handle, const char* name)
{
	CryLogErrorAlways("%s(0x%p, \"%s\"): Not implemented!", __FUNCTION__, handle, name);
}

IResourceList* CryPak::GetRecorderdResourceList(const ERecordFileOpenList list)
{
	// no need to lock m_mutex

	switch (list)
	{
		case RFOM_Disabled:      break;
		case RFOM_EngineStartup: return m_resourceList_EngineStartup.get();
		case RFOM_NextLevel:     return m_resourceList_NextLevel.get();
		case RFOM_Level:         return m_resourceList_Level.get();
	}

	return nullptr;
}

ICryPak::ERecordFileOpenList CryPak::GetRecordFileOpenList()
{
	return RFOM_Disabled;
}

void CryPak::Notify(ENotifyEvent event)
{
}

std::uint32_t CryPak::ComputeCRC(const char* path)
{
	CryLogErrorAlways("%s(\"%s\"): Not implemented!", __FUNCTION__, path);
	return 0;
}

bool CryPak::ComputeMD5(const char* path, unsigned char* md5)
{
	CryLogErrorAlways("%s(\"%s\"): Not implemented!", __FUNCTION__, path);
	return false;
}

void CryPak::RegisterFileAccessSink(ICryPakFileAcesssSink* pSink)
{
	CryLogErrorAlways("%s(0x%p): Not implemented!", __FUNCTION__, pSink);
}

void CryPak::UnregisterFileAccessSink(ICryPakFileAcesssSink* pSink)
{
	CryLogErrorAlways("%s(0x%p): Not implemented!", __FUNCTION__, pSink);
}

bool CryPak::GetLvlResStatus() const
{
	return false;
}

const char* CryPak::GetModDir() const
{
	return "";
}

////////////////////////////////////////////////////////////////////////////////

void CryPak::LoadInternalPak(const void* data, std::size_t size)
{
	std::lock_guard lock(m_mutex);

	std::unique_ptr<ZipPak> pakImpl = ZipPak::OpenMemory(data, size);
	if (!pakImpl)
	{
		CryLogErrorAlways("%s: Failed!", __FUNCTION__);
		return;
	}

	PakSlot* pak = m_paks.GetFreeSlot();
	if (!pak)
	{
		CryLogErrorAlways("%s: Max number of slots reached!", __FUNCTION__);
		return;
	}

	pak->path.clear();
	pak->bindingRoot = "Game";
	pak->impl = std::move(pakImpl);
	pak->priority = Priority::HIGH;
	pak->refCount = 1;

	this->AddPakToTree(pak);

	CryLog("%s: Success", __FUNCTION__);
}

void CryPak::AddRedirect(std::string_view path, std::string_view newPath)
{
	std::lock_guard lock(m_mutex);

	auto [node, added] = m_redirects.AddNode(this->AdjustFileNameImplWithoutRedirect(path, 0));
	if (!node)
	{
		CryLogErrorAlways("%s(\"%.*s\", \"%.*s\"): Cannot replace more specific redirect!", __FUNCTION__,
			static_cast<int>(path.size()), path.data(), static_cast<int>(newPath.size()), newPath.data());
		return;
	}

	node->assign(this->AdjustFileNameImplWithoutRedirect(newPath, 0));

	CryLog("%s(\"%.*s\", \"%.*s\"): %s", __FUNCTION__, static_cast<int>(path.size()), path.data(),
		static_cast<int>(newPath.size()), newPath.data(), added ? "Added" : "Changed existing");
}

void CryPak::RemoveRedirect(std::string_view path)
{
	std::lock_guard lock(m_mutex);

	bool removed = m_redirects.Erase(this->AdjustFileNameImplWithoutRedirect(path, 0));

	CryLog("%s(\"%.*s\"): %s", __FUNCTION__, static_cast<int>(path.size()), path.data(),
		removed ? "Removed" : "Not found");
}

void CryPak::LogInfo()
{
	std::lock_guard lock(m_mutex);

	CryLogAlways("------------------------------------ CryPak ------------------------------------");

	{
		std::size_t totalCount = 0;
		std::size_t totalCachedBytes = 0;
		CryLogAlways("Open files:");
		for (OpenFileSlot* file = m_files.GetFirstActive(); file; file = m_files.GetNextActive(file))
		{
			std::size_t cachedBytes = file->impl->GetCachedDataSize();
			CryLogAlways("- $8\"%s\"$o in_pak=$5%d$o cached_bytes=$5%zu$o", file->path.c_str(),
				(file->pakHandle) ? 1 : 0, cachedBytes);
			totalCount++;
			totalCachedBytes += cachedBytes;
		}
		CryLogAlways("Total open file count: $5%zu$o", totalCount);
		CryLogAlways("Total cached bytes: $5%zu$o", totalCachedBytes);
	}

	{
		std::size_t totalCount = 0;
		CryLogAlways("Open finds:");
		for (FindSlot* find = m_finds.GetFirstActive(); find; find = m_finds.GetNextActive(find))
		{
			CryLogAlways("- $8\"%s\"$o entry_count=$5%zu$o", find->path.c_str(), find->entries.size());
			totalCount++;
		}
		CryLogAlways("Total open find count: $5%zu$o", totalCount);
	}

	{
		std::size_t totalCount = 0;
		std::size_t totalEntryCount = 0;
		CryLogAlways("Loaded paks:");
		for (PakSlot* pak = m_paks.GetFirstActive(); pak; pak = m_paks.GetNextActive(pak))
		{
			const std::size_t entryCount = pak->impl->GetEntryCount();
			CryLogAlways("- $8\"%s\"$o binding_root=$8\"%s\"$o entry_count=$5%u$o", pak->path.c_str(),
				pak->bindingRoot.c_str(), entryCount);
			totalCount++;
			totalEntryCount += entryCount;
		}
		CryLogAlways("Total pak count: $5%zu$o", totalCount);
		CryLogAlways("Total pak entry count: $5%zu$o", totalEntryCount);
	}

	CryLogAlways("--------------------------------------------------------------------------------");
}

std::string CryPak::AdjustFileNameImplWithoutRedirect(std::string_view path, unsigned int flags)
{
	std::string adjusted;
	adjusted.reserve(260);
	adjusted = path;

	// TODO: prevent access outside Crysis main directory and user directory

	if ((flags & FLAGS_FOR_WRITING) && !m_gameFolderWritable)
	{
		if (adjusted.empty() || (adjusted[0] != '%' && !PathTools::IsAbsolutePath(adjusted)))
		{
			adjusted.insert(0, "%USER%/");
		}
	}

	this->ExpandAliases(adjusted);

	std::replace(adjusted.begin(), adjusted.end(), '\\', '/');
	// TODO: remove duplicate slashes
	// TODO: normalize the path

	if (!PathTools::IsAbsolutePath(adjusted) && !(flags & FLAGS_NO_FULL_PATH))
	{
		const bool hasDotSlash = adjusted.starts_with("./");

		if (!hasDotSlash
		 && !StringTools::StartsWithNoCase(adjusted, "Game/")
		 && !StringTools::StartsWithNoCase(adjusted, "Editor/")
		 && !StringTools::StartsWithNoCase(adjusted, "Bin32/")
		 && !StringTools::StartsWithNoCase(adjusted, "Bin64/")
		 && !StringTools::StartsWithNoCase(adjusted, "Mods/"))
		{
			adjusted.insert(0, "Game/");
		}
		else if (hasDotSlash)
		{
			adjusted.erase(0, 2);
		}
	}

	if (flags & FLAGS_ADD_TRAILING_SLASH)
	{
		PathTools::AddTrailingSlash(adjusted);
	}

	return adjusted;
}

std::string CryPak::AdjustFileNameImpl(std::string_view path, unsigned int flags)
{
	std::string adjusted = this->AdjustFileNameImplWithoutRedirect(path, flags);

	auto [redirectNode, remainingPath] = m_redirects.FindNodeInPath(adjusted);
	if (redirectNode)
	{
		// remaining path is also a string view into adjusted
		std::string_view prefix(adjusted.data(), adjusted.length() - remainingPath.length());

		if (!prefix.empty() && prefix.back() == '/')
		{
			// keep the slash
			prefix.remove_suffix(1);
		}

		adjusted.replace(0, prefix.length(), *redirectNode);
	}

	return adjusted;
}

CryPak::OpenFileSlot* CryPak::OpenFileInPakImpl(std::string&& filePath, FileModeFlags mode)
{
	FileTreeNode* fileNode = m_tree.FindNode(filePath);
	if (!fileNode)
	{
		return nullptr;
	}

	if (this->IsFileModeWriting(mode))
	{
		CryLogErrorAlways("%s(\"%s\"): Writing to paks is not supported!", __FUNCTION__, filePath.c_str());
		return nullptr;
	}

	PakSlot* pak = m_paks.HandleToSlot(fileNode->current.pakHandle);
	if (!pak)
	{
		CryLogErrorAlways("%s(\"%s\"): Invalid pak handle!", __FUNCTION__, filePath.c_str());
		return nullptr;
	}

	const bool isBinary = (mode & FILE_MODE_BINARY);

	std::unique_ptr<IFileInPak> fileImpl = pak->impl->OpenFile(fileNode->current.fileIndex, isBinary);
	if (!fileImpl)
	{
		CryLogErrorAlways("%s(\"%s\"): Error in pak!", __FUNCTION__, filePath.c_str());
		return nullptr;
	}

	OpenFileSlot* file = m_files.GetFreeSlot();
	if (!file)
	{
		CryLogErrorAlways("%s(\"%s\"): Max number of slots reached!", __FUNCTION__, filePath.c_str());
		return nullptr;
	}

	file->path = std::move(filePath);
	file->impl = std::move(fileImpl);
	file->pakHandle = m_paks.SlotToHandle(pak);
	this->IncrementPakRefCount(pak);
	return file;
}

CryPak::OpenFileSlot* CryPak::OpenFileOutsideImpl(std::string&& filePath, FileModeFlags mode)
{
	std::filesystem::path fsPath(filePath);

	if (this->IsFileModeWriting(mode))
	{
		std::error_code ec;
		std::filesystem::create_directories(fsPath.parent_path(), ec);

		if (ec)
		{
			CryLogErrorAlways("%s(\"%s\"): Creating directories failed: Error %d (%s)", __FUNCTION__,
				filePath.c_str(), ec.value(), ec.message().c_str());
			return nullptr;
		}
	}

	FileOutsidePak::Handle handle(std::fopen(filePath.c_str(), this->FileModeToString(mode).c_str()));
	if (!handle)
	{
		const int errnum = errno;
		if (errnum != ENOENT)
		{
			CryLogErrorAlways("%s(\"%s\"): File open failed: Error %d (%s)", __FUNCTION__,
				filePath.c_str(), errnum, std::strerror(errnum));
		}
		return nullptr;
	}

	OpenFileSlot* file = m_files.GetFreeSlot();
	if (!file)
	{
		CryLogErrorAlways("%s(\"%s\"): Max number of slots reached!", __FUNCTION__, filePath.c_str());
		return nullptr;
	}

	file->path = std::move(filePath);
	file->impl = std::make_unique<FileOutsidePak>(std::move(handle), std::move(fsPath));
	file->pakHandle = 0;
	return file;
}

void CryPak::CloseFileImpl(OpenFileSlot* file)
{
	PakSlot* pak = m_paks.HandleToSlot(file->pakHandle);

	// close the file
	file->clear();

	if (pak)
	{
		this->DecrementPakRefCount(pak);
	}
}

CryPak::FindSlot* CryPak::OpenFindImpl(std::string&& wildcardPath)
{
	const auto [dirPath, wildcardName] = PathTools::SplitPathIntoDirAndFile(wildcardPath);
	const CryPakWildcardMatcher matcher(wildcardName);

	std::vector<FindSlot::Entry> entries;

	Tree::DirectoryNode* dirNode = m_tree.FindDirectoryNode(dirPath);
	if (dirNode)
	{
		for (auto& [name, node] : *dirNode)
		{
			if (!matcher(name))
			{
				continue;
			}

			std::uint64_t size = 0;
			bool isDirectory = true;

			if (FileTreeNode* fileNode = std::get_if<FileTreeNode>(&node); fileNode)
			{
				isDirectory = false;
				size = this->GetFileSize(*fileNode);
			}

			entries.emplace_back(name, size, isDirectory, true);
		}
	}

	const auto isDuplicate = [&entries](std::string_view name) -> bool
	{
		return std::any_of(entries.begin(), entries.end(),
			[name](const auto& entry) { return StringTools::IsEqualNoCase(entry.name, name); });
	};

	std::error_code ec;
	for (const auto& fsEntry : std::filesystem::directory_iterator(dirPath, ec))
	{
		std::string name = fsEntry.path().filename().generic_string();

		if (!matcher(name) || isDuplicate(name))
		{
			continue;
		}

		std::uint64_t size = 0;
		bool isDirectory = true;

		if (!fsEntry.is_directory(ec))
		{
			isDirectory = false;
			size = fsEntry.file_size(ec);
		}

		entries.emplace_back(std::move(name), size, isDirectory, false);
	}

	CryLogComment("%s(\"%s\"): Found %zu entries", __FUNCTION__, wildcardPath.c_str(), entries.size());

	if (entries.empty())
	{
		return nullptr;
	}

	FindSlot* find = m_finds.GetFreeSlot();
	if (!find)
	{
		CryLogErrorAlways("%s(\"%s\"): Max number of slots reached!", __FUNCTION__, wildcardPath.c_str());
		return nullptr;
	}

	find->path = std::move(wildcardPath);
	find->entries = std::move(entries);
	find->pos = 0;

	return find;
}

void CryPak::CloseFindImpl(FindSlot* find)
{
	find->clear();
}

CryPak::PakSlot* CryPak::OpenPakImpl(const std::string& pakPath, const std::string& bindingRoot)
{
	PakSlot* pak = this->FindLoadedPakByPath(pakPath);
	if (pak)
	{
		CryLog("%s(\"%s\", \"%s\"): Already loaded", __FUNCTION__, pakPath.c_str(), bindingRoot.c_str());
		return pak;
	}

	std::unique_ptr<ZipPak> pakImpl = ZipPak::OpenFileName(pakPath);
	if (!pakImpl)
	{
		CryLogErrorAlways("%s(\"%s\", \"%s\"): Failed!", __FUNCTION__, pakPath.c_str(), bindingRoot.c_str());
		return nullptr;
	}

	pak = m_paks.GetFreeSlot();
	if (!pak)
	{
		CryLogErrorAlways("%s(\"%s\", \"%s\"): Max number of slots reached!", __FUNCTION__,
			pakPath.c_str(), bindingRoot.c_str());
		return nullptr;
	}

	pak->path = pakPath;
	pak->bindingRoot = bindingRoot;
	pak->impl = std::move(pakImpl);
	pak->priority = Priority::NORMAL;
	pak->refCount = 1;

	this->AddPakToTree(pak);

	CryLog("%s(\"%s\", \"%s\"): Success", __FUNCTION__, pakPath.c_str(), bindingRoot.c_str());

	return pak;
}

CryPak::PakSlot* CryPak::FindLoadedPakByPath(const std::string& pakPath)
{
	if (pakPath.empty())
	{
		return nullptr;
	}

	return m_paks.Find([pakPath](const PakSlot& pak) { return StringTools::IsEqualNoCase(pak.path, pakPath); });
}

void CryPak::ClosePakImpl(PakSlot* pak)
{
	this->RemovePakFromTree(pak);
	this->DecrementPakRefCount(pak);
}

void CryPak::IncrementPakRefCount(PakSlot* pak)
{
	pak->refCount++;
}

void CryPak::DecrementPakRefCount(PakSlot* pak)
{
	pak->refCount--;

	if (pak->refCount <= 0)
	{
		pak->clear();
	}
}

void CryPak::ExpandAliases(std::string& path)
{
	std::string::size_type pos = 0;
	while ((pos = path.find('%', pos)) != std::string::npos)
	{
		auto endPos = path.find('%', pos + 1);
		if (endPos == std::string::npos)
		{
			break;
		}

		++endPos;

		const std::string_view aliasName(path.c_str() + pos, endPos - pos);
		const auto aliasIt = m_aliases.find(aliasName);
		if (aliasIt == m_aliases.end())
		{
			// ignore unknown aliases
			pos = endPos;
		}
		else
		{
			path.replace(path.begin() + pos, path.begin() + endPos, aliasIt->second);

			pos += aliasIt->second.length();
		}
	}
}

void CryPak::FillFindData(struct _finddata_t* fd, const FindSlot::Entry& entry)
{
	CryLogComment("%s: \"%s\" directory=%d in_pak=%d", __FUNCTION__, entry.name.c_str(),
		static_cast<int>(entry.isDirectory), static_cast<int>(entry.isInPak));

	if (!fd)
	{
		return;
	}

	fd->attrib = _A_NORMAL;
	fd->size = entry.size;
	// nothing uses these anyway
	fd->time_create = 0;
	fd->time_access = 0;
	fd->time_write = 0;

	if (entry.isDirectory)
	{
		fd->attrib |= _A_SUBDIR;
	}

	if (entry.isInPak)
	{
		fd->attrib |= _A_IN_CRYPAK | _A_RDONLY;
	}

	const auto length = std::min(entry.name.length(), std::size(fd->name) - 1);

	std::memcpy(fd->name, entry.name.c_str(), length);
	fd->name[length] = '\0';
}

CryPak::FileModeFlags CryPak::FileModeFromString(std::string_view mode)
{
	FileModeFlags flags = 0;

	for (char ch : mode)
	{
		switch (ch)
		{
			case 'r': flags |= FILE_MODE_READ; break;
			case 'w': flags |= FILE_MODE_WRITE; break;
			case 'a': flags |= FILE_MODE_APPEND; break;
			case '+': flags |= FILE_MODE_EXTENDED; break;
			case 'b': flags |= FILE_MODE_BINARY; break;
			case 't': flags &= ~FILE_MODE_BINARY; break;
			case 'x': break;
		}
	}

	return flags;
}

std::string CryPak::FileModeToString(FileModeFlags flags)
{
	std::string mode;

	if (flags & FILE_MODE_READ)
	{
		mode += 'r';
	}

	if (flags & FILE_MODE_WRITE)
	{
		mode += 'w';
	}

	if (flags & FILE_MODE_APPEND)
	{
		mode += 'a';
	}

	if (flags & FILE_MODE_EXTENDED)
	{
		mode += '+';
	}

	if (flags & FILE_MODE_BINARY)
	{
		mode += 'b';
	}

	return mode;
}

bool CryPak::IsFileModeWriting(FileModeFlags flags)
{
	return (flags & (FILE_MODE_WRITE | FILE_MODE_APPEND | FILE_MODE_EXTENDED));
}

std::uint64_t CryPak::GetFileSize(FileTreeNode& fileNode)
{
	PakSlot* pak = m_paks.HandleToSlot(fileNode.current.pakHandle);
	if (!pak)
	{
		return 0;
	}

	std::uint64_t size = 0;
	if (!pak->impl->GetEntrySize(fileNode.current.fileIndex, size))
	{
		return 0;
	}

	return size;
}

void CryPak::AddPakToTree(PakSlot* pak)
{
	const SlotVectorHandle pakHandle = m_paks.SlotToHandle(pak);

	auto [baseNode, baseNodeAdded] = m_tree.AddDirectory(pak->bindingRoot);
	if (!baseNode)
	{
		// binding root path points to a file
		return;
	}

	const std::uint32_t entryCount = pak->impl->GetEntryCount();

	std::string filePath;
	for (std::uint32_t i = 0; i < entryCount; i++)
	{
		if (pak->impl->IsDirectoryEntry(i))
		{
			continue;
		}

		if (!pak->impl->GetEntryPath(i, filePath))
		{
			continue;
		}

		// TODO: normalize the path

		auto [fileNode, fileNodeAdded] = m_tree.AddNode(filePath, baseNode);
		if (!fileNode)
		{
			continue;
		}

		const FileTreeNode::FileInPak file = {
			.fileIndex = i,
			.pakHandle = pakHandle,
			.priority = pak->priority,
		};

		// we don't need to check if the node was added because newly added nodes have invalid pak handle
		this->AddFileAlternative(*fileNode, file);
	}
}

void CryPak::RemovePakFromTree(PakSlot* pak)
{
	const SlotVectorHandle pakHandle = m_paks.SlotToHandle(pak);

	m_tree.EraseIf([pakHandle](FileTreeNode& fileNode) -> bool {
		fileNode.alternatives.remove_if([pakHandle](const auto& x) { return x.pakHandle == pakHandle; });

		if (fileNode.current.pakHandle != pakHandle)
		{
			// keep the node
			return false;
		}

		if (!fileNode.alternatives.empty())
		{
			// use the previous alternative of the file in a different pak with the same or lower priority
			fileNode.current = fileNode.alternatives.front();
			fileNode.alternatives.pop_front();

			// keep the node
			return false;
		}

		// the node is now empty, so erase it
		return true;
	});
}

void CryPak::AddFileAlternative(FileTreeNode& fileNode, const FileTreeNode::FileInPak& newFile)
{
	if (!fileNode.current.pakHandle)
	{
		// newly added empty file node
		fileNode.current = newFile;
		return;
	}

	if (fileNode.current.priority <= newFile.priority)
	{
		fileNode.alternatives.push_front(fileNode.current);
		fileNode.current = newFile;
		return;
	}

	fileNode.alternatives.push_front(newFile);

	// sort from highest priority to lowest
	fileNode.alternatives.sort([](const auto& a, const auto& b) { return a.priority > b.priority; });
}
