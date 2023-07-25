#include <fstream>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <string>

double getSystemMemory()
{
    std::ifstream file("/proc/meminfo");
    std::string line;

    while (getline(file, line))
    {
        if (line.find("MemTotal") != std::string::npos)
        {
            int pos = line.find(":");
            std::string memStr = line.substr(pos + 1);
            return std::stod(memStr) / (1024 * 1024);
        }
    }
    return 0.0;
}

double getProcessMemory()
{
    std::ifstream file("/proc/self/status");
    std::string line;

    while (getline(file, line))
    {
        if (line.find("VmRSS") != std::string::npos)
        {
            int pos = line.find(":");
            std::string memStr = line.substr(pos + 1);
            return std::stod(memStr) / (1024 * 1024);
        }
    }
    return 0.0;
}

void printExecutionTime(std::chrono::system_clock::time_point& start, const std::string& message)
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
        std::cout << std::setw(3) << std::setfill('0') << milliseconds << "ms ";
    }
    std::cout << "\n";
    std::cout << "Memory usage: " << processMemGiB << " GiB / " << totalSystemMemGiB << " GiB (total)\n";

    start = std::chrono::system_clock::now();
}