

#include "../inc/video_to_jpeg.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <libavcodec/avcodec.h>

#include "../inc/common.hpp"

void save_frame_as_jpeg(AVFrame* src_frame, int frame_index)
{
    // * 找 MJPEG 编码器
    // * 编码器类型为: AV_CODEC_ID_MJPEG
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    const AVCodec* jpeg_codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "avcodec_find_encoder: " << time_span.count() << "us" << std::endl;
    if (!jpeg_codec)
    {
        fprintf(stderr, "MJPEG encoder not found\n");
        return;
    }

    // * 解码器上下文编码格式为: AV_PIX_FMT_YUVJ420P
    start = std::chrono::steady_clock::now();
    AVCodecContext* jpeg_ctx = avcodec_alloc_context3(jpeg_codec);
    end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "avcodec_alloc_context3: " << time_span.count() << "us" << std::endl;

    jpeg_ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    jpeg_ctx->height = src_frame->height;
    jpeg_ctx->width = src_frame->width;
    jpeg_ctx->time_base = {1, 25};
    jpeg_ctx->color_range = AVCOL_RANGE_MPEG;

    start = std::chrono::steady_clock::now();
    int ret = avcodec_open2(jpeg_ctx, jpeg_codec, nullptr);
    end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "avcodec_open2: " << time_span.count() << "us" << std::endl;
    if (ret < 0)
    {
        fprintf(stderr, "Could not open MJPEG encoder\n");
        return;
    }

    // // * 像素格式缩放转换（decoder → jpeg encoder）
    // start = std::chrono::steady_clock::now();
    // SwsContext* sws_ctx = sws_getContext(
    //     src_frame->width,
    //     src_frame->height,
    //     (AVPixelFormat) src_frame->format,
    //     src_frame->width,
    //     src_frame->height,
    //     jpeg_ctx->pix_fmt,
    //     SWS_BILINEAR,
    //     nullptr, nullptr, nullptr);
    // end = std::chrono::steady_clock::now();
    // time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "sws_getContext time: " << time_span.count() << "us" << std::endl;

    // AVFrame* jpeg_frame = av_frame_alloc();
    // jpeg_frame->format = jpeg_ctx->pix_fmt;
    // jpeg_frame->width = jpeg_ctx->width;
    // jpeg_frame->height = jpeg_ctx->height;
    // av_frame_get_buffer(jpeg_frame, 32);

    // start = std::chrono::steady_clock::now();
    // // * 可选: 转换像素格式, 需要一个新的上下文和新的AVFrame目标帧
    // sws_scale(
    //     sws_ctx,
    //     src_frame->data,
    //     src_frame->linesize,
    //     0,
    //     src_frame->height,
    //     jpeg_frame->data,
    //     jpeg_frame->linesize);
    // end = std::chrono::steady_clock::now();
    // time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // std::cout << "sws_scale time: " << time_span.count() << "us" << std::endl;

    // * 编码
    AVPacket* pkt = av_packet_alloc();
    pkt->data = nullptr;
    pkt->size = 0;

    start = std::chrono::steady_clock::now();
    // *把缩放后的帧发送到编码器
    avcodec_send_frame(jpeg_ctx, src_frame);
    end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "avcodec_send_frame time: " << time_span.count() << "us" << std::endl;

    start = std::chrono::steady_clock ::now();
    ret = avcodec_receive_packet(jpeg_ctx, pkt);
    end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "avcodec_receive_packet time: " << time_span.count() << "us" << std::endl;

    if (ret == 0)
    {
        auto exe_dir = getExecutableDir();
        auto exe_name = getExecutableName();
        std::ostringstream name;
        name << exe_dir << "/jpeg/frame_"
             << std::setw(6) << std::setfill('0')
             << frame_index << ".jpg";

        std::cout << "save frame " << frame_index << " as " << name.str() << std::endl;

        start = std::chrono::steady_clock::now();
        std::ofstream f(name.str(), std::ios::binary);
        try
        {
            f.write(reinterpret_cast<const char*>(pkt->data), pkt->size);
            std::cout << "write " << pkt->size << " bytes to " << name.str() << std::endl;
            f.close();
        }
        catch (const std::exception& e)
        {
            std::cerr << "write to " << name.str() << " failed: " << e.what() << std::endl;
        }
        end = std::chrono::steady_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "write time: " << time_span.count() << "us" << std::endl;

        av_packet_unref(pkt);
        delete pkt;
    }

    // * 清理, 释放缩放上下文
    start = std::chrono::steady_clock::now();
    // sws_freeContext(sws_ctx);
    // av_frame_free(&jpeg_frame);
    avcodec_free_context(&jpeg_ctx);
    end = std::chrono::steady_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "sws_freeContext time: " << time_span.count() << "us" << std::endl;
}

// 一个 MJPEG 编码器, 可以编码YUVJ420P格式的帧
// 一个缩放上下文, 可以缩放YUVJ420P格式的原生帧

// AVCOL_RANGE_MPEG limited(16–235)
// AVCOL_RANGE_JPEG full(0–255)

// * 两个上下文, 一个用于编码, 一个用于缩放
// * 可以直接用sws_scale()来传入AVFrame来存储缩放后的帧
// * 再将缩放后的帧发送到JPGE编码器, 编码为JPEG格式, 通过avcodec_receive_packet()获取编码后的packet
// * 最后将编码后的packet写入.jpeg文件
