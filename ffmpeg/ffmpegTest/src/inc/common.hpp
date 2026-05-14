
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <openssl/evp.h>
#include <openssl/rand.h>

static constexpr int KEY_SIZE = 32; // 256 bit
constexpr int IV_SIZE = 12;         // GCM 推荐 12 字节
constexpr int TAG_SIZE = 16;

std::string getExecutableDir();

std::string getExecutableName();

std::string getExecutableFullPath();

// * 用 steady_clock, 它是单调递增的，不受系统时间调整影响.做性能测量必须用它，而不是 system_clock
// * 测量函数 f 执行时间，返回 f 的返回值（如果有）
// * td(), 不需要td<>(), F在参数中, 自动推导返回值类型
// * decltype(auto), 用 decltype 规则来推导返回值类型
template <typename F>
decltype(auto) timerUs(F&& f)
{
    using R = std::invoke_result_t<F>;

    auto start = std::chrono::steady_clock::now();

    if constexpr (std::is_void_v<R>)
    {
        std::forward<F>(f)();

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Func Time: " << duration.count() << " us\n";
    }
    else
    {
        R result = std::forward<F>(f)();

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Func Time: " << duration.count() << " us\n";

        return result;
    }
};

// --- Base64 编码 ---
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64Enncode(const std::string& input);

std::string obfuscate(const std::string& email);

bool aes_gcm_encrypt(
    const std::vector<unsigned char>& plaintext,
    const unsigned char* key,
    const unsigned char* iv,
    std::vector<unsigned char>& ciphertext,
    unsigned char* tag);

bool aes_gcm_decrypt(
    const std::vector<unsigned char>& ciphertext,
    const unsigned char* key,
    const unsigned char* iv,
    const unsigned char* tag,
    std::vector<unsigned char>& plaintext);