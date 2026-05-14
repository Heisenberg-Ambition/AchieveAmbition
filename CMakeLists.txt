cmake_minimum_required(VERSION 3.20)

project(XinTools LANGUAGES C CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(HELLOIMGUI_USE_GLFW3 ON CACHE BOOL "" FORCE)
set(HELLOIMGUI_USE_SDL2 OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)
set(HELLOIMGUI_HAS_METAL OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_HAS_VULKAN OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_HAS_DIRECTX11 OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_HAS_DIRECTX12 OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_WIN32_AUTO_WINMAIN OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_WIN32_NO_CONSOLE OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_USE_FREETYPE OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_STB_IMAGE_IMPLEMENTATION ON CACHE BOOL "" FORCE)
set(HELLOIMGUI_STB_IMAGE_WRITE_IMPLEMENTATION OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_DOWNLOAD_SDL_IF_NEEDED OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED OFF CACHE BOOL "" FORCE)

set(HELLOIMGUI_IMGUI_SOURCE_DIR
    "${CMAKE_SOURCE_DIR}/third_party/imgui_bundle-main/external/imgui/imgui"
    CACHE STRING "" FORCE
)

set(build_kcd OFF CACHE BOOL "" FORCE)
set(build_tools OFF CACHE BOOL "" FORCE)
set(build_tests OFF CACHE BOOL "" FORCE)
set(build_examples OFF CACHE BOOL "" FORCE)

# 开启大文件编译支持
add_compile_options(/bigobj)

# 在Visual Studio中开启多线程编译支持
if (MSVC)
    add_compile_options(/MP)
endif()

# 开启UTF-8编码
add_compile_options(/utf-8)

add_subdirectory(third_party/imgui_bundle-main/external/glfw/glfw EXCLUDE_FROM_ALL)
add_subdirectory(third_party/imgui_bundle-main/external/hello_imgui/hello_imgui EXCLUDE_FROM_ALL)
add_subdirectory(third_party/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(third_party/dbcppp EXCLUDE_FROM_ALL)

if(TARGET dbcppp)
    target_compile_definitions(dbcppp PRIVATE DBCPPP_EXPORT)
endif()

add_library(implot STATIC
    third_party/implot-official/implot.cpp
    third_party/implot-official/implot_items.cpp
)
target_include_directories(implot PUBLIC
    "${CMAKE_SOURCE_DIR}/third_party/implot-official"
)
target_compile_definitions(implot PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_link_libraries(implot PUBLIC imgui)

add_library(imgui_toggle STATIC
    third_party/imgui_bundle-main/external/imgui_toggle/imgui_toggle/imgui_toggle.cpp
    third_party/imgui_bundle-main/external/imgui_toggle/imgui_toggle/imgui_toggle_palette.cpp
    third_party/imgui_bundle-main/external/imgui_toggle/imgui_toggle/imgui_toggle_presets.cpp
    third_party/imgui_bundle-main/external/imgui_toggle/imgui_toggle/imgui_toggle_renderer.cpp
)
target_include_directories(imgui_toggle PUBLIC
    "${CMAKE_SOURCE_DIR}/third_party/imgui_bundle-main/external/imgui_toggle"
)
target_compile_definitions(imgui_toggle PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_link_libraries(imgui_toggle PUBLIC imgui)

set(OpenCV_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/opencv/build/include")
set(OpenCV_LIBRARY "${CMAKE_SOURCE_DIR}/third_party/opencv/build/x64/vc16/lib/opencv_world490.lib")
set(OpenCV_RUNTIME "${CMAKE_SOURCE_DIR}/third_party/opencv/build/x64/vc16/bin/opencv_world490.dll")

add_executable(XinReplayBundle
    src/XinReplayBundle/main.cpp
    src/XinReplayBundle/src/xin_replay_gui.cpp
    src/XinReplayBundle/src/assets_function.cpp
    src/XinReplayBundle/src/imnodes.cpp
    src/XinReplayBundle/src/log_parse.cpp
    src/XinReplayBundle/src/graphic.cpp
    src/XinReplayBundle/src/resource.rc
)

add_executable(testImGUI src/XinReplayBundle/testImGUI.cc)

