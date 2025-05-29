//
// Created by RGAA on 4/07/2024.
//

#ifndef GAMMARAYPC_FLOAT_CONTROLLER_PANEL_H
#define GAMMARAYPC_FLOAT_CONTROLLER_PANEL_H
#include <string>
#include "base_widget.h"
#include "client/ct_app_message.h"

class QLabel;

namespace tc
{

    enum class SubPanelType {
        kWorkMode,
        kControl,
        kDisplay,
        kFileTransfer,
        kDebug,
    };

    class ComputerIcon;
    class FloatIcon;

    class FloatControllerPanel : public BaseWidget {
    public:
        explicit FloatControllerPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;

        void SetOnDebugListener(OnClickListener&& l) { debug_listener_ = l; }
        void SetOnFileTransListener(OnClickListener&& listener) { file_trans_listener_ = listener; }
        void SetOnMediaRecordListener(OnClickListener&& listener) { media_record_listener_ = listener; }
        void Hide() override;
        void SetMainControl();
        void SetMonitorName(const std::string& mon_name);
    private:
        BaseWidget* GetSubPanel(const SubPanelType& type);
        void HideAllSubPanels();
        void UpdateCaptureMonitorInfo();
        void SwitchMonitor(ComputerIcon* w);
        void CaptureAllMonitor();
        void UpdateCapturingMonitor(const std::string& name, int cur_cap_mon_index);
        void UpdateStatus(const FloatControllerPanelUpdateMessage& msg) override;
    private:
        OnClickListener debug_listener_;
        OnClickListener file_trans_listener_;
        OnClickListener media_record_listener_;
        std::map<SubPanelType, BaseWidget*> sub_panels_;
        std::vector<ComputerIcon*> computer_icons_;
        CaptureMonitorMessage capture_monitor_;

        const int kInitialWidth = 240;

        FloatIcon* audio_btn_ = nullptr;

        FloatIcon* full_screen_btn_ = nullptr;

        //是否是主窗口的控制面板
        bool is_main_control_ = false;

        std::string monitor_name_;

        QLabel* media_record_lab_ = nullptr;
    };

}

#endif //GAMMARAYPC_FLOAT_CONTROLLER_PANEL_H
