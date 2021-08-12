#include "offset.h"
#include "linearregression.h"
#include "log.h"
#include <sys/timex.h>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "utils.h"


Offset::Offset() : m_dFPS(0.0), m_bSlewing(false), m_bSynced(false)
{

}

std::pair<bool, double> Offset::Add(std::chrono::microseconds offset, unsigned char nFrame, double dFPS)
{
    std::pair<bool, double> crashed(false, 0.0);
    if(dFPS != m_dFPS)
    {
        ClearData();
        m_dFPS = dFPS;
        m_nFrame = 0;

        pmlLog() << "Offset\tFPS change: " << m_dFPS;
    }
    else if(m_lstFrame.size() == 500 && m_dFPS != 0)
    {
        WorkoutLR();
        ClearData();
        m_nFrame = 0;

    }
    if(m_dFPS != 0)
    {
        m_lstOffset.push_back(static_cast<double>(offset.count())/1e6);
        m_lstFrame.push_back(m_lstFrame.size());


    }
    return crashed;
}

void Offset::ClearData()
{
    m_lstOffset.clear();
    m_lstFrame.clear();
}

void Offset::WorkoutLR()
{
    alphabeta ab = GetSlopeAndIntercept(m_lstFrame, m_lstOffset);
    ab.second*=1e6; //ppm
    ab.second*=m_dFPS;
    pmlLog() << "------------------------------------------------------- a=" << ab.first << "\tb=" << ab.second << " ppm";


    if(ab.second > -0.8 && ab.second < 0.8)
    {
        auto av = -GetAverage();
        if(av < -0.5 || av > 0.5)
        {
            CrashTime(av);
            ClearData();
            m_nFrame = 0;
            m_bSlewing = true;
        }
        else
        {
            double dSec;
            double dMicro = modf(-av, &dSec);
            dMicro*=1e6;
            timeval tv, tvOld;
            tv.tv_sec = dSec;
            tv.tv_usec = dMicro;
            if(adjtime(&tv, &tvOld) != 0)
            {
                pmlLog(pml::LOG_ERROR) << "Failed to adjtime " <<strerror(errno);
            }
            else
            {
                pmlLog() << "Time adjusted by " << tv.tv_sec << "s and " << tv.tv_usec << "us" << "\tLeft " << tvOld.tv_sec << "s and " << tvOld.tv_usec << "us";
            }
            m_bSlewing = true;
        }
    }
    else if(!m_bSlewing)
    {
        //change the frequency to get the slope to 0
        timex buf;
        memset(&buf, 0,sizeof(buf));
        if(adjtimex(&buf) == -1)
        {
            pmlLog(pml::LOG_ERROR) << "Failed to read frequency " <<strerror(errno);
            return;
        }
        pmlLog() << "Old freq=" << buf.freq;


        double dOffsetFreq = ab.second*65535.0;
        buf.freq += dOffsetFreq;
        buf.modes = ADJ_FREQUENCY;



        if(adjtimex(&buf) == -1)
        {
            pmlLog(pml::LOG_ERROR) << "Failed to set frequency " <<strerror(errno);
            return;
        }

        pmlLog() << "New freq=" << buf.freq;
    }
    else
    {
        pmlLog() << "Slewing - ignore this data set";

        timeval tvOld;
        if(adjtime(nullptr, &tvOld) != 0)
        {
            pmlLog(pml::LOG_ERROR) << "Failed to read offset " <<strerror(errno);
        }
        if(tvOld.tv_sec == 0 && tvOld.tv_usec == 0)
        {
            m_bSlewing = false;
        }
        else
        {
            pmlLog() << "Still adjusting "  << tvOld.tv_sec << "s and " << tvOld.tv_usec << "us";
        }
    }

    if(!m_bSlewing)
    {
        m_bSynced = (ab.first > -1e-4 && ab.first < 1e-4 && ab.second > -1.0 && ab.second < 1.0);
    }
}

double Offset::GetAverage()
{
    return std::accumulate(m_lstOffset.begin(), m_lstOffset.end(),0.0)/static_cast<double>(m_lstOffset.size());
}


void Offset::CrashTime(double dOffset)
{

    pmlLog() << "CrashTime: " << dOffset;
    auto now = std::chrono::system_clock::now();

    pmlLog() << "TimeWas: " << ConvertTimeToIsoString(now);

    now += DoubleToMicro(-dOffset);

    pmlLog() << "TimeWillbe: " << ConvertTimeToIsoString(now);

    timespec ts;
    ts.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    ts.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count() % 1000000000;


    if(clock_settime(CLOCK_REALTIME, &ts) != 0)
    {
        pmlLog(pml::LOG_ERROR) << "Failed to hard crash " <<strerror(errno);
        return;
    }

    pmlLog() << "Hard crashed to " << ConvertTimeToIsoString(std::chrono::system_clock::now());

}
