#pragma once
#include "iodevice.h"
namespace buffered_io {

class FileWriter : public IODevice
{
public:
    FileWriter(const std::string & path = "", const std::string & extension = ".mp3");
    ~FileWriter() override = default;

    void setPath(const std::string & value) override {}

    void setBufferLimit(size_t value) override {}

    void start() override {}
    void stop() override {}

    bool isFinished() const override {return true;}

    void processData(std::unique_ptr<DataItem> & data) override {}
};

} //end of namespace
