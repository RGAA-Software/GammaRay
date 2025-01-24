//
// Created by RGAA on 4/07/2024.
//

#ifndef GAMMARAYPC_FLOAT_CONTROLLER_PANEL_H
#define GAMMARAYPC_FLOAT_CONTROLLER_PANEL_H

#include "base_widget.h"
#include "app_message.h"

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

    class FloatControllerPanel : public BaseWidget {
    public:
        explicit FloatControllerPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;

        void SetOnDebugListener(OnClickListener&& l) { debug_listener_ = l; }
        void Hide() override;
        int GetCurrentMonitorIndex();

    private:
        BaseWidget* GetSubPanel(const SubPanelType& type);
        void HideAllSubPanels();
        void UpdateCaptureMonitorInfo();
        void SwitchMonitor(ComputerIcon* w);
        void UpdateCapturingMonitor(int index, const std::string& name);

    private:
        OnClickListener debug_listener_;
        std::map<SubPanelType, BaseWidget*> sub_panels_;
        std::map<int, ComputerIcon*> computer_icons_;
        CaptureMonitorMessage capture_monitor_;

    };

}

#endif //GAMMARAYPC_FLOAT_CONTROLLER_PANEL_H
