//
// Created by RGAA on 2024/2/28.
//

#ifndef TC_CLIENT_PC_AUDIO_PLAYER_H
#define TC_CLIENT_PC_AUDIO_PLAYER_H

#include <QMainWindow>
#include <QFile>
#include <QDebug>
#include <SDL2/SDL.h>
#include <queue>
#include <mutex>

namespace tc
{

    class Data;

    class AudioPlayer {
    public:
        AudioPlayer();
        ~AudioPlayer();
        int Init(int freq/*samples eg: 48k*/, int channels);
        void Write(const char* data, int size);
        void Write(const std::shared_ptr<Data>& data);
        void AudioCallback(void* userdata, Uint8* stream, int len);

    private:
        std::queue<Uint8> pcm_queue_;
        std::mutex queue_mutex_;
    };
}

#endif //TC_CLIENT_PC_AUDIO_PLAYER_H
