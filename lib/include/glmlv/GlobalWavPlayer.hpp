#pragma once

#include <iostream>
#include "filesystem.hpp"

#if defined(__linux__)
#include <vector>
#include <sys/types.h>
#endif

namespace glmlv {

class GlobalWavPlayer {
    // Have one instance, only so the destructor is called at program termination.
    static GlobalWavPlayer s_Instance;

    GlobalWavPlayer();
    GlobalWavPlayer(const GlobalWavPlayer&) = delete;
    GlobalWavPlayer& operator=(const GlobalWavPlayer&) = delete;

#if defined(__linux__)
    std::vector<pid_t> m_AplayPids;
#elif defined(_WIN32)
	// Actually nothing here yet
#else
#error "Usupported platform"
#endif

public:
    // Immediately plays a WAV file, on top of any other that could be playing at the moment.
    static bool playWav(const fs::path& path);
    // Stops all playing WAVs at once.
    static void stopAll();

    ~GlobalWavPlayer() {
        stopAll();
    }
};

} // namespace glmlv
