#include <future>
#include <cstring>
#include "dataprocessor.h"

using namespace std::chrono_literals;

DataProcessor::DataProcessor(std::unique_ptr<buffered_io::IODevice> && input,
                             std::unique_ptr<buffered_io::IODevice> && output,
                             size_t threads):
    m_in(std::move(input)),
    m_out(std::move(output))
{
    m_threads = threads > 0 ? threads : 1;
    m_in->setBufferLimit(m_threads + 2);
    m_out->setBufferLimit(m_threads + 2);
}

DataProcessor::~DataProcessor()
{
}

void DataProcessor::start()
{
    if (!m_in || !m_out)//just some simple check-ups before start working
        return;
    m_in->start();
    m_out->start();
    std::unique_ptr<buffered_io::DataItem> data;
    while (!m_in->isFinished())
    {
        if (m_curThreads >= m_threads)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condVar.wait(lock);
            continue;
        }
        m_in->processData(data);
        m_curThreads++;
        std::cout << "Threads: " << m_curThreads << ", new task file " << data->first << std::endl;
        std::thread(&DataProcessor::processData, this, std::move(data)).detach();
    }

    //wait for all threads finish their work and results of conversion are saved
    while (m_curThreads || !m_out->isFinished())
        std::this_thread::sleep_for(50ms);
}


void DataProcessor::processData(std::unique_ptr<buffered_io::DataItem> data)
{
    auto [channels, sampleRate, sampleLen, samples] = waveParameters(data.get()->second);

    //check current memory alignment of data samples according to sample lenght in bytes
    void * dataPtr = data.get()->second.data() + sizeof(WavHeader);
    if (reinterpret_cast<size_t>((data.get()->second.data() + sizeof(WavHeader))) % sizeof(sampleLen / 8))
    {
        data.get()->second.erase(data.get()->second.begin(), data.get()->second.begin() + sizeof(WavHeader));
        dataPtr = data.get()->second.data();
    }
    lame_global_flags * lame = lame_init();
    /* lame initialization and configuration */
    if (lame)
    {
        lame_set_num_channels(lame, channels);
        lame_set_in_samplerate(lame, static_cast<int>(sampleRate));
        lame_set_brate(lame , 128);
        lame_set_mode(lame, STEREO);
        lame_set_quality(lame, 5);   /* 2 = near-best  5 = good  7 = ok  9 = worst*/
        if (lame_init_params(lame) < 0)
        {
            std::cerr << "could not configure lame\n";
            lame_close(lame);
            lame = nullptr;
        }
    }
    else
        std::cerr << "could not init lame\n";
    if (lame)
    {
        int size = 0; //variable for storing size of data after lame encoder
        buffered_io::DataVector mp3Data; //output lame buffers

        switch (channels)
        {
        case 0:
            std::cerr << "bad wav file " << data->first << std::endl;
            break;
        case 1:
            break;
        case 2:
            mp3Data.resize(static_cast<size_t>(1.25 * samples) + 7200);//this buffer is enough even for worst case
            size = lame_encode_buffer_interleaved(lame, reinterpret_cast<short int *>(dataPtr), static_cast<int>(samples),
                                           reinterpret_cast<unsigned char *>(mp3Data.data()), static_cast<int>(mp3Data.size()));
            break;
        default:
            std::cerr << "program doesn't support file " << data->first << std::endl;
            break;
        }

        //postprocessing before saving data to file
        if (size > 0)
        {
            mp3Data.resize(static_cast<size_t>(size));
            std::swap(mp3Data, data->second);
            m_out->processData(data);
        }
        else
            std::cerr << "Error encoding file " << data->first << std::endl;

        lame_close(lame);
    }
    m_curThreads--;
    m_condVar.notify_one();
}

DataProcessor::WaveTuple DataProcessor::waveParameters(const buffered_io::DataVector & data) const
{
    WaveTuple result = std::make_tuple(0, 0, 0, 0);

    //some usual format check-ups
    if (data.size() <= sizeof(WaveTuple))
        return result;
    const WavHeader * header = reinterpret_cast<const WavHeader *>(data.data());
    if (strncmp(header->chunkId, "RIFF", sizeof(header->chunkId)))
        return result;
    if (strncmp(header->format, "WAVE", sizeof(header->format)))
        return result;
    if (strncmp(header->subchunk1Id, "fmt ", sizeof(header->subchunk1Id)))
        return result;
    if (strncmp(header->subchunk2Id, "data", sizeof(header->subchunk2Id)))
        return result;

    //if file is cut(not full)
    uint32_t dataSize = std::min(static_cast<uint32_t>(data.size() - sizeof(WaveTuple)),
                                 header->subchunk2Size);

    std::get<0>(result) = header->numChannels;
    std::get<1>(result) = header->sampleRate;
    std::get<2>(result) = header->bitsPerSample;
    std::get<3>(result) = dataSize / (header->numChannels * (header->bitsPerSample / 8));
    return result;
}
