#pragma once
#include <memory>
#include <atomic>
#include <condition_variable>
#include "iodevice.h"
#include "lame.h"

class DataProcessor
{
public:
    DataProcessor(std::unique_ptr<buffered_io::IODevice> && input,
                  std::unique_ptr<buffered_io::IODevice> && output, size_t threads);
    virtual ~DataProcessor();
    void start();
private:
    void processData(std::unique_ptr<buffered_io::DataItem> data);
    std::unique_ptr<buffered_io::IODevice> m_in;
    std::unique_ptr<buffered_io::IODevice> m_out;
    std::atomic<size_t> m_curThreads = 0; //current launched threads
    size_t m_threads = 1; //max threads number
    std::condition_variable m_condVar;
    std::mutex m_mutex;
    lame_global_flags * m_lame;
};
