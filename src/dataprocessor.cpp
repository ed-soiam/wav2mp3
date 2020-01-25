#include <future>
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
    m_lame = lame_init();
}

DataProcessor::~DataProcessor()
{
    lame_close(m_lame);
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
    m_out->processData(data);
    std::this_thread::sleep_for(10s);
    m_curThreads--;
    std::cout << "Threads: " << m_curThreads << ", completed task file " << data->first << std::endl;
    m_condVar.notify_one();
}
