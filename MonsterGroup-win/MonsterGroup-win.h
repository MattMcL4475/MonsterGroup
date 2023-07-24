#pragma once

void printExecutionTime(std::chrono::system_clock::time_point& start, const std::string& message);

void initializeAndMultiplyMatrices(std::vector<std::thread>& threads, unsigned int numThreads, const size_t& rowsPerThread, unsigned int seed, std::chrono::_V2::system_clock::time_point& start, std::vector<std::vector<std::complex<double>>>& matrixA);
