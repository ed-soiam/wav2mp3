#pragma once
#include <memory>
#include <atomic>
#include <exception>
#include <condition_variable>
#include "iodevice.h"
#include "lame.h"

#pragma pack (push, 1)
struct WavHeader
{
    // "RIFF" in ASCII(0x52494646 in big-endian)
    char chunkId[4];

    uint32_t chunkSize;

    // "WAVE"(0x57415645 in big-endian)
    char format[4];

    // "fmt "(0x666d7420 in big-endian)
    char subchunk1Id[4];

    // 16 for pcm
    uint32_t subchunk1Size;

    //coding format (pcm = 1)
    uint16_t audioFormat;

    // mono = 1, stereo = 2 , etc
    uint16_t numChannels;

    // 8KHz, 44,1KHz, etc
    uint32_t sampleRate;

    // sampleRate * numChannels * bitsPerSample/8
    uint32_t byteRate;

    // numChannels * bitsPerSample/8
    uint16_t blockAlign;

    uint16_t bitsPerSample;

    // "data"(0x64617461 in big-endian)
    char subchunk2Id[4];

    // Total data bytes for all channels numSamples * numChannels * bitsPerSample/8
    uint32_t subchunk2Size;
};
#pragma pack (pop)

class DataProcessor
{
public:    
    DataProcessor(std::unique_ptr<buffered_io::IODevice> && input,
                  std::unique_ptr<buffered_io::IODevice> && output, size_t threads);
    virtual ~DataProcessor();
    void start();
private:

    void processData(std::unique_ptr<buffered_io::DataItem> data);
    //std::tuple<number of channels(0- fail), sampling rate, bytes per sample, samples> */
    using WaveTuple = std::tuple<uint16_t, uint32_t, uint16_t, uint32_t>;
    WaveTuple waveParameters(const buffered_io::DataVector & data) const;

    std::unique_ptr<buffered_io::IODevice> m_in;
    std::unique_ptr<buffered_io::IODevice> m_out;
    std::atomic<size_t> m_curThreads = 0; //current launched threads
    size_t m_threads = 1; //max threads number
    std::condition_variable m_condVar;
    std::mutex m_mutex;
};
