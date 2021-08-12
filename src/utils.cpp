#include "utils.h"
#include <ctime>
#include <iomanip>
#include <cmath>

std::string ConvertTimeToIsoString(std::time_t t, unsigned long nMicro)
{
    std::stringstream ss;

    ss << std::put_time(std::localtime(&t), "%FT%T") << "." << std::setw(3) << std::setfill('0') << nMicro;
    return ss.str();
}


std::string ConvertTimeToIsoString(std::chrono::time_point<std::chrono::system_clock> tp)
{
    std::time_t  t = std::chrono::system_clock::to_time_t(tp);
    return ConvertTimeToIsoString(t, std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count() % 1000000);
}

std::chrono::microseconds DoubleToMicro(double dDuration)
{
    return std::chrono::microseconds(static_cast<long long>(dDuration*1e6));

}
