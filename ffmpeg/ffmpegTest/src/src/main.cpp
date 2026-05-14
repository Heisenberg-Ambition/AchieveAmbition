
// C++ 有一个特性叫 名字改编（name mangling）。
// 为了支持函数重载，它会把函数名偷偷改掉
// 用extern "C", 按 C 的 ABI 规则来
extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>

#include "../inc/common.hpp"
#include "../inc/video_to_jpeg.hpp"
#include "../inc/video_to_nv12.hpp"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "usage: video_parser <input.mp4>\n";
        return -1;
    }

    const char* input_path = argv[1];
    AVFormatContext* fmt_ctx = nullptr;

    // * input video path
    int ret = avformat_open_input(&fmt_ctx, input_path, nullptr, nullptr);
    if (ret < 0)
    {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cerr << "avformat open input failed: " << errbuf << "\n";
        return -1;
    }

    // * 读取流的信息, ≠ 读header
    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0)
    {
        std::cerr << "avformat_find_stream_info failed\n";
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // X 光机, 会输出视频的详细信息
    av_dump_format(fmt_ctx, 0, input_path, 0);

    // * 遍历流，找 AVMEDIA_TYPE_VIDEO(对于视频流应该是0)
    int video_stream_index = -1;
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        AVStream* stream = fmt_ctx->streams[i];
        std::cout << "stream codec_type: " << stream->codecpar->codec_type << "\n";
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index < 0)
    {
        std::cerr << "no video stream found\n";
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // * 找到解码器(视频/音频), AVCodec是真正的“解码器实现”
    AVStream* video_stream = fmt_ctx->streams[video_stream_index];
    const AVCodec* codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
    // * 解码器和编码器的函数是不一样的
    const AVCodec* encoder = avcodec_find_encoder(video_stream->codecpar->codec_id);

    // * 分配 codec context实例
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        std::cerr << "avcodec_alloc_context3 failed\n";
        avformat_close_input(&fmt_ctx);
        return -1;
    }
    if (!codec)
    {
        std::cerr << "unsupported codec\n";
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // * 把“声明参数”拷贝进去, 这是单向复制, 不是绑定。
    ret = avcodec_parameters_to_context(codec_ctx, video_stream->codecpar);
    if (ret < 0)
    {
        std::cerr << "avcodec_parameters_to_context failed\n";
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // * 打开解码器
    ret = avcodec_open2(codec_ctx, codec, nullptr);
    if (ret < 0)
    {
        std::cerr << "avcodec_open2 failed\n";
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    std::cout << "decoder name: " << codec_ctx->codec->name << "\n";
    std::cout << "decoder width: " << codec_ctx->width << "\n";
    std::cout << "decoder height: " << codec_ctx->height << "\n";
    std::cout << "codec name: " << codec->name << "\n";

    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    std::cout << "current video_stream_index before packet: " << video_stream_index << "\n";

    size_t packet_count = 0;
    size_t frame_count = 0;

    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    // * 循环：读mp4容器内的packet
    while (av_read_frame(fmt_ctx, pkt) >= 0)
    {
        packet_count = packet_count + 1;
        // * 过滤非视频流
        if (pkt->stream_index != video_stream_index)
        {
            // * 解除packet的引用, 但是不释放内存
            av_packet_unref(pkt);
            if (pkt == nullptr)
            {
                // ! 不会进入此处
                std::cout << "pkt is nullptr after unref\n";
                continue;
            }
            continue;
        }

        // * 发送packet到解码器
        int ret = avcodec_send_packet(codec_ctx, pkt);
        av_packet_unref(pkt);

        if (ret < 0)
        {
            std::cerr << "send_packet failed\n";
            break;
        }

        while (avcodec_receive_frame(codec_ctx, frame) == 0)
        {
            // ! 可以加上std::function<void()>/不加/<>
            timerUs<std::function<void()>>([&]() { save_frame_as_jpeg(frame, frame_count); return; });
            timerUs([&]() { save_frame_as_nv12(frame, frame_count); return; });

            frame_count = frame_count + 1;

            std::cout
                << "frame pts=" << frame->pts
                // << " jpge time=" << time_span0.count() << "us"
                // << " nv12 time=" << time_span1.count() << "us"
                << " packet dts=" << pkt->dts
                << " size=" << frame->width
                << "x" << frame->height
                << std::endl;
            av_frame_unref(frame);
        }
    }
    std::cout << "packet count: " << packet_count << "\n";
    std::cout << "frame count: " << frame_count << "\n";

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    std::cout << "duration: " << duration.count() << "s\n";

    avformat_close_input(&fmt_ctx);

    std::shared_ptr<int> shared_int {};
    std::cout << "&shared_int: " << &shared_int << " shared_int use_count: " << shared_int.use_count() << "\n";

    // ~ 分配一块控制块
    shared_int = std::make_shared<int>(100);
    std::cout << "&shared_int: " << &shared_int << " shared_int use_count: " << shared_int.use_count() << "\n";
    std::cout << "shared_int: " << shared_int << " get(): " << shared_int.get() << "\n";

    shared_int = std::make_shared<int>(100);
    std::cout << "&shared_int: " << &shared_int << " shared_int use_count: " << shared_int.use_count() << "\n";
    std::cout << "shared_int: " << shared_int << " get(): " << shared_int.get() << "\n";

    std::string email = "abc@example.com";
    std::string result = obfuscate(email);

    std::cout << "Original: " << email << std::endl;
    std::cout << "Obfuscated: " << result << std::endl;

    std::string message = "abc@example.com";
    std::vector<unsigned char> plaintext(message.begin(), message.end());

    unsigned char key[KEY_SIZE] = "shihao liu";
    unsigned char iv[IV_SIZE] = "hello";
    unsigned char tag[TAG_SIZE];

    // 生成随机 key 和 iv
    // RAND_bytes(key, sizeof(key));
    // RAND_bytes(iv, sizeof(iv));

    std::vector<unsigned char> ciphertext;

    if (!aes_gcm_encrypt(plaintext, key, iv, ciphertext, tag))
    {
        std::cerr << "Encryption failed\n";
        return 1;
    }
    else
    {
        std::cout << "ciphertext: " << std::endl;
        for (auto c : ciphertext)
        {
            std::cout << std::hex << (int) c << " ";
        }
        std::cout << std::endl;
    }

    std::vector<unsigned char> decrypted;

    if (!aes_gcm_decrypt(ciphertext, key, iv, tag, decrypted))
    {
        std::cerr << "Decryption failed or tag mismatch\n";
        return 1;
    }

    std::string result0(decrypted.begin(), decrypted.end());
    std::cout << "Decrypted: " << result0 << std::endl;

    return 0;
}

// ./video_parser /home/shihao/repo/Study/ffmpegTest/resource/a.mp4
// ./video_parser /home/shihao/repo/Study/ffmpegTest/resource/VisInsight_20260201101424.h264
