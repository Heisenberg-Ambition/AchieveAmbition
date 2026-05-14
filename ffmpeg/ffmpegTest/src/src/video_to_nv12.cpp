
#include "../inc/video_to_nv12.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <libavcodec/avcodec.h>

#include "../inc/common.hpp"

void save_frame_as_nv12(AVFrame* src_frame, int frame_index)
{
    // * 像素格式缩放转换（decoder → jpeg encoder）
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    SwsContext* sws_ctx = sws_getContext(
        src_frame->width,
        src_frame->height,
        (AVPixelFormat) src_frame->format,
        src_frame->width,
        src_frame->height,
        AV_PIX_FMT_NV12,
        SWS_BILINEAR,
        nullptr, nullptr, nullptr);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "sws_getContext time: " << time_span.count() << "us" << std::endl;

    AVFrame* dst_frame = av_frame_alloc();
    dst_frame->format = AV_PIX_FMT_NV12;
    dst_frame->width = src_frame->width;
    dst_frame->height = src_frame->height;
    av_frame_get_buffer(dst_frame, 32);

    start = std::chrono::steady_clock::now();
    // 转换像素格式, 需要一个新的上下文和新的AVFrame目标帧
    int ret = sws_scale(
        sws_ctx,
        src_frame->data,
        src_frame->linesize,
        0,
        src_frame->height,
        dst_frame->data,
        dst_frame->linesize);
    end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "sws_scale time: " << time_span.count() << "us" << std::endl;

    auto exe_dir = getExecutableDir();
    auto exe_name = getExecutableName();
    std::ostringstream name;
    name << exe_dir << "/nv12/frame_"
         << std::setw(6) << std::setfill('0')
         << frame_index << ".nv12";

    std::cout << "save frame " << frame_index << " as " << name.str() << std::endl;

    start = std::chrono::steady_clock::now();
    std::ofstream f(name.str(), std::ios::binary);
    try
    {
        f.write(reinterpret_cast<const char*>(src_frame->data), src_frame->width * src_frame->height);
        std::cout << "write " << src_frame->width * src_frame->height << " bytes to " << name.str() << std::endl;
        f.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << "write to " << name.str() << " failed: " << e.what() << std::endl;
    }
    end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "write time: " << time_span.count() << "us" << std::endl;

    // * 清理, 释放缩放上下文
    start = std::chrono::steady_clock::now();
    sws_freeContext(sws_ctx);
    end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "sws_freeContext time: " << time_span.count() << "us" << std::endl;
}

// 一个 MJPEG 编码器, 可以编码YUVJ420P格式的帧
// 一个缩放上下文, 可以缩放YUVJ420P格式的原生帧

// AVCOL_RANGE_MPEG limited(16–235)
// AVCOL_RANGE_JPEG full(0–255)
