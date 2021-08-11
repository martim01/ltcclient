#include "audioinput.h"
#include "log.h"
#include "pa_linux_alsa.h"
#include "utils.h"
#include <cmath>

int paCallback( const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData )
{
    if(userData)
    {
        AudioInput* pComp = reinterpret_cast<AudioInput*>(userData);
        pComp->Callback(reinterpret_cast<const float*>(input), frameCount, timeInfo, statusFlags);
    }

    return 0;
}


AudioInput::AudioInput(unsigned long nDevice, unsigned long nSampleRate, unsigned char nChannels, std::condition_variable& cv) :
    m_nDevice(nDevice),
    m_nSampleRate(nSampleRate),
    m_nChannels(nChannels),
    m_cv(cv),
    m_pStream(nullptr)
{

}

AudioInput::~AudioInput()
{
    if(m_pStream)
    {

        PaError err = Pa_AbortStream(m_pStream);
        err = Pa_CloseStream(m_pStream);
        if(err != paNoError)
        {
            pmlLog(pml::LOG_ERROR) << "AudioInput\tFailed to stop PortAudio stream: " << Pa_GetErrorText(err);
        }
    }
    Pa_Terminate();
}


bool AudioInput::Init()
{
    if(Pa_Initialize() != paNoError)
    {
        pmlLog(pml::LOG_CRITICAL) << "AudioInput\tCould not initialize PortAudio";
        return false;
    }
    return OpenStream();
}


bool AudioInput::OpenStream()
{
    pmlLog() << "AudioInput\tAttempt to open device " << m_nDevice;


    PaStreamParameters inputParameters;

    const PaDeviceInfo* pInfo = Pa_GetDeviceInfo(m_nDevice);
    if(pInfo)
    {
        if(pInfo->maxInputChannels < 2)
        {
            m_nChannels = pInfo->maxInputChannels;
            pmlLog() << "AudioInput\tInput channels changed to " << m_nChannels;
        }

    }


    inputParameters.channelCount = m_nChannels;
    inputParameters.device = m_nDevice;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = 0;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    double dLatency(.04);

    PaError err;

    pmlLog() << "AudioInput\tAttempt to open " << m_nChannels << " channel INPUT stream on device " << m_nDevice;
    err = Pa_OpenStream(&m_pStream, &inputParameters, 0, m_nSampleRate, 1024, paNoFlag, paCallback, reinterpret_cast<void*>(this) );

    if(err == paNoError)
    {
        m_tpOpen = std::chrono::system_clock::now();
        m_OpenTime = Pa_GetStreamTime(m_pStream);


        err = Pa_StartStream(m_pStream);
        if(err == paNoError)
        {
            PaAlsa_EnableRealtimeScheduling(m_pStream,1);
            pmlLog() << "AudioInput\tDevice " << m_nDevice << " opened";
            const PaStreamInfo* pStreamInfo = Pa_GetStreamInfo(m_pStream);
            if(pStreamInfo)
            {
                pmlLog() << "AudioInput\tStreamInfo: Input Latency " << pStreamInfo->inputLatency << " Sample Rate " << pStreamInfo->sampleRate;
            }
            return true;
        }
    }
    m_pStream = 0;
    pmlLog(pml::LOG_ERROR) << "Audio\tFailed to open device " << m_nDevice << " " << Pa_GetErrorText(err) << " with sample rate=" << m_nSampleRate << " input channels=" << m_nChannels;

    return false;
}


void AudioInput::Callback(const float* pBuffer, size_t nFrameCount, const PaStreamCallbackTimeInfo* pTimeInfo, int nFlags)
{
    //time of first sample of frame is open time + difference
    auto tpNow = std::chrono::system_clock::now();

    auto diff = pTimeInfo->currentTime - pTimeInfo->inputBufferAdcTime;
    auto tpFirst = tpNow - DoubleToMicro(diff);


    aframe af;
    af.reserve(nFrameCount);
    for(size_t i = 0; i < nFrameCount*m_nChannels; i+=m_nChannels)
    {
        af.push_back(pBuffer[i]);
    }

    m_mutex.lock();
    m_qBuffer.push(timedframe(tpFirst, af));
    m_mutex.unlock();

    //@todo tell main thread that we have audio...
    m_cv.notify_one();
}

bool AudioInput::IsReady()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_qBuffer.empty() == false;
}

const timedframe& AudioInput::GetNextFrame()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_qBuffer.front();
}

bool AudioInput::RemoveFrame()
{
    std::lock_guard<std::mutex> lg(m_mutex);
    m_qBuffer.pop();
    return m_qBuffer.empty() == false;
}


void AudioInput::OffsetOpenTime(double dOffset)
{
    std::lock_guard<std::mutex> lg(m_mutex);

    m_tpOpen += DoubleToMicro(dOffset);
}
