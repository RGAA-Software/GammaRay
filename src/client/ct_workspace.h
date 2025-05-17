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

    class Workspace : public QMainWindow, public std::enable_shared_from_this<Workspace> {
    public:

        explicit Workspace(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent = nullptr);
        ~Workspace() override;

        void Init();

        void closeEvent(QCloseEvent *event) override;
        void changeEvent(QEvent* event) override;
        [[nodiscard]] bool IsActiveNow() const;
        void resizeEvent(QResizeEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        bool eventFilter(QObject* watched, QEvent* event) override;
        void SendWindowsKey(unsigned long vk, bool down);

        std::shared_ptr<ThunderSdk> GetThunderSdk();
        std::shared_ptr<ClientContext> GetContext();

    private:
        void RegisterSdkMsgCallbacks();
        void Exit();
        void UpdateNotificationHandlePosition();
        void UpdateLocalCursor(uint32_t type);
        void RegisterControllerPanelListeners();
        void UpdateDebugPanelPosition();
        void SendClipboardMessage(const ClipboardMessage& msg);
        void SendSwitchMonitorMessage(const std::string& name);
        void SendSwitchWorkModeMessage(SwitchWorkMode::WorkMode mode);
        void SendSwitchFullColorMessage(bool enable);
        // client->render 发送刷新桌面的消息
        void SendUpdateDesktopMessage();
        void SwitchScaleMode(const ScaleMode& mode);
        void CalculateAspectRatio();
        void SwitchToFullWindow();
        void SendChangeMonitorResolutionMessage(const MsgChangeMonitorResolution& msg);
        void UpdateFloatButtonIndicatorPosition();
        void UpdateVideoWidgetSize();
        void UpdateGameViewsStatus();
        void OnGetCaptureMonitorsCount(int monitors_count);
        void OnGetCaptureMonitorName(std::string monitor_name);
        void InitGameViews(const std::shared_ptr<ThunderSdkParams>& params);
        void WidgetSelectMonitor(QWidget* widget, QList<QScreen*>& screens);
        void ExitClientWithDialog();

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        std::shared_ptr<AudioPlayer> audio_player_ = nullptr;
        bool is_window_active_ = false;
        acss::QtAdvancedStylesheet* theme_{};
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        Settings* settings_ = nullptr;
        FloatNotificationHandle* notification_handler_ = nullptr;
        NotificationPanel* notification_panel_ = nullptr;
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

    private:
        // 扩展屏
        // to do:
        // 1.将gameview这个类抽象的比较完善, 后面都可以使用 game_views_ 来表示，主屏 与 扩展屏 放在一个vector里 逻辑清晰
        // 2.可能连上对端后，对端设置的显示器采集模式 就是 all，那么客户端默认 kTab 就不合理了 (已完成)
        // 3.70这台电脑,如果dda采集失败了，切换到GDI采集，研究下，GDI采集是否能单独采集某个屏幕
        // 4.game_view 产生的位置要错开(已完成)
        // 5.现在是每秒同步一次显示器信息,导致关闭掉的扩展屏 会再次显示，要改为监听win消息的方式，有变化再通知(已完成)
        // 6.屏幕切换的图标数量 (已完成)
        // 7.获取任一game_view的关闭事件(已完成)
        // 8.每个game_view的标题名字(已完成)
        // 9.切换屏幕按钮，以及分屏合并操作，要刷新桌面 (已完成)
        // 10.关闭弹窗显示在 点击关闭按钮所在的屏幕上(已完成)
        // 11.全屏动作，扩展屏未生效 (已完成)
        // 12.关闭的时候，将widget显示到前面,尤其是在任务栏关闭(已完成)
        // 13.点击文件传输按钮, 文件传输页面显示到前面(已完成)
        // 14.setting 保存各种模式的时候，不需要重复保存，避免重复保存
        // 15.测试一开始 又出现没有当前显示器标识的情况
        // 16.分辨率还要传递回来 当前真正使用的分辨率
        // 17.全屏的时候，多屏之间 要同步全屏或者退出全屏(已完成)
        // 18.不知道为什么会闪烁(已解决)
        // 19.如果对端第一次启动,好像配置信息没有正确传递过来
        // 20.文件传输加接口 控制速度等
        // 21.game_view上的小球，点击后，会full window,需要找下原因
        // 22.刷新 参考 vnc
        QString origin_title_name_;
        std::vector<GameView*> game_views_;  
        std::map<int, std::string> monitor_index_map_name_;
        

        EMultiMonDisplayMode multi_display_mode_ = EMultiMonDisplayMode::kTab;
        int monitors_count_ = 0;
        QWidget* close_event_occurred_widget_ = nullptr;
    };

}

#endif //TC_CLIENT_PC_WORKSPACE_H
