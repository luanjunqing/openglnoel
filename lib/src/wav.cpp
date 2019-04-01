#include <glmlv/wav.hpp>
#include <cstring>

namespace glmlv {

// XXX What about MSVC ??
#ifdef __GNUG__
static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "wav.cpp doesn't support big endian!");
#endif

PcmDuration::PcmDuration(uint64_t frame_count, uint32_t sample_rate) {
    float seconds_total = frame_count / (float)sample_rate;
    hours = seconds_total / 3600;
    minutes = seconds_total / 60 - hours*60;
    seconds = seconds_total - minutes*60 - hours*3600;
    float ms = 1000.f*(seconds_total - seconds*60 - minutes*3600 - hours*3600*60);
    if(ms >= 0)
        milliseconds = ms;
}

bool PcmWav::is_valid() const {
    if(strncmp(chunk_id, "RIFF", 4))
        return false;
    if(strncmp(format, "WAVE", 4))
        return false;
    if(strncmp(subchunk1_id, "fmt ", 4))
        return false;
    if(audio_format != 1) // PCM = 1
        return false;
    if(strncmp(subchunk2_id, "data", 4))
        return false;
    return true;
}

std::ostream& operator<<(std::ostream& os, const PcmDuration& d) {
    os << d.hours << ":" << d.minutes << ":" << d.seconds << ":" << d.milliseconds;
    return os;
}

void PcmWav::log_summary(std::ostream& os) const {
    uint32_t frame_count = data.size() / frame_size;
    PcmDuration d(frame_count, sample_rate);

    os << bits_per_sample << "-bit " << channel_count << " channel(s) @"
       << sample_rate << "Hz, " << frame_count << " frames (" << d << ")";
}

} // namespace glmlv
