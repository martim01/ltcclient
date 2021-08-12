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
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

using namespace std;

bool g_bRun = true;

static void sig(int signo)
{
        switch (signo)
        {
            case SIGSEGV:
            {
                void* arr[10];
                size_t nSize = backtrace(arr, 10);

                pmlLog(pml::LOG_CRITICAL)  << "Segmentation fault, aborting. " << nSize << std::endl;
                for(size_t i = 0; i < nSize; i++)
                {
                    pmlLog(pml::LOG_CRITICAL)  << std::hex << "0x" << reinterpret_cast<int>(arr[i]) <<std::endl;
                }

                _exit(1);
            }
        case SIGTERM:
        case SIGINT:
	    case SIGQUIT:
            {
                if (g_bRun)
                {
                    pmlLog(pml::LOG_WARN)  << "User abort" << std::endl;
                }
                g_bRun = false;
            }
	    break;
        }

}

void init_signals()
{
    signal (SIGTERM, sig);
    signal (SIGINT, sig);
    signal (SIGSEGV, sig);
    signal (SIGQUIT, sig);
}


int main()
{
    init_signals();


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

    while(g_bRun)
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
