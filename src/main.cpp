#include <iostream>
#include <chrono>
#include "lame.h"
#include "filereader.h"

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

    FileReader reader(argv[1]);
    reader.setBufferLimit(threads + 2);
    reader.start();
    DataItem data;
    while (!reader.isFinished())
        reader.processData(data);
    //19908ms for uncached(1,6Gb), 17316ms when cached
    std::cout << "time elapsed from start: " <<
                 std::chrono::duration_cast<std::chrono::milliseconds>
                 (std::chrono::system_clock::now() - startTime).count() <<
                 "ms" << std::endl;

    auto lame_settings = CDECL lame_init();

    //TODO: DataProcessor class

    lame_close (lame_settings);
    return 0;
}
