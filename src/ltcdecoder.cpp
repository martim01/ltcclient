#include "ltcdecoder.h"
#include <bitset>
#include <algorithm>
#include "log.h"
#include "utils.h"
#include <cmath>


const std::string LtcDecoder::STR_MODE[4] = {"Not specified","8-bit","Date","Page/Line"};
const std::string LtcDecoder::STR_DATE_MODE[5] = {"Unknown","SMPTE","BBC","TVE","MTD"};


LtcDecoder::LtcDecoder() :
    m_pDecoder(ltc_decoder_create(APV, 32)),
    m_nTotal(0),
    m_nFPS(0),
    m_nLastFrame(0),
    m_nDateMode(UNKNOWN)
{
}

LtcDecoder::~LtcDecoder()
{
    ltc_decoder_free(m_pDecoder);
}


std::pair<bool, std::chrono::microseconds> LtcDecoder::DecodeLtc(const timedframe& frame)
{
   std::pair<bool, std::chrono::microseconds> decode(false, std::chrono::microseconds(0));

    ltc_decoder_write_float(m_pDecoder, frame.second.data(), frame.second.size(), m_nTotal);
    while (ltc_decoder_read(m_pDecoder, &m_Frame))
    {
        decode.first = true;
        int nMode = WorkoutUserMode();

        decode.second = DecodeDateAndTime(nMode, frame.first, m_Frame.off_start);

        m_sFrameStart = std::to_string(m_Frame.off_end - m_Frame.off_start);
        m_sFrameEnd = std::to_string(m_Frame.off_end);  // -> use this or the above and a timestamp to work out exactly when we got this bit of LTC
        m_sAmpltitude = std::to_string(m_Frame.volume);


        CreateRaw();

    }
    m_nTotal += frame.second.size();
    return decode;
}

const std::string& LtcDecoder::GetFrameStart() const
{
    return m_sFrameStart;
}

const std::string& LtcDecoder::GetFrameEnd() const
{
    return m_sFrameEnd;
}

const std::string& LtcDecoder::GetAmplitude() const
{
    return m_sAmpltitude;
}

const std::string& LtcDecoder::GetRaw() const
{
    return m_sRaw;
}

double LtcDecoder::GetFPS() const
{
    return m_dFPS;
}

const std::string& LtcDecoder::GetFormat() const
{
    return m_sDateFormat;
}

void LtcDecoder::SetDateMode(int nMode)
{
    m_nDateMode = nMode%5;
}


void LtcDecoder::CreateRaw()
{
    std::string sRaw;
    std::string str;

    str = std::bitset<4>(m_Frame.ltc.frame_units).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.user1).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<2>(m_Frame.ltc.frame_tens).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<1>(m_Frame.ltc.dfbit).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<1>(m_Frame.ltc.col_frame).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.user2).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.secs_units).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.user3).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<3>(m_Frame.ltc.secs_tens).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<1>(m_Frame.ltc.biphase_mark_phase_correction).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.user4).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.mins_units).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.user5).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<3>(m_Frame.ltc.mins_tens).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<1>(m_Frame.ltc.binary_group_flag_bit0).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.user6).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.hours_units).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.user7).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<2>(m_Frame.ltc.hours_tens).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<1>(m_Frame.ltc.binary_group_flag_bit1).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<1>(m_Frame.ltc.binary_group_flag_bit2).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<4>(m_Frame.ltc.user8).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;
    sRaw+= " ";
    str = std::bitset<16>(m_Frame.ltc.sync_word).to_string();
    std::reverse(str.begin(), str.end());
    sRaw+=str;


    m_sRaw = sRaw;
}

int LtcDecoder::WorkoutUserMode()
{
    int nbit0 = m_Frame.ltc.binary_group_flag_bit0;
    int nbit1 = m_Frame.ltc.binary_group_flag_bit1;
    int nbit2 = m_Frame.ltc.binary_group_flag_bit2;
    if(m_nFPS == 24)
    {
        nbit0 = m_Frame.ltc.biphase_mark_phase_correction;
        nbit2 = m_Frame.ltc.binary_group_flag_bit0;
    }

    int nMode = nbit0+(nbit2*2);


    m_sMode = STR_MODE[nMode%4];

    return nMode;
}

std::chrono::microseconds LtcDecoder::DecodeDateAndTime(int nUserMode, std::chrono::time_point<std::chrono::system_clock> tp, ltc_off_t startSample)
{
    //work out the time we received this frame
    double diff = startSample-m_nTotal;
    diff /= 48000.0;    //@todo the actual sample rate
    tp += DoubleToMicro(diff);



    //now work out the time this frame says
    SMPTETimecode stime;
    ltc_frame_to_time_only(stime);

    if(nUserMode == 0 || nUserMode == 2)
    {
        int nDateMode =  m_nDateMode;
        if(nDateMode == UNKNOWN)
        {
            while(nDateMode != MTD)
            {
                nDateMode++;
                if(DecodeDateAndTime(stime, nDateMode))
                    break;
            }
        }
        else
        {
            DecodeDateAndTime(stime, nDateMode);
        }
        m_sDateFormat = STR_DATE_MODE[nDateMode];
    }

    //now convert to a chrono
    std::tm ltcTm;
    if(stime.years < 67)
    {
        ltcTm.tm_year = 100+stime.years;
    }
    else
    {
        ltcTm.tm_year = stime.years;
    }

    ltcTm.tm_mon = stime.months-1;
    ltcTm.tm_mday = stime.days;
    ltcTm.tm_hour = stime.hours;
    ltcTm.tm_min = stime.mins;
    ltcTm.tm_sec = stime.secs;

    m_tp = std::chrono::system_clock::from_time_t(mktime(&ltcTm));

    //convert frame to milliseconds
    if(m_nFPS != 0)
    {
        if(m_Frame.ltc.dfbit == 0)
        {
            m_dFPS = m_nFPS+1;
        }
        else
        {
            double m_dFPS = m_nFPS+1;
            m_dFPS -= (1.0/m_dFPS);
        }

        unsigned long milli = (1000.0/m_dFPS)*static_cast<double>(stime.frame);
        m_tp += std::chrono::milliseconds(milli);


    }
    auto difference = std::chrono::duration_cast<std::chrono::microseconds>(m_tp-tp);

   // pmlLog() << "Frame At: " << ConvertTimeToIsoString(tp) << "\tLTC: " << ConvertTimeToIsoString(m_tp)<< "\tOffset: " << difference.count();

    return difference;
}


bool LtcDecoder::DecodeDateAndTime(SMPTETimecode& stime, int nDateMode)
{
    switch(nDateMode)
    {
        case SMPTE:
            ltc_frame_to_time(&stime, &m_Frame.ltc, 1);
            break;
        case BBC:
            ltc_frame_to_time_bbc(stime);
            break;
        case TVE:
            ltc_frame_to_time_tve(stime);
            break;
        case MTD:
            ltc_frame_to_time_mtd(stime);
            break;

    }
    if(stime.days < 1 || stime.days > 31 || stime.months < 1 || stime.months > 12)
    {
        return false;
    }
    return true;
}

void LtcDecoder::ltc_frame_to_time_only(SMPTETimecode& stime)
{
    stime.hours = m_Frame.ltc.hours_units + m_Frame.ltc.hours_tens*10;
    stime.mins  = m_Frame.ltc.mins_units  + m_Frame.ltc.mins_tens*10;
    stime.secs  = m_Frame.ltc.secs_units  + m_Frame.ltc.secs_tens*10;
    stime.frame = m_Frame.ltc.frame_units + m_Frame.ltc.frame_tens*10;

    if(stime.frame == 0 && m_nLastFrame != 0)
    {
        m_nFPS = m_nLastFrame;
    }
    m_nLastFrame = stime.frame;
}


void LtcDecoder::ltc_frame_to_time_bbc(SMPTETimecode& stime)
{
   stime.years = m_Frame.ltc.user6 + m_Frame.ltc.user8*10;
   stime.months = m_Frame.ltc.user3;
   if((m_Frame.ltc.user4&0x4)!=0)
   {
        stime.months += 10;
   }
    stime.days = m_Frame.ltc.user2 + (m_Frame.ltc.user4&0x3)*10;

    sprintf(stime.timezone,"+0");
}

void LtcDecoder::ltc_frame_to_time_tve(SMPTETimecode& stime)
{
    stime.years  = m_Frame.ltc.user6 + m_Frame.ltc.user7*10;
    stime.months = m_Frame.ltc.user4 + m_Frame.ltc.user5*10;
    stime.days   = m_Frame.ltc.user2 + m_Frame.ltc.user3*10;

    sprintf(stime.timezone,"+0");
}


void LtcDecoder::ltc_frame_to_time_mtd(SMPTETimecode& stime)
{
    stime.years  = m_Frame.ltc.user2 + m_Frame.ltc.user1*10;
    stime.months = m_Frame.ltc.user4 + m_Frame.ltc.user3*10;
    stime.days   = m_Frame.ltc.user6 + m_Frame.ltc.user5*10;

    switch((m_Frame.ltc.user7 & 0x3))
    {
        case 0:
        case 3:
            sprintf(stime.timezone,"+0");
            break;
        case 1:
            sprintf(stime.timezone,"+1");
            break;
        case 2:
            sprintf(stime.timezone,"+2");
            break;
    }
}

const std::string& LtcDecoder::GetMode() const
{
    return m_sMode;
}

bool LtcDecoder::IsColourFlagSet() const
{
    return (m_Frame.ltc.col_frame!=0);
}

bool LtcDecoder::IsClockFlagSet() const
{
    return (m_Frame.ltc.binary_group_flag_bit1!=0);
}


