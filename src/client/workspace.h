//
// Created by RGAA on 2023-12-27.
//

#ifndef TC_CLIENT_PC_WORKSPACE_H
#define TC_CLIENT_PC_WORKSPACE_H

#include <QWidget>
#include <QMainWindow>
#include "thunder_sdk.h"
#include "theme/QtAdvancedStylesheet.h"
#include "app_message.h"

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
        void SendSwitchMonitorMessage(int index, const std::string& name);
        void SendSwitchWorkModeMessage(SwitchWorkMode::WorkMode mode);
        void SwitchScaleMode(const ScaleMode& mode);
        void CalculateAspectRatio();
        void SwitchToFullWindow();
        void SendChangeMonitorResolutionMessage(const MsgChangeMonitorResolution& msg);
        void UpdateFloatButtonIndicatorPosition();
        void UpdateVideoWidgetSize();

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        std::shared_ptr<AudioPlayer> audio_player_ = nullptr;
        OpenGLVideoWidget* video_widget_ = nullptr;
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
        DebugPanel* debug_panel_ = nullptr;
        std::shared_ptr<ClipboardManager> clipboard_mgr_ = nullptr;
        FloatButtonStateIndicator* btn_indicator_ = nullptr;
        std::atomic_bool has_frame_arrived_ = false;
    };

}

#endif //TC_CLIENT_PC_WORKSPACE_H
