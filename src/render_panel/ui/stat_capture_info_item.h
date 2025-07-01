//
// Created by RGAA on 26/04/2025.
//

#ifndef GAMMARAY_STAT_CAPTURE_INFO_ITEM_H
#define GAMMARAY_STAT_CAPTURE_INFO_ITEM_H

#include "tc_qt_widget/tc_base_widget.h"
#include "tc_render_panel_message.pb.h"

namespace tc
{

    class TcLabel;
    class GrContext;

    class StatCaptureInfoItem : public TcBaseWidget {
    public:
        StatCaptureInfoItem(const std::shared_ptr<GrContext>& ctx, QWidget* parent);
        void paintEvent(QPaintEvent *event) override;
        std::string GetTargetName();
        void UpdateInfo(const std::shared_ptr<tcrp::RpMsgWorkingCaptureInfo>& info);
        void ClearInfo();
        void Select();
        void Unselect();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        bool selected_ = false;
        TcLabel* lbl_target_name_;
        TcLabel* lbl_capture_size_;
        TcLabel* lbl_capture_fps_;
        TcLabel* lbl_frame_resize_size_;
        TcLabel* lbl_encode_fps_;
    };

}

#endif //GAMMARAY_STAT_CAPTURE_INFO_ITEM_H
