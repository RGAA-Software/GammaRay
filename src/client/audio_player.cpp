//
// Created by RGAA on 2024/2/28.
//

#include "audio_player.h"

#include "audio_player.h"

#include "tc_common_new/data.h"
#include "tc_common_new/log.h"

namespace tc
{
    AudioPlayer::AudioPlayer() {

    }

    AudioPlayer::~AudioPlayer() {

    }

    int AudioPlayer::Init(int freq, int channels) {
        // 初始化 SDL
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            LOGE("SDL initialization failed: {}", SDL_GetError());
            return -1;
        }

        // 设置音频参数
        SDL_AudioSpec spec;
        spec.freq = freq;
        spec.format = AUDIO_S16SYS;
        spec.channels = channels;
        spec.samples = 1024;
        spec.userdata = this;
        spec.callback = [](void* userdata, Uint8* stream, int len) {
            auto player = (AudioPlayer*)userdata;
            player->AudioCallback(userdata, stream, len);
        };

        // 打开音频设备
        if (SDL_OpenAudio(&spec, nullptr) < 0) {
            LOGE("Failed to open audio: {}", SDL_GetError());
            SDL_Quit();
            return 1;
        }

        // 开始播放音频
        SDL_PauseAudio(0);

        return 0;
    }

    void AudioPlayer::AudioCallback(void* userdata, Uint8* stream, int len) {
        if (pcm_queue_.empty()) {
            SDL_memset(stream, 0, len);
        } else {
            queue_mutex_.lock();
            for (int i = 0; i < len; i++) {
                if (!pcm_queue_.empty()) {
                    stream[i] = pcm_queue_.front();
                    pcm_queue_.pop();
                } else {
                    break;
                }
            }
            queue_mutex_.unlock();
        }
    }

    void AudioPlayer::Write(const char* data, int size) {
        // 将接收到的PCM数据存储到队列中
        queue_mutex_.lock();
        for (int i = 0; i < size; i++) {
            pcm_queue_.push(data[i]);
        }
        queue_mutex_.unlock();
    }

    void AudioPlayer::Write(const std::shared_ptr<Data>& data) {
        this->Write(data->DataAddr(), data->Size());
    }
}