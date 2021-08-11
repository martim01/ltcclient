#pragma once
#include "portaudio.h"
#include <queue>
#include <mutex>
#include <condition_variable>

using aframe = std::vector<float>;
using timedframe = std::pair<std::chrono::time_point<std::chrono::system_clock>, aframe>;

class AudioInput
{
    public:
        AudioInput(unsigned long nDevice, unsigned long nSampleRate, unsigned char nChannels, std::condition_variable& cv);
        ~AudioInput();

        bool Init();

        void Callback(const float* pBuffer, size_t nFrameCount,const PaStreamCallbackTimeInfo* pTimeInfo, int nFlags);

        bool IsReady();
        const timedframe& GetNextFrame();
        bool RemoveFrame();

        void OffsetOpenTime(double dOffset);

    private:

        bool OpenStream();

        unsigned long m_nDevice;
        unsigned long m_nSampleRate;
        unsigned char m_nChannels;

        std::mutex m_mutex;
        std::condition_variable& m_cv;

        PaStream* m_pStream;

        std::queue<timedframe> m_qBuffer;

        PaTime m_OpenTime;
        std::chrono::time_point<std::chrono::system_clock> m_tpOpen;
};

int paCallback( const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData );
