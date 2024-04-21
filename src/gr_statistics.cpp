//
// Created by RGAA on 2024-04-20.
//

#include "gr_statistics.h"

#include "app_messages.h"
#include "gr_context.h"
#include "tc_common_new/log.h"

namespace tc
{

    void GrStatistics::RegisterEventListeners() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgCaptureStatistics>([=, this](const MsgCaptureStatistics& msg) {
            this->video_frame_gaps_.clear();
            this->video_frame_gaps_.insert(this->video_frame_gaps_.begin(),
                                           msg.statistics_.video_frame_gaps().begin(),
                                           msg.statistics_.video_frame_gaps().end());
            this->encode_durations_.clear();
            this->encode_durations_.insert(this->encode_durations_.begin(),
                                           msg.statistics_.encode_durations().begin(),
                                           msg.statistics_.encode_durations().end());
            this->decode_durations_.clear();
            this->decode_durations_.insert(this->decode_durations_.begin(),
                                           msg.statistics_.decode_durations().begin(),
                                           msg.statistics_.decode_durations().end());
        });
    }

}
