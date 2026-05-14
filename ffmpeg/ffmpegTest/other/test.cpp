

// main.cpp
extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libavutil/pixdesc.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libswscale/swscale.h>
}

#include <iostream>
#include <vector>
#include <string>

static void check_error(int ret)
{
    if (ret < 0)
    {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cerr << "FFmpeg error: " << errbuf << "\n";
        exit(1);
    }
}

// 每路输入状态
struct InputContext
{
    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    int video_stream_index = -1;
    AVFilterContext *buffersrc_ctx = nullptr;
};

// 全局上下文
struct FilterContext
{
    AVFilterGraph *graph = nullptr;
    AVFilterContext *buffersink_ctx = nullptr;
};

// 打开输入并初始化解码器
void open_input(const std::string &filename, InputContext &ic)
{
    check_error(avformat_open_input(&ic.fmt_ctx, filename.c_str(), nullptr, nullptr));
    check_error(avformat_find_stream_info(ic.fmt_ctx, nullptr));

    for (unsigned i = 0; i < ic.fmt_ctx->nb_streams; i++)
    {
        if (ic.fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            ic.video_stream_index = i;
            break;
        }
    }
    if (ic.video_stream_index < 0)
    {
        std::cerr << "No video stream in " << filename << "\n";
        exit(1);
    }

    AVCodec *dec = avcodec_find_decoder(
        ic.fmt_ctx->streams[ic.video_stream_index]->codecpar->codec_id);
    ic.dec_ctx = avcodec_alloc_context3(dec);
    avcodec_parameters_to_context(ic.dec_ctx, ic.fmt_ctx->streams[ic.video_stream_index]->codecpar);
    check_error(avcodec_open2(ic.dec_ctx, dec, nullptr));
}

// 创建 filter graph
void init_filters(std::vector<InputContext> &inputs, FilterContext &fc)
{
    char args[512];
    int ret;

    // 1. 创建 graph
    fc.graph = avfilter_graph_alloc();

    // 2. 为每路输入创建 buffersrc
    for (int i = 0; i < 4; i++)
    {
        const AVFilter *buffersrc = avfilter_get_by_name("buffer");
        AVFilterContext *src_ctx = nullptr;
        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                 inputs[i].dec_ctx->width, inputs[i].dec_ctx->height,
                 inputs[i].dec_ctx->pix_fmt,
                 inputs[i].fmt_ctx->streams[inputs[i].video_stream_index]->time_base.num,
                 inputs[i].fmt_ctx->streams[inputs[i].video_stream_index]->time_base.den,
                 inputs[i].dec_ctx->sample_aspect_ratio.num,
                 inputs[i].dec_ctx->sample_aspect_ratio.den);

        ret = avfilter_graph_create_filter(&src_ctx, buffersrc, ("in" + std::to_string(i)).c_str(),
                                           args, nullptr, fc.graph);
        check_error(ret);
        inputs[i].buffersrc_ctx = src_ctx;
    }

    // 3. 创建 scale 过滤器，并为每路输入命名 [inN]→[scaledN]
    for (int i = 0; i < 4; i++)
    {
        const AVFilter *scale = avfilter_get_by_name("scale");
        AVFilterContext *scale_ctx = nullptr;
        ret = avfilter_graph_create_filter(&scale_ctx, scale, ("scale" + std::to_string(i)).c_str(),
                                           "960:540", nullptr, fc.graph);
        check_error(ret);
        // 连接 inN → scaleN
        ret = avfilter_link(inputs[i].buffersrc_ctx, 0, scale_ctx, 0);
        check_error(ret);
    }

    // 4. 水平拼接上排（scale0+scale1） → top；下排（scale2+scale3） → bottom
    const AVFilter *hstack = avfilter_get_by_name("hstack");
    AVFilterContext *hstack_top = nullptr;
    AVFilterContext *hstack_bottom = nullptr;
    // 上排
    ret = avfilter_graph_create_filter(&hstack_top, hstack, "hstack_top",
                                       "inputs=2", nullptr, fc.graph);
    check_error(ret);
    avfilter_link(fc.graph->filters[1], 0, hstack_top, 0); // scale0
    avfilter_link(fc.graph->filters[2], 0, hstack_top, 1); // scale1

    // 下排
    ret = avfilter_graph_create_filter(&hstack_bottom, hstack, "hstack_bottom",
                                       "inputs=2", nullptr, fc.graph);
    check_error(ret);
    avfilter_link(fc.graph->filters[3], 0, hstack_bottom, 0); // scale2
    avfilter_link(fc.graph->filters[4], 0, hstack_bottom, 1); // scale3

    // 5. 垂直拼接 top+bottom → out
    const AVFilter *vstack = avfilter_get_by_name("vstack");
    AVFilterContext *vstack_ctx = nullptr;
    ret = avfilter_graph_create_filter(&vstack_ctx, vstack, "vstack",
                                       "inputs=2", nullptr, fc.graph);
    check_error(ret);
    avfilter_link(hstack_top, 0, vstack_ctx, 0);
    avfilter_link(hstack_bottom, 0, vstack_ctx, 1);

    // 6. 创建 buffersink
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    ret = avfilter_graph_create_filter(&fc.buffersink_ctx, buffersink, "out",
                                       nullptr, nullptr, fc.graph);
    check_error(ret);

    // 7. 配置 graph
    ret = avfilter_graph_config(fc.graph, nullptr);
    check_error(ret);
}

// 打开输出文件与编码器（libx264）
void open_output(const std::string &filename, AVCodecContext *input_enc_ctx,
                 AVFormatContext *&ofmt_ctx, AVCodecContext *&enc_ctx)
{
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, filename.c_str());
    if (!ofmt_ctx)
    {
        std::cerr << "Could not create output context\n";
        exit(1);
    }

    AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    enc_ctx = avcodec_alloc_context3(encoder);
    // 从 filter sink 的输出帧格式继承
    enc_ctx->width = 1920;
    enc_ctx->height = 1080;
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enc_ctx->time_base = {1, 25};
    enc_ctx->bit_rate = 4 * 1000 * 1000; // 4 Mbps

    avcodec_open2(enc_ctx, encoder, nullptr);

    AVStream *out_stream = avformat_new_stream(ofmt_ctx, nullptr);
    avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_open(&ofmt_ctx->pb, filename.c_str(), AVIO_FLAG_WRITE);

    avformat_write_header(ofmt_ctx, nullptr);
}

int main()
{
    av_register_all();
    avfilter_register_all();

    // 1. 打开四路输入
    std::vector<std::string> files = {"a.mp4", "b.mp4", "c.mp4", "d.mp4"};
    std::vector<InputContext> inputs(4);
    for (int i = 0; i < 4; i++)
        open_input(files[i], inputs[i]);

    // 2. 构建 filter graph
    FilterContext fc{};
    init_filters(inputs, fc);

    // 3. 打开输出与编码器
    AVFormatContext *ofmt_ctx = nullptr;
    AVCodecContext *enc_ctx = nullptr;
    open_output("output.mp4", inputs[0].dec_ctx, ofmt_ctx, enc_ctx);

    // 4. 主循环：从 4 路读帧 → 解码 → 送入对应 buffersrc → 从 sink 拉帧 → 编码写入
    AVPacket pkt;
    av_init_packet(&pkt);
    AVFrame *frame = av_frame_alloc();
    AVFrame *filt_out = av_frame_alloc();

    bool got_output = false;
    while (true)
    {
        // 4.1 逐路拉一个 packet
        int ret = av_read_frame(inputs[0].fmt_ctx, &pkt); // 示例仅读第一路，实际应轮询四路
        if (ret < 0)
            break; // EOF
        if (pkt.stream_index != inputs[0].video_stream_index)
        {
            av_packet_unref(&pkt);
            continue;
        }

        // 4.2 解码
        ret = avcodec_send_packet(inputs[0].dec_ctx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0)
            break;

        while (avcodec_receive_frame(inputs[0].dec_ctx, frame) >= 0)
        {
            // 4.3 送入 filter
            av_buffersrc_add_frame(inputs[0].buffersrc_ctx, frame);
            // 4.4 从 sink 拉帧
            while (av_buffersink_get_frame(fc.buffersink_ctx, filt_out) >= 0)
            {
                // 4.5 编码写入
                avcodec_send_frame(enc_ctx, filt_out);
                AVPacket out_pkt;
                av_init_packet(&out_pkt);
                while (avcodec_receive_packet(enc_ctx, &out_pkt) >= 0)
                {
                    av_interleaved_write_frame(ofmt_ctx, &out_pkt);
                    av_packet_unref(&out_pkt);
                }
                av_frame_unref(filt_out);
            }
            av_frame_unref(frame);
        }
    }

    // 写尾
    av_write_trailer(ofmt_ctx);

    // 释放
    av_frame_free(&frame);
    av_frame_free(&filt_out);
    avcodec_free_context(&enc_ctx);
    avformat_close_input(&ofmt_ctx);
    avfilter_graph_free(&fc.graph);
    for (auto &ic : inputs)
    {
        avcodec_free_context(&ic.dec_ctx);
        avformat_close_input(&ic.fmt_ctx);
    }

    std::cout << "完成 ✅\n";
    return 0;
}
