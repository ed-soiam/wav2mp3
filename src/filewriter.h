#pragma once
#include <list>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "iodevice.h"

namespace buffered_io {

class FileWriter : public IODevice
{
public:
    FileWriter(const std::string & path = "", const std::string & extension = ".mp3");
    ~FileWriter() override;

    void setPath(const std::string & value) override;

    void setBufferLimit(size_t value) override;

    void start() override;
    void stop() override;

    bool isFinished() const override;

    void processData(std::unique_ptr<DataItem> & data) override;
private:
    void threadWorker();

    bool write(DataItemUPtr item);

    std::string m_path;
    std::string m_extension;

    std::list<std::unique_ptr<DataItem>> m_dataItems;
    std::atomic<size_t> m_limit = 1;
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_finished = false;

    mutable std::mutex m_mutex;
    std::condition_variable m_condVar;
};

} //end of namespace
