//
// Created by RGAA on 2023-12-25.
//

#include "message_maker.h"

#include "tc_common_new/data.h"
#include "tc_message.pb.h"
#include "statistics.h"

namespace tc
{

    std::string NetMessageMaker::MakeHelloMsg() {
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kHello);
        auto hello = new Hello();
        msg->set_allocated_hello(hello);
        return msg->SerializeAsString();
    }

    std::string NetMessageMaker::MakeAckMsg() {
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kAck);
        auto ack = new Ack();
        msg->set_allocated_ack(ack);
        return msg->SerializeAsString();
    }

    std::string NetMessageMaker::MakeHeartBeatMsg() {
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kHeartBeat);
        auto heart_beat = new HeartBeat();
        msg->set_allocated_heart_beat(heart_beat);
        return msg->SerializeAsString();
    }

    std::string NetMessageMaker::MakeVideoFrameMsg(const tc::VideoType& vt,
                                                   const std::shared_ptr<Data>& data,
                                                   uint64_t frame_index,
                                                   int frame_width,
                                                   int frame_height,
                                                   bool key) {
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kVideoFrame);
        auto frame = new VideoFrame();
        frame->set_type(vt);
        frame->set_data(data->AsString());
        frame->set_frame_index(frame_index);
        frame->set_key(key);
        frame->set_frame_width(frame_width);
        frame->set_frame_height(frame_height);
        msg->set_allocated_video_frame(frame);
        return msg->SerializeAsString();
    }

    std::string NetMessageMaker::MakeAudioFrameMsg(const std::shared_ptr<Data>& data,
                                                   int samples, int channels, int bits, int frame_size) {
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kAudioFrame);
        auto frame = new AudioFrame();
        frame->set_data(data->CStr(), data->Size());
        frame->set_samples(samples);
        frame->set_channels(channels);
        frame->set_bits(bits);
        frame->set_frame_size(frame_size);
        msg->set_allocated_audio_frame(frame);
        return msg->SerializeAsString();
    }

    std::string NetMessageMaker::MakeServerAudioSpectrumMsg() {
        auto st = Statistics::Instance();
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kServerAudioSpectrum);
        auto sas = msg->mutable_server_audio_spectrum();
        sas->set_samples(st->audio_samples_);
        sas->set_bits(st->audio_bits_);
        sas->set_channels(st->audio_channels_);
        sas->mutable_left_spectrum()->Add(st->left_spectrum_.begin(), st->left_spectrum_.end());
        sas->mutable_right_spectrum()->Add(st->right_spectrum_.begin(), st->right_spectrum_.end());
        return msg->SerializeAsString();
    }

}