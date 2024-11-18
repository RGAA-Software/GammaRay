//
// Created by RGAA on 15/11/2024.
//

#include "gr_plugin_interface.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"
#include "gr_plugin_events.h"

namespace tc
{

    std::string GrPluginInterface::GetPluginName() {
        return "dummy";
    }

    GrPluginType GrPluginInterface::GetPluginType() {
        return plugin_type_;
    }

    bool GrPluginInterface::IsStreamPlugin() {
        return plugin_type_ == GrPluginType::kStream;
    }

    std::string GrPluginInterface::GetVersionName() {
        return "1.0.0";
    }

    uint32_t GrPluginInterface::GetVersionCode() {
        return 1;
    }

    bool GrPluginInterface::IsPluginEnabled() {
        return enabled_;
    }

    void GrPluginInterface::EnablePlugin() {
        enabled_ = true;
    }

    void GrPluginInterface::DisablePlugin() {
        enabled_ = false;
    }

    bool GrPluginInterface::IsWorking() {
        return false;
    }

    bool GrPluginInterface::OnCreate(const GrPluginParam& param) {
        this->param_ = param;
        this->enabled_ = true;
        if (param.cluster_.contains("name")) {
            auto n = param.cluster_.at("name");
            plugin_file_name_ = std::any_cast<std::string>(n);
        }

        work_thread_ = Thread::Make(GetPluginName(), 1024);
        work_thread_->Poll();

        timer_ = std::make_shared<asio2::timer>();

        return true;
    }

    bool GrPluginInterface::OnResume() {
        this->stopped_ = false;
        return true;
    }

    bool GrPluginInterface::OnStop() {
        this->stopped_ = true;
        return true;
    }

    bool GrPluginInterface::OnDestroy() {
        if (work_thread_) {
            work_thread_->Exit();
        }
        if (timer_) {
            timer_->stop_all_timers();
        }
        return true;
    }

    void GrPluginInterface::OnVideoEncoderCreated(const GrPluginEncodedVideoType& type, int width, int height) {

    }

    void GrPluginInterface::OnEncodedVideoFrame(const GrPluginEncodedVideoType& video_type,
                                 const std::shared_ptr<Data>& data,
                                 uint64_t frame_index,
                                 int frame_width,
                                 int frame_height,
                                 bool key) {

    }

    void GrPluginInterface::OnEncodedVideoFrameInProtobufFormat(const std::string& msg) {

    }

    void GrPluginInterface::OnRawVideoFrame(uint64_t handle) {

    }

    void GrPluginInterface::OnRawVideoFrame(const std::shared_ptr<Image>& image) {

    }

    void GrPluginInterface::OnAudioFormat(int samples, int channels, int bits) {

    }

    void GrPluginInterface::OnRawAudioData(const std::shared_ptr<Data>& data) {

    }

    void GrPluginInterface::OnSplitRawAudioData(const std::shared_ptr<Data>& left_ch_data, const std::shared_ptr<Data>& right_ch_data) {

    }

    void GrPluginInterface::OnSplitFFTAudioData(const std::vector<double>& left_fft, const std::vector<double>& right_fft) {

    }

    void GrPluginInterface::PostWorkThread(std::function<void()>&& task) {
        if (work_thread_ && !stopped_) {
            work_thread_->Post(std::move(task));
        }
    }

    void GrPluginInterface::RegisterEventCallback(const GrPluginEventCallback& cbk) {
        event_cbk_ = cbk;
    }

    void GrPluginInterface::CallbackEvent(const std::shared_ptr<GrPluginBaseEvent>& event) {
        if (!event_cbk_) {
            return;
        }
        PostWorkThread([=, this]() {
            event_cbk_(event);
        });
    }

    void GrPluginInterface::StartTimer(int millis, std::function<void()>&& cbk) {
        if (timer_) {
            timer_->start_timer(std::to_string(millis), millis, std::move(cbk));
        }
    }

    void GrPluginInterface::On1Second() {

    }

}