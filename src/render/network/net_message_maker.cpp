//
// Created by RGAA on 2023-12-25.
//

#include "net_message_maker.h"

#include "tc_common_new/data.h"
#include "tc_message.pb.h"
//#include "rd_statistics.h"
#include "tc_common_new/key_helper.h"

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
        msg->set_allocated_heartbeat(heart_beat);
        return msg->SerializeAsString();
    }

    std::string NetMessageMaker::MakeOnHeartBeatMsg(uint64_t index) {
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kOnHeartBeat);
        auto hb = msg->mutable_on_heartbeat();
        hb->set_index(index);
        hb->set_caps_lock_pressed(KeyHelper::IsCapsLockPressed());
        hb->set_caps_lock_state(KeyHelper::GetCapsLockState());
        hb->set_num_lock_pressed(KeyHelper::IsNumLockPressed());
        hb->set_num_lock_state(KeyHelper::GetNumLockState());
        hb->set_alt_pressed(KeyHelper::IsAltPressed());
        hb->set_control_pressed(KeyHelper::IsControlPressed());
        hb->set_win_pressed(KeyHelper::IsWinPressed());
        hb->set_shift_pressed(KeyHelper::IsShiftPressed());
        return msg->SerializeAsString();
    }

    std::string NetMessageMaker::MakeVideoFrameMsg(const tc::VideoType& vt, const std::shared_ptr<Data>& data,
                                                   uint64_t frame_index, int frame_width, int frame_height, bool key,
                                                   const std::string& display_name, int mon_left,
                                                   int mon_top, int mon_right, int mon_bottom) {
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kVideoFrame);
        auto frame = new VideoFrame();
        frame->set_type(vt);
        frame->set_data(data->AsString());
        frame->set_frame_index(frame_index);
        frame->set_key(key);
        frame->set_frame_width(frame_width);
        frame->set_frame_height(frame_height);
        frame->set_mon_name(display_name);
        frame->set_mon_left(mon_left);
        frame->set_mon_top(mon_top);
        frame->set_mon_right(mon_right);
        frame->set_mon_bottom(mon_bottom);
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

//    std::string NetMessageMaker::MakeServerAudioSpectrumMsg() {
//        auto st = RdStatistics::Instance();
//        auto msg = std::make_shared<Message>();
//        msg->set_type(tc::kServerAudioSpectrum);
//        auto sas = msg->mutable_server_audio_spectrum();
//        sas->set_samples(st->audio_samples_);
//        sas->set_bits(st->audio_bits_);
//        sas->set_channels(st->audio_channels_);
//        sas->mutable_left_spectrum()->Add(st->left_spectrum_.begin(), st->left_spectrum_.end());
//        sas->mutable_right_spectrum()->Add(st->right_spectrum_.begin(), st->right_spectrum_.end());
//        return msg->SerializeAsString();
//    }

    std::string NetMessageMaker::MakeCursorInfoSyncMsg(uint32_t x, uint32_t y, uint32_t hotspot_x, uint32_t hotspot_y,
                                                       uint32_t  width, uint32_t height, bool visable,
                                                       const std::shared_ptr<Data>& data, uint32_t type) {
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kCursorInfoSync);
        auto cursor_info = new CursorInfoSync();
        cursor_info->set_visible(visable);
        cursor_info->set_width(width);
        cursor_info->set_height(height);
        cursor_info->set_x(x);
        cursor_info->set_y(y);
        cursor_info->set_hotspot_x(hotspot_x);
        cursor_info->set_hotspot_y(hotspot_y);
        cursor_info->set_bitmap(data->DataAddr(), data->Size());
        cursor_info->set_type((CursorInfoSync::CursorType)type);
        msg->set_allocated_cursor_info_sync(cursor_info);
        return msg->SerializeAsString();
    }

    std::string NetMessageMaker::MakeMonitorSwitched(const std::string& name) {
        tc::Message msg;
        msg.set_type(kMonitorSwitched);
        msg.mutable_monitor_switched()->set_name(name);
        return msg.SerializeAsString();
    }

}