#include <iostream>
#include "audioinput.h"
#include "ltcdecoder.h"
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "log.h"
#include <sys/time.h>
#include <cstring>
#include "offset.h"
#include "utils.h"

using namespace std;





void SetTime(const std::chrono::time_point<std::chrono::system_clock>& tp)
{
    timespec tv;
    tv.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    tv.tv_nsec =std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count() % 1000000000;

    auto local = std::chrono::system_clock::now();
    if(clock_settime(CLOCK_REALTIME, &tv) != 0)
    {
        pmlLog(pml::LOG_ERROR) << "Failed to hard crash " <<strerror(errno);
    }
    else
    {
        auto offset = std::chrono::system_clock::now()-local;
        pmlLog() << ConvertTimeToIsoString(tp) << "\tAdjustment = " << std::setw(6) << std::setfill(' ') << std::chrono::duration_cast<std::chrono::microseconds>(offset).count() << "us";
    }

}


int main()
{
    std::mutex mainMutex;
    std::condition_variable mainCv;

    pml::LogStream::AddOutput(std::make_unique<pml::LogOutput>());

    pmlLog(pml::LOG_TRACE) << "Create audio input";
    AudioInput ai(0, 48000, 2, mainCv);

    pmlLog(pml::LOG_TRACE) << "Start audio input";
    if(ai.Init() == false)
    {
        return -1;
    }


    LtcDecoder ltc;
    Offset data;

    pmlLog(pml::LOG_TRACE) << "Start loop";
    bool bLocked(false);
    bool bSynced(false);

    while(true)
    {
        std::unique_lock<std::mutex> lk(mainMutex);
        mainCv.wait(lk, [&ai]{ return ai.IsReady();});
        do
        {
            auto decode = ltc.DecodeLtc(ai.GetNextFrame());
            if(decode.first)
            {
                if(bLocked == false)
                {
                    pmlLog() << "Locked to LTC";
                    bLocked = true;
                }

                auto crashed = data.Add(decode.second, 0, ltc.GetFPS());

                if(data.IsSynced() && !bSynced)
                {
                    pmlLog() << "Synced to LTC";
                    bSynced =true;
                }
                else if(!data.IsSynced() && bSynced)
                {
                    pmlLog(pml::LOG_WARN) << "Lost sync to LTC";
                    bSynced = false;
                }
            }
        }while(ai.RemoveFrame());
    }


    return 0;
}
