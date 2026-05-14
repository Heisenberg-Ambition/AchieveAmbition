
#include "../inc/common.hpp"

#include <filesystem>

#include <limits.h>
#include <unistd.h>

std::string getExecutableFullPath()
{
    char buf[PATH_MAX] = {0};
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0)
    {
        return {};
    }
    return std::string(buf, len);
};

std::string getExecutableDir()
{
    std::string path = getExecutableFullPath();
    if (path.empty())
    {
        return {};
    }
    return std::filesystem::path(path).parent_path().string();
};

std::string getExecutableName()
{
    std::string path = getExecutableFullPath();
    if (path.empty())
    {
        return {};
    }
    return std::filesystem::path(path).filename().string();
};

std::string base64_encode(const std::string& input)
{
    std::string output;
    int val = 0;
    int valb = -6;

    for (unsigned char c : input)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            output.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6)
        output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);

    while (output.size() % 4)
        output.push_back('=');

    return output;
}

// * --- 混淆函数 ---
std::string obfuscate(const std::string& email)
{
    std::string temp = email;

    // 1️⃣ 反转
    std::reverse(temp.begin(), temp.end());

    // 2️⃣ ASCII +3
    for (char& c : temp)
    {
        c = static_cast<char>(c + 3);
    }

    // 3️⃣ Base64
    return base64_encode(temp);
};

// * --- AES-GCM 加密 ---
bool aes_gcm_encrypt(
    const std::vector<unsigned char>& plaintext,
    const unsigned char* key,
    const unsigned char* iv,
    std::vector<unsigned char>& ciphertext,
    unsigned char* tag)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return false;

    int len;
    ciphertext.resize(plaintext.size());

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr))
        return false;

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr))
        return false;

    if (!EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv))
        return false;

    if (!EVP_EncryptUpdate(ctx,
            ciphertext.data(),
            &len,
            plaintext.data(),
            plaintext.size()))
        return false;

    int ciphertext_len = len;

    if (!EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len))
        return false;

    ciphertext_len += len;

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag))
        return false;

    ciphertext.resize(ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);
    return true;
};

// * --- AES-GCM 解密 ---
bool aes_gcm_decrypt(
    const std::vector<unsigned char>& ciphertext,
    const unsigned char* key,
    const unsigned char* iv,
    const unsigned char* tag,
    std::vector<unsigned char>& plaintext)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return false;

    int len;
    plaintext.resize(ciphertext.size());

    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr))
        return false;

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr))
        return false;

    if (!EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, iv))
        return false;

    if (!EVP_DecryptUpdate(ctx,
            plaintext.data(),
            &len,
            ciphertext.data(),
            ciphertext.size()))
        return false;

    int plaintext_len = len;

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, (void*) tag))
        return false;

    // 关键：验证 tag
    int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);

    EVP_CIPHER_CTX_free(ctx);

    if (ret > 0)
    {
        plaintext_len += len;
        plaintext.resize(plaintext_len);
        return true; // 验证成功
    }

    return false; // 认证失败（数据被篡改或 key 错误）
};
