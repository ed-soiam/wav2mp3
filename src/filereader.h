#pragma once
#include <list>
#include <vector>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "iodevice.h"
namespace buffered_io {
/* FileReader object is used for reading files with specific extension
 * from specific folder in his own thread.
 * FileReader stores limited amount of files in his memory(for memory usage reduction)
 * and caches new file only when other object gets stored file from FileReader
 * Usage: create FileReader object, call "setBufferLimit" to limit amount of files
 * that should be cached, call "start", call "processData" and "isFinished" to get
 * data and monitor FileReader state.
 */
class FileReader: public IODevice
{
public:
    FileReader(const std::string & path = "", const std::string & extension = ".wav");
    ~FileReader() override;

    void setPath(const std::string & value) override;

    void setBufferLimit(size_t value) override;

    void start() override;
    void stop() override;

    bool isFinished() const override;

    void processData(std::unique_ptr<DataItem> & data) override;
private:
    void threadWorker();

    DataItemUPtr read(const std::string & file);

    std::string m_path;
    std::string m_extension;
    std::list<std::string> m_files;
    std::list<DataItemUPtr> m_dataItems;
    std::atomic<size_t> m_limit = 1;
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_finished = false;

    std::mutex m_mutex;
    std::condition_variable m_innerCondVar;
    std::condition_variable m_readCondVar;
};
} //end of namespace
