#include <iostream>
#include "lame.h"
using namespace std;

int main(int argc, char **argv)
{
    auto lame_settings = CDECL lame_init();

    std::cout << lame_settings << std::endl;
    lame_close (lame_settings);
    return 0;
}
