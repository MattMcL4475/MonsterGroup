#include <fstream>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <psapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif

double getSystemMemory()
{
#if defined(_WIN32) || defined(_WIN64)
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo))
    {
        return static_cast<double>(memInfo.ullTotalPhys) / (1024.0 * 1024.0 * 1024.0);
    }
    return 0.0;
#else
    std::ifstream file("/proc/meminfo");
    std::string line;

    while (getline(file, line))
    {
        if (line.find("MemTotal") != std::string::npos)
        {
            size_t pos = line.find(":");
            std::string memStr = line.substr(pos + 1);
            return std::stod(memStr) / (1024 * 1024);
        }
    }
    return 0.0;
#endif
}

double getProcessMemory()
{
#if defined(_WIN32) || defined(_WIN64)
    HANDLE process = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
    {
        // Dividing by 1024 twice for KB, then MB, and then again for GB
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0 * 1024.0);
    }
    return 0.0;
#else
    std::ifstream file("/proc/self/status");
    std::string line;

    while (getline(file, line))
    {
        if (line.find("VmRSS") != std::string::npos)
        {
            size_t pos = line.find(":");
            std::string memStr = line.substr(pos + 1);
            // Assuming the value in memStr is in KB, divide by 1024 for MB, and then again for GB
            return std::stod(memStr) / (1024.0 * 1024.0);
        }
}
    return 0.0;
#endif
}

double getCPUClockSpeed() 
{
#if defined(_WIN32) || defined(_WIN64)
    double GHz = 0.0;
    HRESULT hres;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (SUCCEEDED(hres)) {
        hres = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    }

    IWbemLocator* pLoc = nullptr;
    if (SUCCEEDED(hres)) {
        hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>(&pLoc));
    }

    IWbemServices* pSvc = nullptr;
    if (SUCCEEDED(hres)) {
        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pSvc);
    }

    if (SUCCEEDED(hres)) {
        hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    }

    IEnumWbemClassObject* pEnumerator = nullptr;
    if (SUCCEEDED(hres)) {
        hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_Processor"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;
    while (pEnumerator && pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK && uReturn != 0) {
        VARIANT vtProp;
        pclsObj->Get(L"MaxClockSpeed", 0, &vtProp, nullptr, nullptr);
        GHz = vtProp.uintVal / 1000.0;
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    if (pEnumerator) pEnumerator->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();

    return GHz;
#else
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            std::size_t pos = line.find(':');
            double MHz = std::stod(line.substr(pos + 1));
            return MHz / 1000.0; // Convert from MHz to GHz
        }
    }
    return 0.0; // Return 0.0 if not found
#endif
}



int getCacheLineSize() {
#ifdef _WIN32
    DWORD bufferSize = 0;
    DWORD buffer[4];
    GetLogicalProcessorInformation(0, &bufferSize);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* bufferInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(bufferSize);
    GetLogicalProcessorInformation(&bufferInfo[0], &bufferSize);

    for (DWORD i = 0; i != bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (bufferInfo[i].Relationship == RelationCache && bufferInfo[i].Cache.Level == 1) {
            return bufferInfo[i].Cache.LineSize;
        }
    }

    free(bufferInfo);
    return 0; // Unknown cache line size
#else
    long lineSize = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    if (lineSize == -1) lineSize = sysconf(_SC_LEVEL2_CACHE_LINESIZE);
    if (lineSize == -1) lineSize = sysconf(_SC_LEVEL3_CACHE_LINESIZE);
    return lineSize != -1 ? lineSize : 0; // 0 if unknown cache line size
#endif
}



void printTimeAndRestart(std::chrono::system_clock::time_point& start, const std::string& message)
{
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    int days = int(elapsed_seconds.count()) / (60 * 60 * 24);
    int hours = (int(elapsed_seconds.count()) / (60 * 60)) % 24;
    int minutes = (int(elapsed_seconds.count()) / 60) % 60;
    int seconds = int(elapsed_seconds.count()) % 60;

    // Extract milliseconds from the elapsed time
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_seconds).count() % 1000;

    // Get system and process memory information
    double totalSystemMemGiB = getSystemMemory();
    double processMemGiB = getProcessMemory();

    std::cout << message << " execution time: ";
    if (days > 0) {
        std::cout << days << "d ";
    }
    if (hours > 0) {
        std::cout << hours << "h ";
    }
    if (minutes > 0) {
        std::cout << minutes << "m ";
    }
    if (seconds > 0 || milliseconds > 0) {
        std::cout << seconds << "s ";
    }
    if (milliseconds > 0) {
        std::cout << milliseconds << "ms ";
    }
    std::cout << "\n";
    std::cout << "Memory usage: " << processMemGiB << " GiB / " << totalSystemMemGiB << " GiB (total)\n";

    start = std::chrono::system_clock::now();
}