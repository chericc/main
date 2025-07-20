#include <random>

#include "gtest/gtest.h"

#include "xtea.h"
#include "base64.h"
#include "xlog.h"

namespace {

std::mt19937 rng(0);

// Function to generate random data buffer
std::vector<uint8_t> generateRandomData() {
    // Initialize random number generators
    std::uniform_int_distribution<size_t> len_dist(10, 1000); // Random length between 10 and 1000
    std::uniform_int_distribution<int> byte_dist(0, 255);     // Random byte values
    
    // Generate random length
    size_t length = len_dist(rng);
    std::vector<uint8_t> data(length);
    
    // Fill with random bytes
    for (auto& byte : data) {
        byte = static_cast<uint8_t>(byte_dist(rng));
    }
    
    return data;
}

// Function to generate random string
std::string generateRandomString() {
    // Initialize random number generators
    std::uniform_int_distribution<size_t> len_dist(5, 50);    // Random length between 5 and 50
    std::uniform_int_distribution<uint8_t> char_dist('a', 'z');  // Random lowercase letters
    
    // Generate random length
    size_t length = len_dist(rng);
    std::string str(length, '\0');
    
    // Fill with random characters
    for (auto& c : str) {
        c = char_dist(rng);
    }
    
    return str;
}

}


TEST(xtea, xtea_encipher_string)
{
    std::uniform_int_distribution<int> round_dis(1, 255);


    for (int i = 0; i < 1000; ++i) {
        int round = round_dis(rng);

        auto data = generateRandomData();
        auto password = generateRandomString();

        std::vector<char> base64_not_enc_buf;
        base64_not_enc_buf.resize(BASE64_ENCODE_OUT_SIZE(data.size()));
        base64_encode(data.data(), data.size(), base64_not_enc_buf.data());

        size_t base64_enc_buf_str_len = strlen(base64_not_enc_buf.data());

        std::vector<uint8_t> enc_buf;
        enc_buf.resize(base64_not_enc_buf.size() + 8);
        size_t enc_buf_size = enc_buf.size();
        int ret = xtea_encipher_string(base64_not_enc_buf.data(), base64_enc_buf_str_len, 
            enc_buf.data(), &enc_buf_size, password.data(), round, 1);
        EXPECT_EQ(ret, 0);
        if (ret < 0) {
            break;
        }

        std::vector<char> dec_buf;
        dec_buf.resize(enc_buf.size() + 8);
        size_t dec_buf_size = dec_buf.size();
        ret = xtea_encipher_string(enc_buf.data(), enc_buf_size, 
            dec_buf.data(), &dec_buf_size, password.data(), round, 0);
        EXPECT_EQ(ret, 0);
        if (ret < 0) {
            break;
        }

        // xlog_err("orig_base64: \n%s\ndec: \n%s\n\n", 
        //     base64_not_enc_buf.data(), dec_buf.data());

        ret = strcmp(dec_buf.data(), base64_not_enc_buf.data());
        EXPECT_EQ(ret, 0);

        std::vector<unsigned char> base64_dec_buf;
        base64_dec_buf.resize(dec_buf.size());

        ret = base64_decode(dec_buf.data(), strlen(dec_buf.data()), base64_dec_buf.data());
        base64_dec_buf.resize(ret);
        
        EXPECT_EQ(base64_dec_buf, data);
    }

}