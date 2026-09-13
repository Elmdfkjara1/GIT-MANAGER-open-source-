#include "Env/Data.h"
#include <zlib.h>
#include <openssl/sha.h>


void git::getRawHash(std::string_view content, unsigned char* outBuffer) {
    SHA1(reinterpret_cast<const unsigned char*>(content.data()), content.size(), outBuffer);
}

std::string git::calculateHash(std::string_view content){
    unsigned char hash[SHA_DIGEST_LENGTH];
    getRawHash(content, hash);

    static constexpr char hex_chars[] = "0123456789abcdef";
    std::string hex_str;
    hex_str.resize(40);

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        hex_str[2 * i]     = hex_chars[(hash[i] >> 4) & 0x0F];
        hex_str[2 * i + 1] = hex_chars[hash[i] & 0x0F];
    }

    return hex_str;
}

std::string git::compress(std::string_view content){
	uLongf destLen = compressBound(content.size());
	std::string compressed;
	compressed.resize(destLen);

	int res = compress2(
        reinterpret_cast<Bytef*>(compressed.data()), &destLen,
        reinterpret_cast<const Bytef*>(content.data()), content.size(),
        Z_DEFAULT_COMPRESSION
    );

    if (res != Z_OK) return " ";

    compressed.resize(destLen);
    return compressed;
}