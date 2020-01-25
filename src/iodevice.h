#pragma once
#include <iostream>
#include <vector>
#include <memory>

namespace buffered_io {

using DataVector = std::vector<char>;
using DataItem = std::pair<std::string, DataVector>;
using DataItemUPtr = std::unique_ptr<DataItem>;

class IODevice
{
public:
    virtual ~IODevice() = default;

    //Set path to current device
    virtual void setPath(const std::string & value) = 0;
    //set Items buffer limit for IODEvice. IODEvice should try to store no more than "value" items
    virtual void setBufferLimit(size_t value) = 0;
    //start iodevice
    virtual void start() = 0;
    //stop iodevice
    virtual void stop() = 0;
    //did current device finish all work
    virtual bool isFinished() const = 0;

    virtual void processData(std::unique_ptr<DataItem> & data) = 0;
};
} //end namespace
