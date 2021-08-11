#pragma once
#include <string>
#include <chrono>


extern std::string ConvertTimeToIsoString(std::time_t t, unsigned long nMicro);

extern std::string ConvertTimeToIsoString(std::chrono::time_point<std::chrono::system_clock> tp);

extern std::chrono::microseconds DoubleToMicro(double dDuration);
