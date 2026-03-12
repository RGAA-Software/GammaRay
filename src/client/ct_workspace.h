//
// Created by RGAA on 2023-12-27.
//

#ifndef TC_CLIENT_PC_WORKSPACE_H
#define TC_CLIENT_PC_WORKSPACE_H

#include <QWidget>
#include <QMainWindow>
#include <QLibrary>
#include <map>
#include <vector>
#include <qlist.h>
#include "thunder_sdk.h"
#include "theme/QtAdvancedStylesheet.h"
#include "client/ct_app_message.h"
#include "ct_base_workspace.h"

namespace tc
{

    class GameView;

    class Workspace : public BaseWorkspace {
    public:
        explicit Workspace(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent = nullptr);
        ~Workspace() override;

        void Init() override;
        bool eventFilter(QObject* watched, QEvent* event) override;
        void SendWindowsKey(unsigned long vk, bool down) override;
    protected:
        void InitGameView(const std::shared_ptr<ThunderSdkParams>& params) override;
        void RegisterBaseListeners() override;
    private:
        void RegisterSdkMsgCallbacks() override;
        void CalculateAspectRatio() override;
        void SwitchToFillWindow() override;
        void UpdateGameViewsStatus(bool force_layout_screens) override;
        void OnGetCaptureMonitorsCount(int monitors_count) override;
        void OnGetCaptureMonitorName(std::string monitor_name) override;
    private:
        std::vector<GameView*> game_views_;  
  
        EMultiMonDisplayMode multi_display_mode_ = EMultiMonDisplayMode::kTab;     
    private:
        void ListenMultiMonDisplayModeMessage();

        // auto split the windows
        std::once_flag send_split_windows_flag_;

        // auto layout the windows
        std::once_flag layout_windows_;
    };

}

#endif //TC_CLIENT_PC_WORKSPACE_H
