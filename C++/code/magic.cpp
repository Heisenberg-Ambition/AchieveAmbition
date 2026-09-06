
/*
 * @Author: Shihao Liu
 * @Date: 2026-08-30 00:41:33
 * @LastEditors: Shihao Liu / orianna_a24@foxmail.com
 * @LastEditTime: 2026-08-30 00:41:34
 * @FilePath: /AchieveAmbition/magic.cpp
 * @Code: b3JpYW5uYV9hMjRAZm94bWFpbC5jb20=
 */

// -pedantic（严格遵循标准）
// -Wall -Wextra -Wpedantic

// FFmpeg 常见参数（-i 输入、-c:v/libx264 视频编解码、-c:a/aac 音频编码、-b:v/-crf 码率控制、-s 分辨率、-r 帧率、-vf/-af 滤镜等），
// 展示 格式转换、视频裁剪分割、合并拼接、截取缩略图、录制屏幕/摄像头 等核心操作。文章还详细讲解 版本兼容问题、编解码器授权、路径与权限、命令行拼写、输出质量与体积平衡

#include <chrono>
#include <iostream>
#include <type_traits>

// * 时间魔法
// * 用 steady_clock, 它是单调递增的，不受系统时间调整影响.做性能测量必须用它，而不是 system_clock
template <typename F>
auto time_magic(F&& f)
{
    using R = std::invoke_result_t<F>;

    auto start = std::chrono::steady_clock::now();

    if constexpr (std::is_void_v<R>)
    {
        std::forward<F>(f)();

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Time: " << duration.count() << " us\n";
    }
    else
    {
        R result = std::forward<F>(f)();

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Time: " << duration.count() << " us\n";

        return result;
    }
};

int main()
{
    time_magic([]() {
        for (int i = 0; i < 1000000; ++i)
        {
            // 模拟一些工作
            int x = i * i;
        }
    });

    int result = time_magic([]() -> int {
        int sum = 0;
        for (int i = 0; i < 1000000; ++i)
        {
            sum += i;
        }
        return sum;
    });

    std::cout << "Func Result: " << result << "\n";

    return 0;
};

// * g++ -std=c++17 -Wall -Wextra -pedantic magic.cpp -o magic