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
#include "transfer/file_transfer.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "ui/debug_panel.h"
#include "client/ct_clipboard_manager.h"
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



namespace tc
{

    Workspace::Workspace(const std::shared_ptr<ClientContext>& ctx, const ThunderSdkParams& params, QWidget* parent) : QMainWindow(parent) {
        this->context_ = ctx;
        this->settings_ = Settings::Instance();

        auto title_name = QMainWindow::tr("GammaRay Streamer") + "[" + settings_->stream_name_.c_str() + "]";
        auto notifier = this->context_->GetMessageNotifier();
        //(new MainWindowWrapper(notifier, this))->Setup(title_name);

        setAcceptDrops(true);
        QString app_dir = qApp->applicationDirPath();
        QString style_dir = app_dir + "/resources/";
        theme_ = new acss::QtAdvancedStylesheet(this);
        theme_->setStylesDirPath(style_dir);
        theme_->setOutputDirPath(app_dir + "/output");
        theme_->setCurrentStyle("qt_material");
        theme_->setCurrentTheme("light_blue");
        theme_->updateStylesheet();
        setWindowIcon(theme_->styleIcon());
        qApp->setStyleSheet(theme_->styleSheet());

        sdk_ = ThunderSdk::Make(ctx->GetMessageNotifier());
        sdk_->Init(params, nullptr, DecoderRenderType::kFFmpegI420);

        // ui
        auto root_layout = new NoMarginVLayout();
        auto root_widget = new QWidget(this);
        root_widget->setLayout(root_layout);

        video_widget_ = new OpenGLVideoWidget(ctx, sdk_, 0, RawImageFormat::kI420, this);
        //layout->addWidget(video_widget_);


        //dev test
        // init extend game view
        for (int index = 0; index < kMaxExtendGameViewCount; ++index) {
            auto game_view = new GameView(ctx, sdk_, params, nullptr);
            game_view->resize(800, 600);

            game_view->hide();
            game_view->monitor_index_ = index + 1; // beacuse extend game view index start from 1
            extend_game_views_.push_back(game_view);
        }
        setCentralWidget(root_widget);

        main_progress_ = new MainProgress(sdk_, context_, this);
        main_progress_->show();

        // button indicator
        int shadow_color = 0x999999;
        btn_indicator_ = new FloatButtonStateIndicator(ctx, this);
        btn_indicator_->hide();
        WidgetHelper::AddShadow(btn_indicator_, shadow_color);

        // debug panel
        debug_panel_ = new DebugPanel(context_, this);
        WidgetHelper::AddShadow(debug_panel_, 0x999999);
        debug_panel_->hide();

        // float controller
        float_controller_ = new FloatController(ctx, this);
        float_controller_->setFixedSize(50, 50);
        WidgetHelper::AddShadow(float_controller_, shadow_color);
        controller_panel_ = new FloatControllerPanel(ctx, this);
        WidgetHelper::AddShadow(controller_panel_, shadow_color);
        RegisterControllerPanelListeners();
        controller_panel_->hide();

        float_controller_->SetOnClickListener([=, this]() {
            QPoint point = float_controller_->mapToGlobal(QPoint(0, 0));
            point.setX(float_controller_->pos().x() + float_controller_->width() + 10);
            point.setY(float_controller_->pos().y());
            controller_panel_->move(point);
            if (controller_panel_->isHidden()) {
                if (!float_controller_->HasMoved()) {
                    controller_panel_->show();
                }
            } else {
                controller_panel_->Hide();
            }
        });
        float_controller_->SetOnMoveListener([=, this]() {
            if (!controller_panel_) {
                return;
            }
            controller_panel_->Hide();
        });

        // notification handle
        notification_handler_ = new FloatNotificationHandle(context_, this);
        notification_handler_->SetPixmap(":resources/image/ic_mail.svg");
        notification_handler_->SetOnClickListener([=, this](QWidget* w) {
            if (notification_panel_->isHidden()) {
                notification_panel_->show();
            } else {
                notification_panel_->hide();
            }
            UpdateNotificationHandlePosition();
        });

        // notification panel
        notification_panel_ = new NotificationPanel(ctx, this);
        notification_panel_->hide();

        // message listener
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();

        // sdk
        RegisterSdkMsgCallbacks();
        sdk_->Start();

        msg_listener_->Listen<ExitAppMessage>([=, this](const ExitAppMessage& msg) {
            context_->PostUITask([=, this]() {
                this->Exit();
            });
        });

        msg_listener_->Listen<ClipboardMessage>([=, this](const ClipboardMessage& msg) {
            this->SendClipboardMessage(msg.msg_);
        });

        msg_listener_->Listen<SwitchMonitorMessage>([=, this](const SwitchMonitorMessage& msg) {
            this->SendSwitchMonitorMessage(msg.name_);
        });

        msg_listener_->Listen<SwitchWorkModeMessage>([=, this](const SwitchWorkModeMessage& msg) {
            this->SendSwitchWorkModeMessage(msg.mode_);
        });

        msg_listener_->Listen<SwitchScaleModeMessage>([=, this](const SwitchScaleModeMessage& msg) {
            this->SwitchScaleMode(msg.mode_);
        });

        // step 1
        msg_listener_->Listen<SdkMsgNetworkConnected>([=, this](const SdkMsgNetworkConnected& msg) {
            this->SendSwitchWorkModeMessage(settings_->work_mode_);
            main_progress_->ResetProgress();
            main_progress_->StepForward();
            LOGI("Step: MsgNetworkConnected, at: {}", main_progress_->GetCurrentProgress());
        });

        msg_listener_->Listen<SdkMsgNetworkDisConnected>([=, this](const SdkMsgNetworkDisConnected& msg) {

        });

        // step 2
        msg_listener_->Listen<SdkMsgFirstConfigInfoCallback>([=, this](const SdkMsgFirstConfigInfoCallback& msg) {
            main_progress_->StepForward();
            LOGI("Step: MsgFirstConfigInfoCallback, at: {}", main_progress_->GetCurrentProgress());
        });

        // step 3
        msg_listener_->Listen<SdkMsgFirstVideoFrameDecoded>([=, this](const SdkMsgFirstVideoFrameDecoded& msg) {
            main_progress_->CompleteProgress();
            LOGI("Step: MsgFirstVideoFrameDecoded, at: {}", main_progress_->GetCurrentProgress());
        });

        msg_listener_->Listen<MsgChangeMonitorResolution>([=, this](const MsgChangeMonitorResolution& msg) {
            this->SendChangeMonitorResolutionMessage(msg);
        });

        msg_listener_->Listen<MsgCtrlAltDelete>([=, this](const MsgCtrlAltDelete& msg) {
            tc::Message m;
            m.set_type(tc::kReqCtrlAltDelete);
            m.set_device_id(settings_->device_id_);
            m.set_stream_id(settings_->stream_id_);
            auto _ = m.mutable_req_ctrl_alt_delete();
            sdk_->PostMediaMessage(m.SerializeAsString());
        });

        msg_listener_->Listen<MultiMonDisplayModeMessage>([=, this](const MultiMonDisplayModeMessage& msg) {
            multi_display_mode_ = msg.mode_;
        });

#ifdef TC_ENABLE_FILE_TRANSMISSION
        file_trans_interface_ = FileTransInterface::Make(sdk_);
#endif // TC_ENABLE_FILE_TRANSMISSION

        QTimer::singleShot(100, [=, this](){
            file_transfer_ = std::make_shared<FileTransferChannel>(context_);
            file_transfer_->Start();
        });

        // clipboard manager
        clipboard_mgr_ = std::make_shared<ClipboardManager>(context_);
        clipboard_mgr_->Monitor();
    }

    Workspace::~Workspace() {

    }

    void Workspace::RegisterSdkMsgCallbacks() {
        sdk_->SetOnVideoFrameDecodedCallback([=, this](const std::shared_ptr<RawImage>& image, const SdkCaptureMonitorInfo& info) {
            if (!has_frame_arrived_) {
                has_frame_arrived_ = true;
                UpdateVideoWidgetSize();
            }
            //LOGI("SdkCaptureMonitorInfo mon_index_: {}", info.mon_index_);
            if (EMultiMonDisplayMode::kTab == multi_display_mode_) {
                video_widget_->RefreshCapturedMonitorInfo(info);
                video_widget_->RefreshI420Image(image);
            }
            else if (EMultiMonDisplayMode::kSeparate == multi_display_mode_) {
                if (0 == info.mon_index_) {
                    video_widget_->RefreshCapturedMonitorInfo(info);
                    video_widget_->RefreshI420Image(image);
                }
                else {
                    if (extend_game_views_.size() >= info.mon_index_) {
                        if (extend_game_views_[info.mon_index_ - 1]) {
                            extend_game_views_[info.mon_index_ - 1]->RefreshCapturedMonitorInfo(info);
                            extend_game_views_[info.mon_index_ - 1]->RefreshI420Image(image);
                            if (!extend_game_views_[info.mon_index_ - 1]->GetActiveStatus()) {
                                extend_game_views_[info.mon_index_ - 1]->SetActiveStatus(true);
                                UpdateGameViewsStatus();
                            }
                        }
                    }
                }
            }
            context_->UpdateCapturingMonitorInfo(info);
        });

        sdk_->SetOnAudioFrameDecodedCallback([=, this](const std::shared_ptr<Data>& data, int samples, int channels, int bits) {
            //LOGI("data size: {}, samples: {}, channel: {}, bits: {}, audio on: {}", data->Size(), samples, channels, bits, settings_->IsAudioEnabled());
            if (!settings_->IsAudioEnabled()) {
                return;
            }
            if (!audio_player_) {
                audio_player_ = std::make_shared<AudioPlayer>();
                context_->PostUITask([=, this]() {
                    audio_player_->Init(samples, channels);
                });
                return;
            }
            audio_player_->Write(data);
        });

        sdk_->SetOnAudioSpectrumCallback([=](const tc::ServerAudioSpectrum& spectrum) {

        });

        sdk_->SetOnCursorInfoCallback([=, this](const CursorInfoSync& cursor_info) {
            this->UpdateLocalCursor(cursor_info.type());
        });

        sdk_->SetOnHeartBeatCallback([=, this](const OnHeartBeat& hb) {
            if (debug_panel_) {
                debug_panel_->UpdateOnHeartBeat(hb);
            }
            if (btn_indicator_) {
                btn_indicator_->UpdateOnHeartBeat(hb);
            }
        });

        sdk_->SetOnClipboardCallback([=, this](const ClipboardInfo& clipboard) {
            if (settings_->clipboard_on_ && clipboard_mgr_) {
                clipboard_mgr_->UpdateRemoteInfo(QString::fromStdString(clipboard.msg()));
            }
        });

        // todo: Send by Renderer every 1S
        sdk_->SetOnServerConfigurationCallback([=, this](const ServerConfiguration& config) {
            CaptureMonitorMessage msg;
            msg.capturing_monitor_name_ = config.capturing_monitor_name();
            LOGI("capturing monitor name: {}", msg.capturing_monitor_name_);
            int monitor_index = 0;
            for (const auto& item : config.monitors_info()) {
                std::string monitor_name = item.name();
                //LOGI("monitor name: {}", item.name());
                monitor_name_map_index_[monitor_name] = monitor_index;
                std::vector<CaptureMonitorMessage::Resolution> resolutions;
                for (auto& res : item.resolutions()) {
                    resolutions.push_back(CaptureMonitorMessage::Resolution {
                        .width_ = res.width(),
                        .height_ = res.height(),
                    });
                }
                msg.monitors_.push_back(CaptureMonitorMessage::CaptureMonitor {
                    .name_ = item.name(),
                    .resolutions_ = resolutions,
                });
                ++monitor_index;
            }
            context_->SendAppMessage(msg);

            int monitors_count = config.monitors_info().size();
            context_->PostUITask([=]() {
                OnGetCaptureMonitorsCount(monitors_count);
                OnGetCaptureMonitorName(config.capturing_monitor_name());
            });
            
        });

        sdk_->SetOnMonitorSwitchedCallback([=, this](const MonitorSwitched& ms) {
            context_->SendAppMessage(MonitorSwitchedMessage {
                .name_ = ms.name(),
            });
        });

        sdk_->SetOnRawMessageCallback([=, this](const std::shared_ptr<tc::Message>& msg) {
            if (file_trans_interface_) {
                file_trans_interface_->OnProtoMessage(msg);
            }
        });

        msg_listener_->Listen<SdkMsgChangeMonitorResolutionResult>([=, this](const SdkMsgChangeMonitorResolutionResult& msg) {
            context_->PostUITask([=, this]() {
                // to trigger re-layout
                if (msg.result) {
                    this->move(pos().x()+1, pos().y());
//                    auto box = SizedMessageBox::MakeInfoOkBox(tr("Info"), tr("Changing resolution success."));
//                    box->exec();

                    auto dlg = TcDialog::Make(tr("Tips"), tr("Changing resolution success."), nullptr);
                    dlg->SetOnDialogSureClicked([=, this]() {

                    });
                    dlg->show();


                } else {
//                    auto box = SizedMessageBox::MakeErrorOkBox(tr("Error"), tr("Changing resolution failed, please check your server's monitor."));
//                    box->exec();

                    auto dlg = TcDialog::Make(tr("Error"), tr("Changing resolution failed, please check your server's monitor."), nullptr);
                    dlg->SetOnDialogSureClicked([=, this]() {

                    });
                    dlg->show();
                }
            });

        });

        msg_listener_->Listen<SdkMsgTimer1000>([=, this](const SdkMsgTimer1000& msg) {
            force_update_cursor_ = true;
        });
    }

    void Workspace::changeEvent(QEvent* event) {
        is_window_active_ = isActiveWindow() && !(windowState() & Qt::WindowMinimized);
        qDebug() << "window state: " << is_window_active_;
    }

    bool Workspace::IsActiveNow() const {
        return is_window_active_;
    }

    void Workspace::closeEvent(QCloseEvent *event) {
//        auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Stop"), tr("Do you want to stop controlling of remote PC ?"));
//        if (msg_box->exec() == 0) {
//            Exit();
//        } else {
//            event->ignore();
//        }

        event->ignore();
        auto dg = TcDialog::Make(tr("Stop"), tr("Do you want to stop controlling of remote PC ?"), nullptr);
        dg->SetOnDialogSureClicked([=, this]() {
            Exit();
        });
        dg->Show();
    }

    void Workspace::dragEnterEvent(QDragEnterEvent *event) {
        event->accept();
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        }
    }

    void Workspace::dragMoveEvent(QDragMoveEvent *event) {
        event->accept();
    }

    void Workspace::dropEvent(QDropEvent *event) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.isEmpty()) {
            return;
        }
        std::vector<QString> files;
        for (const auto& url : urls) {
            files.push_back(url.toLocalFile());
        }
        if (file_transfer_) {
            file_transfer_->SendFiles(files);
        }
    }

    void Workspace::SendWindowsKey(unsigned long vk, bool down) {
        if (video_widget_) {
           video_widget_->SendKeyEvent(vk, down);
        }
    }

    void Workspace::resizeEvent(QResizeEvent *event) {
        main_progress_->setGeometry(0, title_bar_height_, event->size().width(), event->size().height());
        UpdateNotificationHandlePosition();
        UpdateDebugPanelPosition();
        UpdateVideoWidgetSize();
        UpdateFloatButtonIndicatorPosition();
        float_controller_->ReCalculatePosition();
    }

    void Workspace::UpdateNotificationHandlePosition() {
        int notification_panel_width = 0;
        int offset_border = 8;
        int handle_offset = 0;
        if (!notification_panel_->isHidden()) {
            notification_panel_width = notification_panel_->width();
            handle_offset = offset_border;
        }
        notification_panel_->setGeometry(this->width()-notification_panel_->width() - offset_border, offset_border, notification_panel_->width(), this->height() - 2*offset_border);
        notification_handler_->setGeometry(this->width()-notification_handler_->width()/2 - notification_panel_width-handle_offset, 100, notification_handler_->width(), notification_handler_->height());
    }

    void Workspace::UpdateFloatButtonIndicatorPosition() {
        btn_indicator_->setGeometry(0, 0, btn_indicator_->width(), btn_indicator_->height());
    }

    void Workspace::UpdateLocalCursor(uint32_t type) {
        if (cursor_type_ == type && !force_update_cursor_) {
            return;
        }
        force_update_cursor_ = false;
        cursor_type_ = type;
        context_->PostUITask([=, this]() {
            if (cursor_type_ == CursorInfoSync::kIdcArrow) {
                this->setCursor(Qt::ArrowCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcIBeam) {
                this->setCursor(Qt::IBeamCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcWait) {
                this->setCursor(Qt::WaitCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcCross) {
                this->setCursor(Qt::CrossCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcUpArrow) {
                this->setCursor(Qt::UpArrowCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcSize) {
                this->setCursor(Qt::SizeAllCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcIcon) {
                this->setCursor(Qt::ArrowCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcSizeNWSE) {
                this->setCursor(Qt::SizeFDiagCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcSizeNESW) {
                this->setCursor(Qt::SizeBDiagCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcSizeWE) {
                this->setCursor(Qt::SizeHorCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcSizeNS)  {
                this->setCursor(Qt::SizeVerCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcSizeAll) {
                this->setCursor(Qt::SizeAllCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcHand) {
                this->setCursor(Qt::PointingHandCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcPin) {
                this->setCursor(Qt::PointingHandCursor);
            } else if (cursor_type_ == CursorInfoSync::kIdcHelp) {
                this->setCursor(Qt::WhatsThisCursor);
            }
        });
    }

    void Workspace::RegisterControllerPanelListeners() {
        controller_panel_->SetOnDebugListener([=, this](QWidget* w) {
            controller_panel_->Hide();
            debug_panel_->setHidden(!debug_panel_->isHidden());
        });

        controller_panel_->SetOnFileTransListener([=, this](QWidget* w) {
            if (!file_trans_interface_) {
                return;
            }
            context_->PostUITask([=, this]() {
                controller_panel_->Hide();
                file_trans_interface_->OnClickedFileTrans();
            });
        });
    }

    void Workspace::UpdateDebugPanelPosition() {
        int offset = 120;
        debug_panel_->resize(this->width()-offset, this->height()-offset);
        debug_panel_->move(offset/2, offset/2);
    }

    void Workspace::SendClipboardMessage(const std::string& msg) {
        if (!sdk_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kClipboardInfo);
        m.set_device_id(settings_->device_id_);
        m.set_stream_id(settings_->stream_id_);
        m.mutable_clipboard_info()->set_msg(msg);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void Workspace::SendSwitchMonitorMessage(const std::string& name) {
        if (!sdk_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kSwitchMonitor);
        m.set_device_id(settings_->device_id_);
        m.set_stream_id(settings_->stream_id_);
        m.mutable_switch_monitor()->set_name(name);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void Workspace::SendSwitchWorkModeMessage(SwitchWorkMode::WorkMode mode) {
        if (!sdk_) {
            return;
        }
        settings_->SetWorkMode(mode);
        tc::Message m;
        m.set_type(tc::kSwitchWorkMode);
        m.set_device_id(settings_->device_id_);
        m.set_stream_id(settings_->stream_id_);
        auto wm = m.mutable_work_mode();
        wm->set_mode(mode);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void Workspace::SwitchScaleMode(const tc::ScaleMode& mode) {
        settings_->SetScaleMode(mode);
        if (!video_widget_) {return;}
        if (mode == ScaleMode::kFullWindow) {
            SwitchToFullWindow();
        } else if (mode == ScaleMode::kKeepAspectRatio) {
            CalculateAspectRatio();
        }
    }

    void Workspace::CalculateAspectRatio() {
        auto vw = video_widget_->GetCapturingMonitorWidth();
        auto vh = video_widget_->GetCapturingMonitorHeight();
        // no frame, fill the window
        if (vw <= 0 || vh <= 0) {
            video_widget_->setGeometry(0, title_bar_height_, this->width(), this->height());
            return;
        }

        auto target_title_bar_height = this->isFullScreen() ? 0 : title_bar_height_;

        int available_height = this->height() - target_title_bar_height;
        float h_ratio = vw * 1.0f / this->width();
        float v_ratio = vh * 1.0f / available_height;//this->height();
        int target_width = 0;
        int target_height = 0;

        float widget_ratio = this->width() * 1.0f / available_height;
        float frame_ratio = vw * 1.0f / vh;
        if (widget_ratio > frame_ratio) {
            // along to height
            target_height = available_height;
            target_width = vw * (available_height*1.0f/vh);
        }
        else {
            // along to width
            target_width = this->width();
            target_height = vh * (this->width()*1.0f/vw);
        }

//        if (h_ratio > v_ratio) {
//            // use width
//            target_width = this->width();
//            target_height = vh * (this->width()*1.0f/vw);
//            LOGI("H > V");
//        } else {
//            LOGI("H < V");
//            // use height
//            //target_height = this->height();
//            //target_width = vw * (this->height()*1.0f/vh);
//            target_height = available_height;
//            target_width = vw * (available_height*1.0f/vh);
//        }

//        target_height = available_height;
//        target_width = vw * (available_height*1.0f/vh);

        //video_widget_->setGeometry((this->width()-target_width)/2, (this->height()-target_height)/2, target_width, target_height);
        video_widget_->setGeometry((this->width()-target_width)/2, (available_height-target_height)/2 + target_title_bar_height, target_width, target_height);
    }

    void Workspace::SwitchToFullWindow() {
        auto target_title_bar_height = this->isFullScreen() ? 0 : title_bar_height_;
        video_widget_->setGeometry(0, target_title_bar_height, this->width(), this->height() - target_title_bar_height);
    }

    void Workspace::SendChangeMonitorResolutionMessage(const MsgChangeMonitorResolution& msg) {
        if (!sdk_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kChangeMonitorResolution);
        m.set_device_id(settings_->device_id_);
        m.set_stream_id(settings_->stream_id_);
        auto cmr = m.mutable_change_monitor_resolution();
        cmr->set_monitor_name(msg.monitor_name_);
        cmr->set_target_width(msg.width_);
        cmr->set_target_height(msg.height_);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void Workspace::UpdateVideoWidgetSize() {
        context_->PostUITask([=, this]() {
            auto scale_mode = settings_->scale_mode_;
            SwitchScaleMode(scale_mode);
        });
    }

    void Workspace::Exit() {
        std::thread t([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            ProcessUtil::KillProcess(QApplication::applicationPid());
        });
        t.detach();
        if (sdk_) {
            sdk_->Exit();
            sdk_ = nullptr;
        }
        if (context_) {
            context_->Exit();
            context_ = nullptr;
        }
        if (file_transfer_) {
            file_transfer_->Exit();
        }
        if (file_trans_interface_) {
            file_trans_interface_->Exit();
        }
        ProcessUtil::KillProcess(QApplication::applicationPid());
        qApp->exit(0);
    }

    void Workspace::UpdateGameViewsStatus() {
        if (EMultiMonDisplayMode::kTab ==  multi_display_mode_) {
            for (auto game_view : extend_game_views_) {
                game_view->hide();
            }
        }
        else if (EMultiMonDisplayMode::kSeparate == multi_display_mode_) {
            for (auto game_view : extend_game_views_) {
                if (game_view->active_) {
                    game_view->show();
                }
                else {
                    game_view->hide();
                }
            }
        }
    }

    void Workspace::OnGetCaptureMonitorsCount(int monitors_count) {
        int min_temp = std::min(monitors_count - 1, static_cast<int>(extend_game_views_.size()));
        for (int index = 1; index <= min_temp; ++index) {
            extend_game_views_[index - 1]->SetActiveStatus(true);
        }

        for (; min_temp < extend_game_views_.size(); ++min_temp) {
            extend_game_views_[min_temp]->SetActiveStatus(false);
        }
        UpdateGameViewsStatus();
    }

    void Workspace::OnGetCaptureMonitorName(std::string monitor_name) {
        if (kCaptureAllMonitorsSign == monitor_name) {
            multi_display_mode_ = EMultiMonDisplayMode::kSeparate;
        }
        else {
            multi_display_mode_ = EMultiMonDisplayMode::kTab;
        }
    }
}