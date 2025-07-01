//
// Created by RGAA on 2024-04-20.
//

#include "gr_statistics.h"
#include "tc_render_panel_message.pb.h"
#include "gr_app_messages.h"
#include "gr_context.h"
#include "tc_common_new/log.h"
#include "tc_common_new/client_id_extractor.h"

namespace tc
{

    void GrStatistics::RegisterEventListeners() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgCaptureStatistics>([=, this](const MsgCaptureStatistics& msg) {
            ProcessCaptureStatistics(msg);
        });

        msg_listener_->Listen<MsgServerAudioSpectrum>([=, this](const MsgServerAudioSpectrum& msg) {
            ProcessAudioSpectrum(msg);
        });

        msg_listener_->Listen<MsgGrTimer1S>([=, this](const MsgGrTimer1S& msg) {
            Process1SCalculation();
        });
    }

    void GrStatistics::ProcessCaptureStatistics(const MsgCaptureStatistics& msg) {
        this->audio_frame_gaps_.clear();
        this->audio_frame_gaps_.insert(this->audio_frame_gaps_.begin(),
                                       msg.statistics_->audio_frame_gaps().begin(),
                                       msg.statistics_->audio_frame_gaps().end());

        // from inner server
        this->app_running_time = msg.statistics_->app_running_time();
        // from inner server
        this->server_send_media_bytes = msg.statistics_->server_send_media_data();

        // captures information
        this->encode_durations_.clear();
        this->video_capture_gaps_.clear();
        this->copy_texture_durations_.clear();
        this->map_cvt_texture_durations_.clear();
        {
            captures_info_.clear();
            int size = msg.statistics_->working_captures_info_size();
            for (int i = 0; i < size; i++) {
                auto info = msg.statistics_->working_captures_info(i);
                auto cpy_info = std::make_shared<tcrp::RpMsgWorkingCaptureInfo>();
                cpy_info->CopyFrom(info);
                captures_info_.push_back(cpy_info);

                if (video_capture_type_ != info.capture_type()) {
                    video_capture_type_ = info.capture_type();
                }

                // encode durations
                std::vector<int32_t> encode_durations;
                for (const auto& v : info.encode_durations()) {
                    encode_durations.push_back(v);
                }
                encode_durations_.insert({info.target_name(), encode_durations});

                // video capture gaps
                std::vector<int32_t> video_capture_gaps;
                for (const auto& v : info.video_capture_gaps()) {
                    video_capture_gaps.push_back(v);
                }
                video_capture_gaps_.insert({info.target_name(), video_capture_gaps});

                // copy texture durations
                std::vector<int32_t> copy_texture_durations;
                for (const auto& v : info.copy_texture_durations()) {
                    copy_texture_durations.push_back(v);
                }
                copy_texture_durations_.insert({info.target_name(), copy_texture_durations});

                // map cvt texture durations
                std::vector<int32_t> map_cvt_texture_durations;
                for (const auto& v : info.map_cvt_texture_durations()) {
                    map_cvt_texture_durations.push_back(v);
                }
                map_cvt_texture_durations_.insert({info.target_name(), map_cvt_texture_durations});
            }
        }

        connected_clients_ = msg.statistics_->connected_clients_count();
        connected_clients_info_.clear();
        for (const auto& item : msg.statistics_->connected_clients()) {
            auto info = std::make_shared<tcrp::RpConnectedClientInfo>();
            info->CopyFrom(item);
            connected_clients_info_.push_back(info);
        }
        context_->PostTask([=, this]() {
            context_->SendAppMessage(MsgUpdateConnectedClientsInfo {
                .clients_info_ = connected_clients_info_,
            });
            // test beg //
            //LOGI("*** Connected client count: {}", connected_clients_info_.size());
            //for (const auto& item : connected_clients_info_) {
            //    LOGI("connected, device id: {}, room id: {}", ExtractClientId(item->device_id()), item->room_id());
            //}
            // test end //
        });

        relay_connected_ = msg.statistics_->relay_connected();
        audio_capture_type_ = msg.statistics_->audio_capture_type();

        {
            auto type = msg.statistics_->video_encode_type();
            if (type == tcrp::VideoType::kNetH264) {
                video_encode_type_ = "H264";
            }
            else if (type == tcrp::VideoType::kNetHevc) {
                video_encode_type_ = "HEVC";
            }
            else if (type == tcrp::VideoType::kNetVp9) {
                video_encode_type_ = "VP9";
            }
            else {
                video_encode_type_ = "N/A";
            }
        }

    }

    void GrStatistics::ProcessAudioSpectrum(const MsgServerAudioSpectrum& msg) {
        this->audio_samples_ = msg.spectrum_->samples();
        this->audio_channels_ = msg.spectrum_->channels();
        this->audio_bits_ = msg.spectrum_->bits();
        if (this->left_spectrum_.size() != msg.spectrum_->left_spectrum().size()) {
            this->left_spectrum_.resize(msg.spectrum_->left_spectrum().size());
        }
        memcpy(this->left_spectrum_.data(), msg.spectrum_->left_spectrum().data(), msg.spectrum_->left_spectrum().size() * sizeof(double));

        if (this->right_spectrum_.size() != msg.spectrum_->right_spectrum().size()) {
            this->right_spectrum_.resize(msg.spectrum_->right_spectrum().size());
        }
        memcpy(this->right_spectrum_.data(), msg.spectrum_->right_spectrum().data(), msg.spectrum_->right_spectrum().size() * sizeof(double));
    }

    void GrStatistics::Process1SCalculation() {
        // speed
        send_speed_bytes = server_send_media_bytes - last_server_send_media_bytes;
        last_server_send_media_bytes = server_send_media_bytes;

        //
    }

}
