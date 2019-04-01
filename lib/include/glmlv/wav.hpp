#pragma once

#include "filesystem.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>

namespace glmlv {

struct PcmWav {
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t channel_count;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t frame_size; // block_align;
    uint16_t bits_per_sample;
    char subchunk2_id[4];
    /*
    uint32_t data_size;
    uint8_t data[];
    */
    std::vector<uint8_t> data;

    PcmWav(const fs::path& path) {
        std::ifstream file(path.string(), std::ios::binary | std::ios::ate);
        auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size); // Can't use uint8_t here. Ugh.
        if(!file.read(buffer.data(), size)) {
            auto msg = "Couldn't load WAV file \"" + path.string() + "\"!";
            std::cerr << msg << std::endl;
            throw std::runtime_error(msg);
        }
        load_raw(buffer.data(), buffer.size());
    }
    PcmWav(const void *raw_, size_t size) {
        load_raw(raw_, size);
    }
    void load_raw(const void *raw_, size_t size) {
        auto raw = (const uint8_t*) raw_;
        const size_t header_size = offsetof(PcmWav, data);
        assert(size >= header_size);
        memcpy(this, raw, header_size);
        auto data_size = *(const uint32_t*) &raw[header_size];
        data.resize(data_size);
        memcpy(data.data(), &raw[header_size + 4], size - (header_size + 4));
    }
    void saveToFile(const fs::path& path) const {
        std::ofstream file(path.string(), std::ios::binary);

        auto header_len = offsetof(PcmWav, data);
        file.write((const char*)this, header_len);
        uint32_t data_size = data.size();
        file.write((const char*)&data_size, 4);
        file.write((const char*)data.data(), data.size());
    }
    bool is_valid() const;
    void log_summary(std::ostream&) const;
};

struct PcmDuration {
    uint32_t hours=0, minutes=0, seconds=0, milliseconds=0;
    PcmDuration(uint64_t frame_count, uint32_t sample_rate);
    friend std::ostream& operator<<(std::ostream& os, const PcmDuration& d);
};

} // namespace glmlv
