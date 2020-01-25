#include <iostream>
#include <chrono>
#include "filereader.h"
#include "filewriter.h"
#include "dataprocessor.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "folder argument wasn't specified" << std::endl;
        return 1;
    }

    //thread count approximating
    auto threads = std::thread::hardware_concurrency();
    if (threads == 0)
        threads = 16;

    auto startTime = std::chrono::system_clock::now();
    DataProcessor dp(std::make_unique<buffered_io::FileReader>(argv[1]),
            std::make_unique<buffered_io::FileWriter>(argv[1]), threads);
    dp.start();

    //19908ms for uncached(1,6Gb), 17316ms when cached
    std::cout << "time elapsed from start: " <<
                 std::chrono::duration_cast<std::chrono::milliseconds>
                 (std::chrono::system_clock::now() - startTime).count() <<
                 "ms" << std::endl;
    return 0;
}
