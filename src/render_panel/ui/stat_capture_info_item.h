//
// Created by RGAA on 26/04/2025.
//

#ifndef GAMMARAY_STAT_CAPTURE_INFO_ITEM_H
#define GAMMARAY_STAT_CAPTURE_INFO_ITEM_H

#include "tc_qt_widget/tc_base_widget.h"
#include "tc_message.pb.h"

namespace tc
{

    class TcLabel;
    class GrContext;

    class StatCaptureInfoItem : public TcBaseWidget {
    public:
        StatCaptureInfoItem(const std::shared_ptr<GrContext>& ctx, QWidget* parent);
        void paintEvent(QPaintEvent *event) override;
        std::string GetTargetName();
        void UpdateInfo(const PtMsgWorkingCaptureInfo& info);
        void ClearInfo();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        TcLabel* lbl_target_name_;
        TcLabel* lbl_capture_size_;
        TcLabel* lbl_capture_fps_;
        TcLabel* lbl_encode_fps_;
    };

}

#endif //GAMMARAY_STAT_CAPTURE_INFO_ITEM_H
