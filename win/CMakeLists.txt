cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
INCLUDE(ExternalProject)
project(wav2mp3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

ExternalProject_Add(lame
    SVN_REPOSITORY      https://svn.code.sf.net/p/lame/svn/trunk/lame
    SVN_TRUST_CERT      1
    PREFIX              lame-svn
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/../lib/lame
    CONFIGURE_COMMAND   ""
    BUILD_IN_SOURCE     1
    BUILD_COMMAND       msbuild "../lame.vcxproj"
    INSTALL_COMMAND     ""
)
ExternalProject_Get_Property(lame SOURCE_DIR)
SET(liblamefile_dir "${SOURCE_DIR}/libmp3lame/.libs")
ADD_LIBRARY(liblamefile STATIC IMPORTED)
SET_PROPERTY(TARGET liblamefile PROPERTY IMPORTED_LOCATION "${liblamefile_dir}/lame.lib")
SET(SRC
    ../src/main.cpp
    ../src/filereader.cpp
    ../src/filewriter.cpp
    ../src/dataprocessor.cpp
    )
add_executable(${PROJECT_NAME} ${SRC})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} liblamefile)



include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/lame/include)


