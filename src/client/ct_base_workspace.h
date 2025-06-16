//
// Created by RGAA on 2023-12-27.
//
#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QLibrary>
#include <map>
#include <vector>
#include <qlist.h>
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
    class CtStatisticsPanel;
    class ClipboardManager;
    class FloatButtonStateIndicator;
    class FileTransInterface;
    class MainProgress;
    class GameView;
    class RtcClientInterface;
    class CtPanelClient;
    class ClientPluginManager;
    class MediaRecordPluginClientInterface;

    class BaseWorkspace : public QMainWindow, public std::enable_shared_from_this<BaseWorkspace> {
    public:

        explicit BaseWorkspace(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent = nullptr);
        ~BaseWorkspace() override;

        virtual void Init();

        void closeEvent(QCloseEvent *event) override;
        void changeEvent(QEvent* event) override;
        [[nodiscard]] bool IsActiveNow() const;
        void resizeEvent(QResizeEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        bool eventFilter(QObject* watched, QEvent* event) override;
        virtual void SendWindowsKey(unsigned long vk, bool down);

        std::shared_ptr<ThunderSdk> GetThunderSdk();
        std::shared_ptr<ClientContext> GetContext();

    protected:
        
        void InitPluginsManager();
        void InitTheme();
        void InitSampleWidget();
        virtual void InitListener();
        void InitFileTrans();
        void InitPanelClient();
        void InitClipboardManager();

        virtual void RegisterSdkMsgCallbacks();
        void Exit();
        //void UpdateNotificationHandlePosition();
        void UpdateLocalCursor();


        virtual void RegisterBaseListeners();
        void RegisterControllerPanelListeners();

        void UpdateDebugPanelPosition();
        void SendClipboardMessage(const ClipboardMessage& msg);
        void SendSwitchMonitorMessage(const std::string& name);
        void SendSwitchWorkModeMessage(SwitchWorkMode::WorkMode mode);
        void SendSwitchFullColorMessage(bool enable);
        // client->render 发送刷新桌面的消息
        void SendUpdateDesktopMessage();
        // client->render 发送修改帧率的消息
        void SendModifyFpsMessage();
        void SendHardUpdateDesktopMessage();
        void SwitchScaleMode(const ScaleMode& mode);
        virtual void CalculateAspectRatio();
        virtual void SwitchToFullWindow();
        void SendChangeMonitorResolutionMessage(const MsgChangeMonitorResolution& msg);
        void UpdateFloatButtonIndicatorPosition();
        void UpdateVideoWidgetSize();
        virtual void UpdateGameViewsStatus();
        virtual void OnGetCaptureMonitorsCount(int monitors_count);
        virtual void OnGetCaptureMonitorName(std::string monitor_name);
        virtual void InitGameView(const std::shared_ptr<ThunderSdkParams>& params);
        void WidgetSelectMonitor(QWidget* widget, QList<QScreen*>& screens);
        void ExitClientWithDialog();

        // 匹配鼠标形状
        Qt::CursorShape ToQCursorShape(uint32_t cursor_type);

    protected:
        std::shared_ptr<ThunderSdkParams> params_ = nullptr;
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        std::shared_ptr<AudioPlayer> audio_player_ = nullptr;
        bool is_window_active_ = false;
        acss::QtAdvancedStylesheet* theme_{};
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        Settings* settings_ = nullptr;
        //FloatNotificationHandle* notification_handler_ = nullptr;
        //NotificationPanel* notification_panel_ = nullptr;
        std::shared_ptr<FileTransferChannel> file_transfer_ = nullptr;
        uint32_t cursor_type_ = 100000;
        bool force_update_cursor_ = true;
        CtStatisticsPanel* st_panel_ = nullptr;
        std::shared_ptr<ClipboardManager> clipboard_mgr_ = nullptr;
        FloatButtonStateIndicator* btn_indicator_ = nullptr;
        std::atomic_bool has_frame_arrived_ = false;

        //文件传输:
        std::shared_ptr<FileTransInterface> file_trans_interface_ = nullptr;

        // progress
        MainProgress* main_progress_ = nullptr;

        int title_bar_height_ = 0; //35;

        bool full_screen_ = false;

        //
        std::shared_ptr<CtPanelClient> panel_client_ = nullptr;

        // plugin manager
        std::shared_ptr<ClientPluginManager> plugin_manager_ = nullptr;

        MediaRecordPluginClientInterface* media_record_plugin_ = nullptr;

        QCursor cursor_;

    protected:
        QString origin_title_name_;
     
        std::map<int, std::string> monitor_index_map_name_;

        int monitors_count_ = 0;

        QWidget* close_event_occurred_widget_ = nullptr;

    private:
        GameView* game_view_ = nullptr;
    };

}
