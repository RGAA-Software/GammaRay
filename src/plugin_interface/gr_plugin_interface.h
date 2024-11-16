//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_INTERFACE_H
#define GAMMARAY_GR_PLUGIN_INTERFACE_H

#include <mutex>
#include <map>
#include <any>
#include <string>
#include <vector>
#include <functional>

#include "asio2/asio2.hpp"

namespace tc
{

    class Data;
    class Image;
    class Thread;
    class GrPluginBaseEvent;

    // param
    class GrPluginParam {
    public:
        std::map<std::string, std::any> cluster_;
    };

    // encoded video type
    enum class GrPluginEncodedVideoType {
        kH264,
        kH265,
        kVp8,
        kVp9,
        kAv1
    };

    // callback
    using GrPluginEventCallback = std::function<void(const std::shared_ptr<GrPluginBaseEvent>&)>;

    // interface
    class GrPluginInterface {
    public:
        GrPluginInterface() = default;
        virtual ~GrPluginInterface() = default;

        GrPluginInterface(const GrPluginInterface&) = delete;
        GrPluginInterface& operator=(const GrPluginInterface&) = delete;

        // info
        virtual std::string GetPluginName();

        // version
        virtual std::string GetVersionName();
        virtual uint32_t GetVersionCode();

        // enable
        virtual bool IsPluginEnabled();
        virtual void EnablePlugin();
        virtual void DisablePlugin();

        // lifecycle
        virtual bool OnCreate(const GrPluginParam& param);
        virtual bool OnResume();
        virtual bool OnStop();
        virtual bool OnDestroy();

        // video
        virtual void OnVideoEncoderCreated(const GrPluginEncodedVideoType& type, int width, int height);
        virtual void OnEncodedVideoFrame(const GrPluginEncodedVideoType& video_type,
                                         const std::shared_ptr<Data>& data,
                                         uint64_t frame_index,
                                         int frame_width,
                                         int frame_height,
                                         bool key,
                                         int mon_idx,
                                         const std::string& display_name,
                                         int mon_left,
                                         int mon_top,
                                         int mon_right,
                                         int mon_bottom);
        // to see format detail in tc_message_new/tc_message.proto
        // message VideoFrame { ... }
        // you can send it to any clients
        virtual void OnEncodedVideoFrameInProtobufFormat(const std::string& msg);

        // audio
        virtual void OnAudioFormat(int samples, int channels, int bits);
        virtual void OnRawAudioData(const std::shared_ptr<Data>& data);
        virtual void OnSplitRawAudioData(const std::shared_ptr<Data>& left_ch_data, const std::shared_ptr<Data>& right_ch_data);
        virtual void OnSplitFFTAudioData(const std::vector<double>& left_fft, const std::vector<double>& right_fft);

        // task
        void PostWorkThread(std::function<void()>&& task);

        // event
        void RegisterEventCallback(const GrPluginEventCallback& cbk);
        void CallbackEvent(const std::shared_ptr<GrPluginBaseEvent>& event);

        // timer
        void StartTimer(int millis, std::function<void()>&& cbk);

        virtual void On1Second();


    protected:
        std::atomic_bool enabled_ = false;
        std::atomic_bool stopped_ = false;
        GrPluginParam param_;
        std::shared_ptr<Thread> work_thread_ = nullptr;
        GrPluginEventCallback event_cbk_ = nullptr;
        std::shared_ptr<asio2::timer> timer_ = nullptr;
        std::string plugin_file_name_;
    };

}

#endif //GAMMARAY_GR_PLUGIN_INTERFACE_H
