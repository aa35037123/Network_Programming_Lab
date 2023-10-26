#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <curl/curl.h>

// File header structure
typedef struct {
    uint64_t magic;
    uint32_t datasize;
    uint16_t n_blocks;
    uint16_t zeros;
} __attribute__((packed)) binflag_header_t;

// Data blocks structure
typedef struct {
    uint32_t offset;
    uint16_t cksum;
    uint16_t length;
    uint8_t payload[0];
} __attribute__((packed)) block_t;

// Flag information structure
typedef struct {
   uint16_t length;
   uint32_t offset[0];
} __attribute__((packed)) flag_t;

void bxor(uint8_t* b1, const uint8_t* b2, size_t len) {
    for (size_t i = 0; i < len; i++) {
        b1[i] ^= b2[i];
    }
}

int main() {
    std::string challenge_url = "https://inp.zoolab.org/binflag/challenge?id=110611008";
    std::string demo_filename = "challenge.bin";

    // Download the challenge file using libcurl
    CURL* curl = curl_easy_init();
    if (curl) {
        FILE* output = fopen(demo_filename.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, challenge_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(output);
        if (res != CURLE_OK) {
            std::cerr << "Failed to download the challenge file" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Failed to initialize libcurl" << std::endl;
        return 1;
    }

    std::ifstream file(demo_filename, std::ios::binary);

    // Read and parse the file header
    binflag_header_t header;
    if (!file.read(reinterpret_cast<char*>(&header), sizeof(binflag_header_t))) {
        std::cerr << "Failed to read the file header" << std::endl;
        return 1;
    }
    header.magic = be64toh(header.magic);
    header.datasize = be32toh(header.datasize);
    header.n_blocks = be16toh(header.n_blocks);
    header.zeros = be16toh(header.zeros);

    // Read and parse data blocks
    std::vector<block_t> blocks;
    for (int i = 0; i < header.n_blocks; ++i) {
        block_t block;
        if (!file.read(reinterpret_cast<char*>(&block), sizeof(block_t))) {
            std::cerr << "Failed to read data block " << i << std::endl;
            return 1;
        }
        block.offset = be32toh(block.offset);
        block.cksum = be16toh(block.cksum);
        block.length = be16toh(block.length);
        blocks.push_back(block);
    }

    // Sort data blocks by offset
    std::sort(blocks.begin(), blocks.end(), [](const block_t& a, const block_t& b) {
        return a.offset < b.offset;
    });

    // Concatenate payload blocks
    std::vector<uint8_t> payload;
    for (const block_t& block : blocks) {
        payload.insert(payload.end(), block.payload, block.payload + block.length);
    }

    // Read flag information
    flag_t flag_info;
    if (!file.read(reinterpret_cast<char*>(&flag_info), sizeof(flag_t))) {
        std::cerr << "Failed to read flag information" << std::endl;
        return 1;
    }
    flag_info.length = be16toh(flag_info.length);

    // Read flag offsets
    std::vector<uint32_t> flag_offsets;
    for (int i = 0; i < flag_info.length; ++i) {
        uint32_t offset;
        if (!file.read(reinterpret_cast<char>(&offset), sizeof(uint32_t))) {
            std::cerr << "Failed to read flag offset " << i << std::endl;
            return 1;
        }
        flag_offsets.push_back(be32toh(offset));
    }

    // Extract flag bytes
    std::vector<uint8_t> flags;
    for (uint32_t offset : flag_offsets) {
        if (offset < payload.size() && offset + 2 <= payload.size()) {
            flags.push_back(payload[offset]);
            flags.push_back(payload[offset + 1]);
        }
    }

        // Compute and verify the flag
    uint8_t checksum = 0;

    for (size_t i = 0; i < payload.size(); ++i) {
        checksum ^= payload[i];
    }

    bool flag_verified = (checksum == 0);
    if (flag_verified) {
        // Convert flag bytes to a hex string
        std::string flag_hex;
        for (uint8_t flag : flags) {
            flag_hex += StringPrintf("%02x", flag);
        }

        // Prepare the final request URL
        std::string final_request = "https://inp.zoolab.org/binflag/verify?v=" + flag_hex;

        // Use libcurl to send the final request
        curl_easy_setopt(curl, CURLOPT_URL, final_request.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to send the final request" << std::endl;
            return 1;
        }
        curl_easy_cleanup(curl);
        fclose(output);

        // Read and print the response
        std::ifstream response_file(final_filename);
        std::string response((std::istreambuf_iterator<char>(response_file)),
                            std::istreambuf_iterator<char>());
        std::cout << "Response from server: " << response << std::endl;

        return 0;
    } else {
        std::cerr << "Flag verification failed" << std::endl;
        return 1;
    }
}
