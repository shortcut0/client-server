/*
#define NOMINMAX // Prevent Windows from defining min and max macros
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <algorithm> // For std::min and std::max
*/

#include <string>
#include <vector>
#include <cstring>

/*
struct CPUUsageTimes {
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
};*/

class ServerStats
{

public:

    float m_updateTime = 0.0f;
    float m_cpuUsage = 0.0f;

    // --------------
    /*
    CPUUsageTimes m_preTimes;
    CPUUsageTimes m_postTimes;
    bool m_timesUpdated = false;
    */
    void Update(float frameTime) {

        /*
        if (frameTime >= m_updateTime) {
            m_updateTime += 2.0f;

            GetCPUUsageTimes(m_preTimes);
            m_cpuUsage = float(UpdateCPU());
            GetCPUUsageTimes(m_postTimes);
        }*/
    }

    // ------------------------------------------------
    // CPU

    std::string GetCPUName() {
        /*
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            return "Unable to open registry key";
        }

        char cpuName[256];
        DWORD size = sizeof(cpuName);
        if (RegQueryValueEx(hKey, "ProcessorNameString", nullptr, nullptr,
            (LPBYTE)cpuName, &size) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return "Unable to read CPU name";
        }

        RegCloseKey(hKey);
        return std::string(cpuName);
        */

        return "";
    }

    /*
    void GetCPUUsageTimes(CPUUsageTimes& times) {
        GetSystemTimes(&times.idleTime, &times.kernelTime, &times.userTime);
    }
    
    double UpdateCPU() {
        ULONGLONG idleDiff = (reinterpret_cast<ULONGLONG&>(m_postTimes.idleTime) - reinterpret_cast<ULONGLONG&>(m_preTimes.idleTime));
        ULONGLONG kernelDiff = (reinterpret_cast<ULONGLONG&>(m_postTimes.kernelTime) - reinterpret_cast<ULONGLONG&>(m_preTimes.kernelTime));
        ULONGLONG userDiff = (reinterpret_cast<ULONGLONG&>(m_postTimes.userTime) - reinterpret_cast<ULONGLONG&>(m_preTimes.userTime));

        ULONGLONG totalSystem = kernelDiff + userDiff;
        ULONGLONG totalTime = totalSystem - idleDiff;

        return (static_cast<double>(totalTime) / totalSystem) * 100.0;
    }
    */
    float GetCPUUsage() {
        return m_cpuUsage;
    }

    // ------------------------------------------------
    // MEMORY

	float GetMemUsage() {
        /*
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return float(pmc.WorkingSetSize);
        }
        else {
        }*/

        return 0.f;
	}

    // ------------------------------------------------

	float GetMemPeak() {
        /*
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return float(pmc.PeakWorkingSetSize);
        }
        else {
        }*/

        return 0.f;
	}

    // ------------------------------------------------

};