#include "filereader.h"
#include <experimental/filesystem>
#include <algorithm>
#include <fstream>
#include <cstring>

namespace fs = std::experimental::filesystem;
using namespace buffered_io;

FileReader::FileReader(const std::string & path, const std::string & extension):
    m_extension(extension)
{
    std::transform(m_extension.begin(), m_extension.end(), m_extension.begin(), ::tolower);
    setPath(path);
}

FileReader::~FileReader()
{
    stop();
}

void FileReader::setPath(const std::string & value)
{
    m_path = value;
}

void FileReader::setBufferLimit(size_t value)
{
    m_limit = value > 0 ? value : 1;
}

void FileReader::start()
{
    m_thread = std::make_unique<std::thread>(&FileReader::threadWorker, this);
}

void FileReader::stop()
{
    m_files.clear();
    m_dataItems.clear();
    if (m_thread)
    {
        m_finished = true;
        m_innerCondVar.notify_one();
        m_thread->join();
    }
    m_readCondVar.notify_all();
}

bool FileReader::isFinished() const
{
    return m_finished;
}

void FileReader::threadWorker()
{
    m_files.clear();
    m_dataItems.clear();
    m_finished = false;
    //get names of all files with appropriate extension
    fs::path path(m_path);
    for (const auto & file : fs::directory_iterator(path))
        if (fs::is_regular_file(file))
        {
            std::string extension = file.path().extension();
            std::transform(extension.begin(), extension.end(),
                           extension.begin(), ::tolower);
            if (m_extension == extension)
                m_files.push_back(file.path());
        }

    while (!m_finished)
    {
        //if limit exceeds, wait that someone reads our data
        if (m_dataItems.size() >= m_limit)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_innerCondVar.wait(lock);
            continue;
        }

        //if no files to read, just wait while someone read all remained data
        if (m_files.empty())
        {
            //if all data is read, exit thread and mark device as finished
            if (m_dataItems.empty())
                break;
            std::unique_lock<std::mutex> lock(m_mutex);
            m_innerCondVar.wait(lock);
            continue;
        }

        auto data = read(*m_files.begin());

        //only if read operation is successful
        if (!data->second.empty())
        {//lock mutex only for push operation
            std::unique_lock<std::mutex> lock(m_mutex);
            m_dataItems.push_back(std::move(data));
        }

        //notifiers to subscribers, that there is data to read
        m_readCondVar.notify_all();

        m_files.erase(m_files.begin());
    }

    m_finished = true;
    m_readCondVar.notify_all();
}

void FileReader::processData(std::unique_ptr<DataItem> & data)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    //wait only if there is no data and reader hasn't loaded all files yet
    if (m_dataItems.empty() && !isFinished())
        m_readCondVar.wait(lock);
    if (m_dataItems.empty())
        return;
    data = std::make_unique<DataItem>();
    std::swap(data, *m_dataItems.begin());
    m_dataItems.erase(m_dataItems.begin());
    m_innerCondVar.notify_one();
}

DataItemUPtr FileReader::read(const std::string & file)
{
    DataItemUPtr result = std::make_unique<DataItem>(file, DataVector());
    uintmax_t size = 0;
    try
    {
        size = fs::file_size(file);
        if (size == 0)
            return result;

    }
    catch (fs::filesystem_error& e)
    {
        std::cout << e.what() << std::endl;
        return result;
    }

    std::ifstream ifs(file, std::ios::binary);
    if(!ifs)
    {
        std::cout << "input stream error : file " << file << std::endl;
        return result;
    }

    result->second.resize(size);

    if(!ifs.read(result->second.data(), static_cast<ptrdiff_t>(result->second.size())))
    {
        result->second.clear();
        std::cout << "can not read file " << file << std::endl;
    }
    std::cout << "file " << file << " size=" << size << std::endl;
    return result;
}
