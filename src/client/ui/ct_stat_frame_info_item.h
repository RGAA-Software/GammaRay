//
// Created by RGAA on 26/04/2025.
//

#ifndef GAMMARAY_STAT_CAPTURE_INFO_ITEM_H
#define GAMMARAY_STAT_CAPTURE_INFO_ITEM_H

#include "tc_qt_widget/tc_base_widget.h"

namespace tc
{

    class TcLabel;
    class ClientContext;

    class CtStatItemInfo {
    public:
        std::string name_;
        int frame_width_{0};
        int frame_height_{0};
        int received_fps_{0};
        int render_capture_fps_{0};
        int render_capture_frame_width_{0};
        int render_capture_frame_height_{0};
        std::string render_encoder_name_;
        int render_encode_fps_{0};
    };

    class CtStatFrameInfoItem : public TcBaseWidget {
    public:
        CtStatFrameInfoItem(const std::shared_ptr<ClientContext>& ctx, QWidget* parent);
        void paintEvent(QPaintEvent *event) override;
        std::string GetTargetName();
        void UpdateInfo(const CtStatItemInfo& info);
        void ClearInfo();
        void Select();
        void Unselect();

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        bool selected_ = false;
        TcLabel* lbl_target_name_ = nullptr;
        TcLabel* lbl_render_size_ = nullptr;
        TcLabel* lbl_received_fps_ = nullptr;
        TcLabel* lbl_render_capture_fps_ = nullptr;
        TcLabel* lbl_render_capture_size_ = nullptr;
        TcLabel* lbl_render_encoder_name_ = nullptr;
        TcLabel* lbl_render_encode_fps_ = nullptr;
    };

}

#endif //GAMMARAY_STAT_CAPTURE_INFO_ITEM_H
