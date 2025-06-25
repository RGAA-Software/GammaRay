//
// Created by RGAA on 2023-12-27.
//

#include <QHBoxLayout>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTimer>
#include <dwmapi.h>
#include "client/ct_workspace.h"
#include "thunder_sdk.h"
#include "ct_opengl_video_widget.h"
#include "client/ct_client_context.h"
#include "tc_common_new/data.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "client/ct_audio_player.h"
#include "ui/float_controller.h"
#include "ui/float_controller_panel.h"
#include "client/ct_app_message.h"
#include "client/ct_settings.h"
#include "ui/float_notification_handle.h"
#include "ui/notification_panel.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "ui/ct_statistics_panel.h"
#include "ui/no_margin_layout.h"
#include "tc_client_sdk_new/sdk_messages.h"
#include "tc_common_new/process_util.h"
#include "ui/float_button_state_indicator.h"
#include "ct_main_progress.h"
#include "tc_qt_widget/widgetframe/mainwindow_wrapper.h"
#ifdef TC_ENABLE_FILE_TRANSMISSION
#include "core/file_trans_interface.h"
#endif // TC_ENABLE_FILE_TRANSMISSION
#include "tc_dialog.h"
#include "ct_game_view.h"
#include "ct_const_def.h"
#include "tc_common_new/file.h"
#include "tc_common_new/qwidget_helper.h"
#include "network/ct_panel_client.h"
#include "tc_common_new/time_util.h"
#include "plugins/ct_plugin_manager.h"
#include "plugins/ct_app_events.h"
#include "plugin_interface/ct_plugin_interface.h"
#include "plugins/media_record/media_record_plugin.h"
#include "plugin_interface/ct_media_record_plugin_interface.h"
#include "tc_qt_widget/notify/notifymanager.h"

namespace tc
{

    Workspace::Workspace(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent) : BaseWorkspace(ctx, params, parent) {
        this->context_->full_functionality_ = true;        
    }

    Workspace::~Workspace() {

    }

    void Workspace::RegisterBaseListeners() {
        BaseWorkspace::RegisterBaseListeners();
        ListenMultiMonDisplayModeMessage();
    }

    void Workspace::ListenMultiMonDisplayModeMessage() {
        msg_listener_->Listen<MsgClientMultiMonDisplayMode>([=, this](const MsgClientMultiMonDisplayMode& msg) {
            multi_display_mode_ = msg.mode_;
            context_->PostUITask([=]() {
                if (EMultiMonDisplayMode::kSeparate == multi_display_mode_) {
                    if (monitors_count_ > 1) {
                        setWindowTitle(origin_title_name_ + QStringLiteral(" (Desktop:%1)").arg(QString::number(1)));
                    }
                    else {
                        setWindowTitle(origin_title_name_);
                    }
                }
                else if (EMultiMonDisplayMode::kTab == multi_display_mode_) {
                    setWindowTitle(origin_title_name_);
                }
            });
            this->SendUpdateDesktopMessage();
        });
    }

    void Workspace::Init() {
        // plugins
        InitPluginsManager();

        auto beg = TimeUtil::GetCurrentTimestamp();

        InitTheme();

        sdk_ = ThunderSdk::Make(this->context_->GetMessageNotifier());
        sdk_->Init(this->params_, nullptr, DecoderRenderType::kFFmpegI420);

        // init game views
        InitGameView(this->params_);

        InitSampleWidget();

        //InitClipboardManager();
    
        // message listener
        InitListener();

        InitFileTrans();

        // connect to GammaRay Panel
        InitPanelClient();

        auto end = TimeUtil::GetCurrentTimestamp();
        LOGI("Init .3 used: {}ms", (end - beg));
    }

    void Workspace::InitListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        RegisterSdkMsgCallbacks();
        sdk_->Start();
        RegisterBaseListeners();
        RegisterControllerPanelListeners();
    }

    void Workspace::RegisterSdkMsgCallbacks() {
        BaseWorkspace::RegisterSdkMsgCallbacks();

        sdk_->SetOnVideoFrameDecodedCallback([=, this](const std::shared_ptr<RawImage>& image, const SdkCaptureMonitorInfo& info) {
            if (!has_frame_arrived_) {
                has_frame_arrived_ = true;
                UpdateVideoWidgetSize();
            }
            //LOGI("SdkCaptureMonitorInfo mon_index_: {}, w: {}, h: {}", info.mon_index_, image->img_width, image->img_height);
            if (EMultiMonDisplayMode::kTab == multi_display_mode_) {
                if (game_views_.size() > 0) {
                    if (game_views_[kMainGameViewIndex]) {
                        game_views_[kMainGameViewIndex]->RefreshCapturedMonitorInfo(info);
                        game_views_[kMainGameViewIndex]->RefreshImage(image);
                    }
                }
            }
            else if (EMultiMonDisplayMode::kSeparate == multi_display_mode_) {
                if (game_views_.size() > info.mon_index_) {
                    if (game_views_[info.mon_index_]) {
                        game_views_[info.mon_index_]->RefreshCapturedMonitorInfo(info);
                        game_views_[info.mon_index_]->RefreshImage(image);
                        if (!game_views_[info.mon_index_]->GetActiveStatus()) {
                            game_views_[info.mon_index_]->SetActiveStatus(true);
                            UpdateGameViewsStatus();
                        }
                    }
                }
            }
            context_->UpdateCapturingMonitorInfo(info);
        });
    }

    void Workspace::SendWindowsKey(unsigned long vk, bool down) {
        if (game_views_.size() > 0) {
            if (game_views_[kMainGameViewIndex]) {
                game_views_[kMainGameViewIndex]->SendKeyEvent(vk, down);
            }
        }
    }

    void Workspace::CalculateAspectRatio() {
        for (auto game_view : game_views_) {
            if (game_view && !game_view->isHidden()) {
                game_view->CalculateAspectRatio();
            }
        }
    }

    void Workspace::SwitchToFullWindow() {
        for (auto game_view : game_views_) {
            if (game_view && !game_view->isHidden()) {
                game_view->SwitchToFullWindow();
            }
        }
    }

    void Workspace::UpdateGameViewsStatus() {
        QList<QScreen*> screens = QGuiApplication::screens();
        if (EMultiMonDisplayMode::kTab ==  multi_display_mode_) {
            for (auto game_view : game_views_) {
                if (game_view->IsMainView()) {
                    if (full_screen_) {
                        WidgetSelectMonitor(this, screens);
                        this->showFullScreen();
                        game_view->showFullScreen();
                        tc::QWidgetHelper::SetBorderInFullScreen(this, true);
                        tc::QWidgetHelper::SetBorderInFullScreen(game_view, true);
                    }
                    else {
                        if (this->isMaximized()) {
                            this->showMaximized();
                            game_view->showMaximized();
                        }
                        else {
                            this->showNormal();
                            game_view->showNormal();
                        }
                    }
                }
                else {
                    game_view->hide();
                }
            }
        }
        else if (EMultiMonDisplayMode::kSeparate == multi_display_mode_) {
            for (auto game_view : game_views_) {
                if (game_view->GetActiveStatus()) {
                    if (full_screen_) {
                        if (game_view->IsMainView()) {
                            WidgetSelectMonitor(this, screens);
                            this->showFullScreen();
                            this->raise();
                            game_view->showFullScreen();
                            tc::QWidgetHelper::SetBorderInFullScreen(this, true);
                        }
                        else {
                            WidgetSelectMonitor(game_view, screens);
                            game_view->showFullScreen();
                        }
                        tc::QWidgetHelper::SetBorderInFullScreen(game_view, true);
                    }
                    else {
                        if (game_view->isMaximized()) {
                            game_view->showMaximized();
                            if (game_view->IsMainView()) {
                                this->showMaximized();
                            }
                        }
                        else {
                            game_view->showNormal();
                            if (game_view->IsMainView()) {
                                this->showNormal();
                            }
                        }
                    }
                }
                else {
                    game_view->hide();
                }
            }
        }
    }

    void Workspace::OnGetCaptureMonitorsCount(int monitors_count) {
        monitors_count_ = monitors_count;
        if (monitors_count <= 1) {
            setWindowTitle(origin_title_name_);
        }
        int min_temp = std::min(monitors_count, static_cast<int>(game_views_.size()));
        for (int index = 0; index < min_temp; ++index) {
            game_views_[index]->SetActiveStatus(true);
        }

        for (; min_temp < game_views_.size(); ++min_temp) {
            game_views_[min_temp]->SetActiveStatus(false);
        }
        UpdateGameViewsStatus();

        std::call_once(send_split_windows_flag_, [this]() {
            if (monitors_count_ > 1 && settings_->split_windows_) {
                this->SendSwitchMonitorMessage(kCaptureAllMonitorsSign);
            }
        });
    }

    void Workspace::OnGetCaptureMonitorName(std::string monitor_name) {
        LOGI("OnGetCaptureMonitorName monitor_name: {}", monitor_name);
        for (const auto& index_name : monitor_index_map_name_) {
            if (game_views_.size() > index_name.first) {
                if (game_views_[index_name.first]) {
                    game_views_[index_name.first]->SetMonitorName(index_name.second);
                }
            }
        }

        if (kCaptureAllMonitorsSign == monitor_name) {
            multi_display_mode_ = EMultiMonDisplayMode::kSeparate;
            if (monitors_count_ > 1) {
                setWindowTitle(origin_title_name_ + QStringLiteral(" (Desktop:%1)").arg(QString::number(1)));
            }
            
        }
        else {
            multi_display_mode_ = EMultiMonDisplayMode::kTab;
            if (game_views_[kMainGameViewIndex]) {
                game_views_[kMainGameViewIndex]->SetMonitorName(monitor_name);
            }
        }
    }

    void Workspace::InitGameView(const std::shared_ptr<ThunderSdkParams>& params) {
        this->resize(def_window_size_);
        for (int index = 0; index < kMaxGameViewCount; ++index) {
            GameView* game_view = nullptr;
            if (0 == index) {
                game_view = new GameView(context_, sdk_, params, this);    // main view
                game_view->resize(def_window_size_);
                game_view->show();
                game_view->SetMainView(true);
                setCentralWidget(game_view);
            }
            else {
                game_view = new GameView(context_, sdk_, params, nullptr); // extend view
                game_view->resize(def_window_size_);
                game_view->hide();
                game_view->SetMainView(false);
                game_view->installEventFilter(this);
                game_view->setWindowTitle(origin_title_name_ + QStringLiteral(" (Desktop:%1)").arg(QString::number(index + 1)));
            }
            game_view->SetMonitorIndex(index);
            game_views_.push_back(game_view);
        }
        QTimer::singleShot(1, this, [=, this]() {
            {
                QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
                int x = (screenGeometry.width() - this->width()) / 2;
                int y = (screenGeometry.height() - this->height()) / 2;
                this->move(x, y);
            }

            QPoint ws_pos = this->pos();
            const int x_offset = 80;
            const int y_offset = 40;
            const int start_x = ws_pos.x();
            const int start_y = ws_pos.y();
            int index = 0;
            for (auto game_view : game_views_) {
                if (!game_view) {
                    ++index;
                    continue;
                }
                if (game_view->IsMainView()) {
                    ++index;
                    continue;
                }
                game_view->move(start_x + x_offset * index, start_y + y_offset * index);
                ++index;
            }
        });
    }

    bool Workspace::eventFilter(QObject* watched, QEvent* event) {
        for (const auto game_view : game_views_) {
            if (!game_view) {
                continue;
            }
            
            if (game_view == watched) {
                switch (event->type())
                {
                    case QEvent::Close: {
                        close_event_occurred_widget_ = game_view;
                        event->ignore();
                        this->close();
                        return true;
                    }
                }
            }
        }
        return QMainWindow::eventFilter(watched, event);
    }

}