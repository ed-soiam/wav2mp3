#include <filesystem>
#include <fstream>
#include "filewriter.h"

using namespace buffered_io;
namespace fs = std::filesystem;

FileWriter::FileWriter(const std::string & path, const std::string & extension):
    m_extension(extension)
{
    setPath(path);
}

FileWriter::~FileWriter()
{
    stop();
}

void FileWriter::setPath(const std::string & value)
{
    m_path = value;
}

void FileWriter::setBufferLimit(size_t value)
{
    m_limit = value > 0 ? value : 1;
}

void FileWriter::start()
{
    m_thread = std::make_unique<std::thread>(&FileWriter::threadWorker, this);
}

void FileWriter::stop()
{
    m_dataItems.clear();
    if (m_thread)
    {
        m_finished = true;
        m_condVar.notify_one();
        m_thread->join();
    }
}

bool FileWriter::isFinished() const
{
    std::unique_lock<std::mutex> s(m_mutex);
    return m_dataItems.empty();
}

void FileWriter::processData(std::unique_ptr<DataItem> & data)
{
    if (!data)
        return;
    std::unique_lock<std::mutex> s(m_mutex);
    m_dataItems.push_back(std::move(data));
    m_condVar.notify_one();
}

void FileWriter::threadWorker()
{
    m_dataItems.clear();
    m_finished = false;

    while (!m_finished)
    {
        DataItemUPtr write_item;
        {//mutex lock for short peiod
            std::unique_lock<std::mutex> lock(m_mutex);
            //if no files to write, just wait while someone add us writing task
            if (m_dataItems.empty())
            {
                m_condVar.wait(lock);
                continue;
            }
            write_item = std::move(*m_dataItems.begin());
            m_dataItems.erase(m_dataItems.begin());
        }

        //we are free to write at unlocked state
        write(std::move(write_item));
    }

    m_finished = true;
}

bool FileWriter::write(DataItemUPtr item)
{
    if (!item)
        return false;
    fs::path file = item->first;
    file.replace_extension(m_extension);
    if (fs::exists(file))
    {
        std::cout << "file " << file << "exists. Try to delete it\n";
        try
        {
            fs::remove(file);
        }
        catch (fs::filesystem_error & e)
        {
            std::cout << e.what() << std::endl;
            return false;
        }
    }

    std::ofstream ofs(file, std::ios::binary);
    if(!ofs)
    {
        std::cout << "output stream error : file " << file << std::endl;
        return false;
    }

    ofs.write(item->second.data(), static_cast<ptrdiff_t>(item->second.size()));
    ofs.close();
    if(ofs)
    {
        std::cout << "write to file " << file << std::endl;
        return true;
    }
    std::cout << "can not write to file " << file << std::endl;
    return false;
}
