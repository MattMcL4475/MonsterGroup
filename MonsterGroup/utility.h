#ifndef UTILITY_H
#define UTILITY_H

double getSystemMemory();
double getProcessMemory();
void printExecutionTime(std::chrono::system_clock::time_point& start, const std::string& message);

#endif
