//
// Created by RGAA on 2023-12-27.
//
#pragma once

#include <QWidget>
#include <QLibrary>
#include <QMainWindow>
#include <map>
#include <vector>
#include <qlist.h>
#include "thunder_sdk.h"
#include "client/ct_app_message.h"
#include "theme/QtAdvancedStylesheet.h"

#ifdef WIN32
#include <d3d11.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

#endif

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
    class FloatButtonStateIndicator;
    class FileTransInterface;
    class MainProgress;
    class GameView;
    class RtcClientInterface;
    class CtPanelClient;
    class ClientPluginManager;
    class MediaRecordPluginClientInterface;
    class RetryConnDialog;
    class D3D11DeviceWrapper;
    class HWInfoWidget;
    class CtSpvrClient;
    class PlVulkan;


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
        bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
        // virtual void focusOutEvent(QFocusEvent* event) override; 此窗口接收不到, 原因未知
        virtual void SendWindowsKey(unsigned long vk, bool down);

        std::shared_ptr<ThunderSdk> GetThunderSdk();
        std::shared_ptr<ClientContext> GetContext();
        std::shared_ptr<D3D11DeviceWrapper> GetD3D11DeviceWrapper(uint64_t adapter_uid);
        void PostMediaMessage(std::shared_ptr<Data> msg);
        void PostFileTransferMessage(std::shared_ptr<Data> msg);

    protected:
        void InitPluginsManager();
        void InitTheme();
        void InitSampleWidget();
        virtual void InitListener();
        void InitPanelClient();

        virtual void RegisterSdkMsgCallbacks();
        void Exit();
        //void UpdateNotificationHandlePosition();
        void UpdateLocalCursor();

        virtual void RegisterBaseListeners();
        void RegisterControllerPanelListeners();

        void UpdateDebugPanelPosition();
        void SendClipboardMessage(const MsgClientClipboard& msg);
        void SendSwitchMonitorMessage(const std::string& name);
        void SendSwitchWorkModeMessage(SwitchWorkMode::WorkMode mode);
        void SendSwitchFullColorMessage(bool enable);
        // client->render 发送刷新桌面的消息
        void SendUpdateDesktopMessage();
        // client->render 发送修改帧率的消息
        void SendModifyFpsMessage();
        // client->render 发送退出被控端的消息
        void SendExitControlledEndMessage();
        void SendHardUpdateDesktopMessage();
        void SwitchScaleMode(const ScaleMode& mode);
        virtual void CalculateAspectRatio();
        virtual void SwitchToFullWindow();
        void SendChangeMonitorResolutionMessage(const MsgClientChangeMonitorResolution& msg);
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

        // reconnect when the remote device was in relay mode
        void ReconnectInRelayMode();
        // dismiss connecting dialog
        void DismissConnectingDialog();

        // messages defined in tc_message.proto
        void ProcessNetworkMessage(const std::shared_ptr<tc::Message>& msg);

    private:
        //uint64_t adapter_uid
        bool GenerateD3DDevice();

        // exit sdk
        void ExitSdk();

    protected:
        Settings* settings_ = nullptr;
        std::shared_ptr<ThunderSdkParams> params_ = nullptr;
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        std::shared_ptr<AudioPlayer> audio_player_ = nullptr;
        bool is_window_active_ = false;
        acss::QtAdvancedStylesheet* theme_{};
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<FileTransferChannel> file_transfer_ = nullptr;
        uint32_t cursor_type_ = 100000;
        bool force_update_cursor_ = true;
        CtStatisticsPanel* st_panel_ = nullptr;
        FloatButtonStateIndicator* btn_indicator_ = nullptr;
        std::atomic_bool has_frame_arrived_ = false;

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

        QString origin_title_name_;
     
        std::map<int, std::string> monitor_index_map_name_;

        int monitors_count_ = 0;

        QWidget* close_event_occurred_widget_ = nullptr;

        std::string last_cursor_bitmap_data_;
        std::string cursor_bitmap_data_;

        QSize def_window_size_ = QSize(1366, 768);

        // disconnected dialog
        std::shared_ptr<RetryConnDialog> retry_conn_dialog_ = nullptr;
        std::atomic_bool remote_force_closed_ = false;

        // uint64_t adapter_uid <==> D3D11Device/D3D11DeviceContext
        std::map<uint64_t, std::shared_ptr<D3D11DeviceWrapper>> d3d11_devices_;

        // show remote hardware info
        HWInfoWidget* hw_info_widget_ = nullptr;

        // spvr client
        std::shared_ptr<CtSpvrClient> spvr_client_ = nullptr;

        // can generate relative d3d11device & context
        bool gen_d3d11_device_ = false;

        // libplacebo vulkan
        std::shared_ptr<PlVulkan> pl_vulkan_ = nullptr;

        std::string render_type_name_ = "unknow";
    private:
        GameView* game_view_ = nullptr;
    };

    extern std::shared_ptr<BaseWorkspace> gWorkspace;

}
