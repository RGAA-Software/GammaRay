//
// Created by RGAA on 2024-04-20.
//

#include "gr_statistics.h"
#include "tc_message.pb.h"
#include "gr_app_messages.h"
#include "gr_context.h"
#include "tc_common_new/log.h"

namespace tc
{

    void GrStatistics::RegisterEventListeners() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgCaptureStatistics>([=, this](const MsgCaptureStatistics& msg) {
            this->video_frame_gaps_.clear();
            this->video_frame_gaps_.insert(this->video_frame_gaps_.begin(),
                                           msg.statistics_->video_frame_gaps().begin(),
                                           msg.statistics_->video_frame_gaps().end());
            this->encode_durations_.clear();
            this->encode_durations_.insert(this->encode_durations_.begin(),
                                           msg.statistics_->encode_durations().begin(),
                                           msg.statistics_->encode_durations().end());
            this->audio_frame_gaps_.clear();
            this->audio_frame_gaps_.insert(this->audio_frame_gaps_.begin(),
                                           msg.statistics_->audio_frame_gaps().begin(),
                                           msg.statistics_->audio_frame_gaps().end());
            //this->decode_durations_.clear();
            //this->decode_durations_.insert(this->decode_durations_.begin(),
            //                               msg.statistics_->decode_durations().begin(),
            //                               msg.statistics_->decode_durations().end());
            //this->client_video_recv_gaps_.clear();
            //this->client_video_recv_gaps_.insert(this->client_video_recv_gaps_.begin(),
            //                               msg.statistics_->client_video_recv_gaps().begin(),
            //                               msg.statistics_->client_video_recv_gaps().end());
            //this->client_fps_video_recv_ = msg.statistics_->client_fps_video_recv();
            //this->client_fps_render_ = msg.statistics_->client_fps_render();
            //this->client_recv_media_data_ = msg.statistics_->client_recv_media_data();
            // from inner server
            this->fps_video_encode = msg.statistics_->fps_video_encode();
            // from inner server
            this->app_running_time = msg.statistics_->app_running_time();
            // from inner server
            this->server_send_media_bytes = msg.statistics_->server_send_media_data();
            //this->render_width_ = msg.statistics_->render_width();
            //this->render_height_ = msg.statistics_->render_height();
            //this->capture_width_ = msg.statistics_->capture_width();
            //this->capture_height_ = msg.statistics_->capture_height();

            // captures information
            {
                captures_info_.clear();
                int size = msg.statistics_->working_captures_info_size();
                for (int i = 0; i < size; i++) {
                    auto info = msg.statistics_->working_captures_info(i);
                    PtMsgWorkingCaptureInfo cpy_info;
                    cpy_info.CopyFrom(info);
                    captures_info_.push_back(cpy_info);

                    if (video_capture_type_ != info.capture_type()) {
                        video_capture_type_ = info.capture_type();
                    }
                }
            }

            connected_clients_ = msg.statistics_->connected_clients();
            relay_connected_ = msg.statistics_->relay_connected();
            audio_capture_type_ = msg.statistics_->audio_capture_type();
        });

        msg_listener_->Listen<MsgServerAudioSpectrum>([=, this](const MsgServerAudioSpectrum& msg) {
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
        });
    }

}
