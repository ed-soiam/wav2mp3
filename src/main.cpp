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
    try {
        DataProcessor dp(std::make_unique<buffered_io::FileReader>(argv[1]),
                std::make_unique<buffered_io::FileWriter>(argv[1]), threads);
        dp.start();
    }
    catch (std::exception & e)
    {
        std::cout << e.what() << std::endl;
        std::cout << "abnormal exit from application\n";
        return 1;
    }


    //171469ms for 1,6Gb Wav data, 44100Hz, 4 threads at intel i5-6200U CPU @ 2.30GHz
    //172390ms for 1,6Gb Wav data, 44100Hz, 8 threads at intel i5-6200U CPU @ 2.30GHz

    std::cout << "time elapsed from start: " <<
                 std::chrono::duration_cast<std::chrono::milliseconds>
                 (std::chrono::system_clock::now() - startTime).count() <<
                 "ms" << std::endl;
    return 0;
}
