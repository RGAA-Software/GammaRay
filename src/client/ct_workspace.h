//
// Created by RGAA on 2023-12-27.
//

#ifndef TC_CLIENT_PC_WORKSPACE_H
#define TC_CLIENT_PC_WORKSPACE_H

#include <QWidget>
#include <QMainWindow>
#include <map>
#include <vector>
#include "thunder_sdk.h"
#include "theme/QtAdvancedStylesheet.h"
#include "client/ct_app_message.h"

namespace tc
{

    class ClientContext;
    class ThunderSdk;
    class OpenGLVideoWidget;
    class AudioPlayer;
    class FloatController;
    class FloatControllerPanel;
    class MessageListener;
    class Settings;
    class FloatNotificationHandle;
    class NotificationPanel;
    class FileTransferChannel;
    class DebugPanel;
    class ClipboardManager;
    class FloatButtonStateIndicator;
    class FileTransInterface;
    class MainProgress;
    class GameView;

    class Workspace : public QMainWindow {
    public:

        explicit Workspace(const std::shared_ptr<ClientContext>& ctx, const ThunderSdkParams& params, QWidget* parent = nullptr);
        ~Workspace() override;

        void closeEvent(QCloseEvent *event) override;
        void changeEvent(QEvent* event) override;
        [[nodiscard]] bool IsActiveNow() const;
        void resizeEvent(QResizeEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        void SendWindowsKey(unsigned long vk, bool down);

    private:
        void RegisterSdkMsgCallbacks();
        void Exit();
        void UpdateNotificationHandlePosition();
        void UpdateLocalCursor(uint32_t type);
        void RegisterControllerPanelListeners();
        void UpdateDebugPanelPosition();
        void SendClipboardMessage(const std::string& msg);
        void SendSwitchMonitorMessage(const std::string& name);
        void SendSwitchWorkModeMessage(SwitchWorkMode::WorkMode mode);
        void SwitchScaleMode(const ScaleMode& mode);
        void CalculateAspectRatio();
        void SwitchToFullWindow();
        void SendChangeMonitorResolutionMessage(const MsgChangeMonitorResolution& msg);
        void UpdateFloatButtonIndicatorPosition();
        void UpdateVideoWidgetSize();
        void UpdateGameViewsStatus();
        void OnGetCaptureMonitorsCount(int monitors_count);
        void OnGetCaptureMonitorName(std::string monitor_name);

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        std::shared_ptr<AudioPlayer> audio_player_ = nullptr;
        FloatController* float_controller_ = nullptr;
        FloatControllerPanel* controller_panel_ = nullptr;
        bool is_window_active_ = false;
        acss::QtAdvancedStylesheet* theme_{};
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        Settings* settings_ = nullptr;
        FloatNotificationHandle* notification_handler_ = nullptr;
        NotificationPanel* notification_panel_ = nullptr;
        std::shared_ptr<FileTransferChannel> file_transfer_ = nullptr;
        uint32_t cursor_type_ = 100000;
        bool force_update_cursor_ = true;
        DebugPanel* debug_panel_ = nullptr;
        std::shared_ptr<ClipboardManager> clipboard_mgr_ = nullptr;
        FloatButtonStateIndicator* btn_indicator_ = nullptr;
        std::atomic_bool has_frame_arrived_ = false;

        //文件传输:
        std::shared_ptr<FileTransInterface> file_trans_interface_ = nullptr;

        // progress
        MainProgress* main_progress_ = nullptr;

        int title_bar_height_ = 0; //35;
    private:
        // 扩展屏
        // to do:
        // 1.将将gameview这个类抽象的比较完善, 后面都可以使用 game_views_ 来表示，主屏 与 扩展屏 放在一个vector里 逻辑清晰
        // 2.可能连上对端后，对端设置的显示器采集模式 就是 all，那么客户端默认 kTab 就不合理了 (已完成)
        // 3.70这台电脑,如果dda采集失败了，切换到GDI采集，研究下，GDI采集是否能单独采集某个屏幕
        // 4.game_view 产生的位置要错开
        // 5.现在是每秒同步一次显示器信息,导致关闭掉的扩展屏 会再次显示，要改为监听win消息的方式，有变化再通知
        // 6.屏幕切换的图标数量
        std::vector<GameView*> game_views_;  
        std::map<std::string, int> monitor_name_map_index_;
        const int kMaxGameViewCount = 8;

        EMultiMonDisplayMode multi_display_mode_ = EMultiMonDisplayMode::kTab;

    };

}

#endif //TC_CLIENT_PC_WORKSPACE_H
