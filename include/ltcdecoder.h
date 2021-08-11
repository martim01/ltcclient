#pragma once
#include "ltc.h"
#include "audioinput.h"
#include <string>

class LtcDecoder
{
    public:
        LtcDecoder();
        ~LtcDecoder();
        std::pair<bool, std::chrono::microseconds> DecodeLtc(const timedframe& buffer);

        const std::chrono::time_point<std::chrono::system_clock>& GetTime() { return m_tp;}

        const std::string& GetFrameStart() const;
        const std::string& GetFrameEnd() const;
        const std::string& GetAmplitude() const;
        const std::string& GetRaw() const;
        double GetFPS() const;
        const std::string& GetMode() const;
        const std::string& GetFormat() const;

        bool IsColourFlagSet() const;
        bool IsClockFlagSet() const;

        void SetDateMode(int nMode);

        enum {UNKNOWN, SMPTE, BBC, TVE, MTD};

    private:

        void CreateRaw();

        int WorkoutUserMode();
        std::chrono::microseconds DecodeDateAndTime(int nUserMode, std::chrono::time_point<std::chrono::system_clock> tp, ltc_off_t startSample);
        bool DecodeDateAndTime(SMPTETimecode& stime, int nDateMode);

        void ltc_frame_to_time_bbc(SMPTETimecode& stime);
        void ltc_frame_to_time_tve(SMPTETimecode& stime);
        void ltc_frame_to_time_mtd(SMPTETimecode& stime);
        void ltc_frame_to_time_only(SMPTETimecode& stime);

        LTCDecoder* m_pDecoder;
        LTCFrameExt m_Frame;
        std::string m_sDate;
        std::string m_sTime;
        std::string m_sFrameStart;
        std::string m_sFrameEnd;
        std::string m_sAmpltitude;
        std::string m_sFPS;
        std::string m_sRaw;
        std::string m_sMode;
        std::string m_sDateFormat;




        ltc_off_t m_nTotal;
        unsigned char m_nFPS;
        unsigned char m_nLastFrame;
        unsigned char m_nLastFPS;
        unsigned int m_nDateMode;
        double m_dFPS;

        std::chrono::time_point<std::chrono::system_clock> m_tp;

        static const int APV = 1920;

        static const std::string STR_MODE[4];
        static const std::string STR_DATE_MODE[5];
};

