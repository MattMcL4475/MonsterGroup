#ifndef UTILITY_H
#define UTILITY_H

double getSystemMemory();
double getProcessMemory();
double getCPUClockSpeed();
int getCacheLineSize();
void printTimeAndRestart(std::chrono::system_clock::time_point& start, const std::string& message);

#endif
