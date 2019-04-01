#include <glmlv/GlobalWavPlayer.hpp>
#include <iostream>
#include <string.h>

#if defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace glmlv {

GlobalWavPlayer::GlobalWavPlayer() {}
GlobalWavPlayer GlobalWavPlayer::s_Instance;

#if defined(__linux__)

bool GlobalWavPlayer::playWav(const fs::path& path) {
    pid_t pid = fork();
    switch(pid) {
    case 0:
        if(-1 == execlp("aplay", "aplay", path.string().c_str(), nullptr)) {
            perror("GlobalWavPlayer: execlp");
            exit(EXIT_FAILURE);
        }
        break;
    case -1:
        perror("GlobalWavPlayer: fork");
        return false;
    }
    std::cout << "GlobalWavPlayer: Spawned child aplay process " << pid << " (playing " << path << ")" << std::endl;
    s_Instance.m_AplayPids.push_back(pid);
    return true;
}
void GlobalWavPlayer::stopAll() {
    for(pid_t pid: s_Instance.m_AplayPids) {
        std::cout << "GlobalWavPlayer: Sending SIGKILL to child aplay process " << pid << std::endl;
        kill(pid, SIGKILL);
    }
    s_Instance.m_AplayPids.clear();
}

#elif defined(_WIN32)

bool GlobalWavPlayer::playWav(const fs::path& path) {
	const auto flags = SND_FILENAME | SND_ASYNC | SND_NODEFAULT | SND_NOSTOP;
	const BOOL success = PlaySoundW(path.wstring().c_str(), nullptr, flags);
    if(!success) {
		auto err = GetLastError();
		wchar_t buf[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf, sizeof(buf), NULL);
        std::cerr << "GlobalWavPlayer: Could not play " << path << ": PlaySoundW error " << err << ": " << buf << std::endl;
    } else {
        std::cout << "GlobalWavPlayer: Playing " << path << "" << std::endl;
    }
    return success;
}
void GlobalWavPlayer::stopAll() {
    PlaySoundW(NULL, NULL, 0);
}
#endif

} // namespace glmlv
