
cmake_minimum_required(VERSION 3.16)
project(video_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED
    libavformat
    libavcodec
    libavutil
    libswscale
)

find_package(OpenSSL REQUIRED)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# 单独使用QT5模块也需要Qt5::Core, 
find_package(Qt5 REQUIRED COMPONENTS Core Network)

add_executable(video_parser
    src/src/main.cpp
    src/src/video_to_jpeg.cpp
    src/src/video_to_nv12.cpp
    src/src/common.cpp
)

add_executable(server
    src/src/server.cpp
)

add_executable(client
    src/src/client.cpp
)

add_executable(server_async
    src/src/server_async.cpp
)

add_executable(client_async
    src/src/client_async.cpp
)

add_executable(server_qt5
    src/src/server_qt5.cpp
)

add_executable(client_qt5
    src/src/client_qt5.cpp
)

target_include_directories(video_parser PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(video_parser PRIVATE ${FFMPEG_LIBRARIES})
target_link_libraries(video_parser
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_link_libraries(server PRIVATE Boost::system Threads::Threads)
target_link_libraries(client PRIVATE Boost::system Threads::Threads)


target_link_libraries(server_async PRIVATE Boost::system Threads::Threads)
target_link_libraries(client_async PRIVATE Boost::system Threads::Threads)


target_link_libraries(server_qt5
    Qt5::Core
    Qt5::Network
)

target_link_libraries(client_qt5
    Qt5::Core
    Qt5::Network
)
