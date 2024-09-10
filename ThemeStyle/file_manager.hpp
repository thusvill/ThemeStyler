#include <cstdlib> // For getenv()
#include <cstring> // For strcat()
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

bool ensureDirectoryExists(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        if (mkdir(path.c_str(), 0777) != 0)
        {
            std::cerr << "Error: Failed to create directory " << path << std::endl;
            return false;
        }
    }
    else if (!(info.st_mode & S_IFDIR))
    {
        std::cerr << "Error: " << path << " is not a directory" << std::endl;
        return false;
    }
    return true;
}

void writeFile(const std::string &filename, const std::string &content)
{
    // Get the home directory of the current user
    const char *homeDir = getenv("HOME");
    if (homeDir == nullptr)
    {
        std::cerr << "Error: Unable to retrieve home directory" << std::endl;
        return;
    }

    // Concatenate the home directory with the rest of the path
    std::string fullPath = std::string(homeDir) + "/" + filename;

    std::string directory = fullPath.substr(0, fullPath.find_last_of("/\\"));
    if (!ensureDirectoryExists(directory))
    {
        std::cerr << "Error: Unable to create directory for file " << filename << std::endl;
        return;
    }

    std::FILE *file = std::fopen(fullPath.c_str(), "w");
    if (file != nullptr)
    {
        std::fwrite(content.c_str(), sizeof(char), content.size(), file);
        std::fclose(file);
    }
    else
    {
        std::cerr << "Error: Unable to open file " << fullPath << " for writing" << std::endl;
    }
}

std::string readFile(const std::string &filename)
{
    std::string content;
    std::FILE *file = std::fopen(filename.c_str(), "r");
    if (file != nullptr)
    {
        std::fseek(file, 0, SEEK_END);
        long fileSize = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);
        char *buffer = new char[fileSize + 1];
        std::fread(buffer, sizeof(char), fileSize, file);
        buffer[fileSize] = '\0';
        std::fclose(file);
        content = std::string(buffer);
        delete[] buffer;
    }
    else
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
    }
    return content;
}

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

namespace std{
template <typename T>
constexpr const T &clamp(const T &v, const T &lo, const T &hi)
{
    return (v < lo) ? lo : (hi < v) ? hi
                                    : v;
}}

std::string stringToBase64(const std::string &input)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return result;
}

std::string base64ToString(const std::string &input)
{
    BIO *bio, *b64;
    char *buffer = reinterpret_cast<char *>(malloc(input.size()));
    memset(buffer, 0, input.size());

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(input.c_str(), input.size());
    bio = BIO_push(b64, bio);

    BIO_read(bio, buffer, input.size());
    BIO_free_all(bio);

    std::string result(buffer);
    free(buffer);
    return result;
}
void CopyStringToCharArray(const std::string &str, char *buffer, size_t bufferSize)
{
    // Copy the string content to the buffer
    std::strncpy(buffer, str.c_str(), bufferSize - 1);
    // Ensure the buffer is null-terminated
    buffer[bufferSize - 1] = '\0';
}