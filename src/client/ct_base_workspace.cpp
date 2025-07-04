//
// Created by RGAA on 2023-12-27.
//
#include "client/ct_base_workspace.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTimer>
#include <dwmapi.h>
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
#include "tc_dialog.h"
#include "tc_label.h"
#include "ct_game_view.h"
#include "ct_const_def.h"
#include "tc_common_new/file.h"
#include "tc_common_new/qwidget_helper.h"
#include "ui/retry_conn_dialog.h"
#include "network/ct_panel_client.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/time_util.h"
#include "plugins/ct_plugin_manager.h"
#include "plugins/ct_app_events.h"
#include "plugin_interface/ct_plugin_interface.h"
#include "plugins/media_record/media_record_plugin.h"
#include "plugin_interface/ct_media_record_plugin_interface.h"
#include "tc_qt_widget/notify/notifymanager.h"
#include "tc_relay_client/relay_api.h"

namespace tc
{

    BaseWorkspace::BaseWorkspace(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent) : QMainWindow(parent) {
        this->context_ = ctx;
        this->context_->InitNotifyManager(this);
        this->settings_ = Settings::Instance();
        this->params_ = params;
        cursor_ = QCursor(Qt::ArrowCursor);
        retry_conn_dialog_ = std::make_shared<RetryConnDialog>(tcTr("id_warning"));
    }

    void BaseWorkspace::Init() {
        // plugins
        InitPluginsManager();

        auto beg = TimeUtil::GetCurrentTimestamp();

        InitTheme();

        sdk_ = ThunderSdk::Make(this->context_->GetMessageNotifier());
        sdk_->Init(this->params_, nullptr, DecoderRenderType::kFFmpegI420);
        
        // init game views
        InitGameView(this->params_);

        InitSampleWidget();

        // message listener
        InitListener();
        // connect to GammaRay Panel
        InitPanelClient();

        auto end = TimeUtil::GetCurrentTimestamp();
        LOGI("Init .3 used: {}ms", (end - beg));
    }

    void BaseWorkspace::InitPluginsManager() {
        plugin_manager_ = ClientPluginManager::Make(shared_from_this());
        context_->SetPluginManager(plugin_manager_);
        plugin_manager_->LoadAllPlugins();
        plugin_manager_->RegisterPluginEventsCallback();
        plugin_manager_->DumpPluginInfo();
    }

    void BaseWorkspace::InitSampleWidget() {
        main_progress_ = new MainProgress(sdk_, context_, this);
        main_progress_->show();

        // button indicator
        int shadow_color = 0x999999;
        btn_indicator_ = new FloatButtonStateIndicator(this->context_, this);
        btn_indicator_->hide();
        WidgetHelper::AddShadow(btn_indicator_, shadow_color);

        // debug panel
        st_panel_ = new CtStatisticsPanel(context_, nullptr);
        st_panel_->resize(def_window_size_);
        st_panel_->hide();
    }

    void BaseWorkspace::InitTheme() {
        WidgetHelper::SetTitleBarColor(this);

        origin_title_name_ = tcTr("id_gr_client") + "[" + this->params_->stream_name_.c_str() + "]";
        setWindowTitle(origin_title_name_);
        auto notifier = this->context_->GetMessageNotifier();

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
    }

    void BaseWorkspace::InitListener() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        RegisterSdkMsgCallbacks();
        sdk_->Start();
        RegisterBaseListeners();
        RegisterControllerPanelListeners();
    }

    void BaseWorkspace::InitPanelClient() {
        panel_client_ = std::make_shared<CtPanelClient>(context_);
        panel_client_->Start();
    }

    void BaseWorkspace::RegisterBaseListeners() {
        msg_listener_->Listen<MsgClientExitApp>([=, this](const MsgClientExitApp& msg) {
            context_->PostDelayUITask([=, this]() {
                this->ExitClientWithDialog();
            }, 10);
        });

        msg_listener_->Listen<MsgClientClipboard>([=, this](const MsgClientClipboard& msg) {
            this->SendClipboardMessage(msg);
        });

        msg_listener_->Listen<MsgClientSwitchMonitor>([=, this](const MsgClientSwitchMonitor& msg) {
            this->SendSwitchMonitorMessage(msg.name_);
            this->SendUpdateDesktopMessage();
            context_->PostTask([=, this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                this->SendUpdateDesktopMessage();
            });
        });

        msg_listener_->Listen<MsgClientSwitchWorkMode>([=, this](const MsgClientSwitchWorkMode& msg) {
            //this->SendSwitchWorkModeMessage(msg.mode_);
        });

        msg_listener_->Listen<MsgClientSwitchScaleMode>([=, this](const MsgClientSwitchScaleMode& msg) {
            this->SwitchScaleMode(msg.mode_);
        });

        msg_listener_->Listen<MsgClientSwitchFullColor>([=, this](const MsgClientSwitchFullColor& msg) {
            this->SendSwitchFullColorMessage(msg.enable_);
        });

        // step 1
        msg_listener_->Listen<SdkMsgNetworkConnected>([=, this](const SdkMsgNetworkConnected& msg) {
            //this->SendSwitchWorkModeMessage(settings_->work_mode_);
            this->SendUpdateDesktopMessage();
            main_progress_->ResetProgress();
            main_progress_->StepForward();
            LOGI("Step: MsgNetworkConnected, at: {}", main_progress_->GetCurrentProgress());

            // dismiss dialog
            DismissConnectingDialog();
        });

        // reconnection
        // relay mode now, already connected
        msg_listener_->Listen<SdkMsgReconnect>([=, this](const SdkMsgReconnect& msg) {
            main_progress_->ResetProgress();
            main_progress_->StepForward();
            LOGI("Step: SdkMsgReconnect, at: {}", main_progress_->GetCurrentProgress());
        });

        msg_listener_->Listen<SdkMsgNetworkDisConnected>([=, this](const SdkMsgNetworkDisConnected& msg) {
            if (remote_force_closed_) {
                return;
            }
            context_->PostUITask([=, this]() {
                if (retry_conn_dialog_->isHidden()) {
                    WidgetHelper::SetTitleBarColor((QWidget*)(retry_conn_dialog_.get()));
                    retry_conn_dialog_->Exec();
                }
            });
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

            DismissConnectingDialog();
        });

        msg_listener_->Listen<MsgClientChangeMonitorResolution>([=, this](const MsgClientChangeMonitorResolution& msg) {
            this->SendChangeMonitorResolutionMessage(msg);
        });

        msg_listener_->Listen<MsgClientCtrlAltDelete>([=, this](const MsgClientCtrlAltDelete& msg) {
            tc::Message m;
            m.set_type(tc::kReqCtrlAltDelete);
            m.set_device_id(settings_->device_id_);
            m.set_stream_id(settings_->stream_id_);
            auto _ = m.mutable_req_ctrl_alt_delete();
            sdk_->PostMediaMessage(m.SerializeAsString());
        });

        msg_listener_->Listen<MsgClientHardUpdateDesktop>([=, this](const MsgClientHardUpdateDesktop& msg) {
            this->SendHardUpdateDesktopMessage();
        });
    }

    BaseWorkspace::~BaseWorkspace() {

    }

    void BaseWorkspace::RegisterSdkMsgCallbacks() {
//        sdk_->SetOnVideoFrameDecodedCallback([=, this](const std::shared_ptr<RawImage>& image, const SdkCaptureMonitorInfo& info) {
//            if (!has_frame_arrived_) {
//                has_frame_arrived_ = true;
//                UpdateVideoWidgetSize();
//            }
//
//            if (game_view_) {
//                game_view_->RefreshCapturedMonitorInfo(info);
//                game_view_->RefreshImage(image);
//            }
//
//            context_->UpdateCapturingMonitorInfo(info);
//        });

        // save pcm file , use ffplay.exe -ar 48000 -ac 2 -f s16le -i .\audio_48000_2.pcm
        sdk_->SetOnAudioFrameDecodedCallback([=, this](const std::shared_ptr<Data>& data, int samples, int channels, int bits) {
            if (!settings_->IsAudioEnabled() || remote_force_closed_) {
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

        sdk_->SetOnAudioSpectrumCallback([=](const tc::RendererAudioSpectrum& spectrum) {

        });

        sdk_->SetOnCursorInfoCallback([=, this](const CursorInfoSync& cursor_info) {
            // remote cursor's bitmap
            std::string bitmap_data = cursor_info.bitmap();
            bool change = false;
            if (!bitmap_data.empty()) {
                cursor_bitmap_data_ = bitmap_data;
                if (last_cursor_bitmap_data_ != bitmap_data) {
                    last_cursor_bitmap_data_ = bitmap_data;
                    change = true;
                }
            }

            if (change) {
                QImage image((uchar*)cursor_bitmap_data_.data(), cursor_info.width(), cursor_info.height(), QImage::Format_RGBA8888);
                QPixmap pixmap = QPixmap::fromImage(image);
                QCursor cursor(pixmap, cursor_info.hotspot_x(), cursor_info.hotspot_y());
                cursor_ = cursor;
                this->UpdateLocalCursor();
            }
        });

        sdk_->SetOnHeartBeatCallback([=, this](const OnHeartBeat& hb) {
            if (st_panel_) {
                st_panel_->UpdateOnHeartBeat(hb);
            }
            if (btn_indicator_) {
                btn_indicator_->UpdateOnHeartBeat(hb);
            }
        });

        sdk_->SetOnClipboardCallback([=, this](std::shared_ptr<tc::Message> msg) {
            // See: RawMessageCallback
        });

        sdk_->SetOnServerConfigurationCallback([=, this](const ServerConfiguration& config) {
            monitor_index_map_name_.clear();

            MsgClientCaptureMonitor msg;
            msg.capturing_monitor_name_ = config.capturing_monitor_name();
            LOGI("capturing monitor name: {}", msg.capturing_monitor_name_);
            int monitor_index = 0;
            for (const auto& item : config.monitors_info()) {
                std::string monitor_name = item.name();
                //LOGI("monitor name: {}", item.name());
                monitor_index_map_name_[monitor_index] = monitor_name;
                std::vector<MsgClientCaptureMonitor::Resolution> resolutions;
                for (auto& res : item.resolutions()) {
                    resolutions.push_back(MsgClientCaptureMonitor::Resolution {
                        .width_ = res.width(),
                        .height_ = res.height(),
                    });
                }
                msg.monitors_.push_back(MsgClientCaptureMonitor::CaptureMonitor {
                    .name_ = item.name(),
                    .resolutions_ = resolutions,
                });
                ++monitor_index;
            }
            LOGI("capturing monitors count: {}", monitor_index);

            //
            settings_->is_render_file_transfer_enabled_ = config.file_transfer_enabled();
            settings_->is_render_audio_capture_enabled_ = config.audio_enabled();
            settings_->is_render_be_operated_by_mk_ = config.can_be_operated();

            context_->SendAppMessage(msg);

            int fps = config.fps();
            settings_->SetFps(fps);
            LOGI("capturing fps: {}", fps);
            context_->SendAppMessage(MsgClientFloatControllerPanelUpdate{
                .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kFps
            });

            int monitors_count = config.monitors_info().size();
            context_->PostUITask([=, this]() {
                OnGetCaptureMonitorName(config.capturing_monitor_name());
                OnGetCaptureMonitorsCount(monitors_count);
            });
        });

        sdk_->SetOnMonitorSwitchedCallback([=, this](const MonitorSwitched& ms) {
            context_->SendAppMessage(MsgClientMonitorSwitched {
                .name_ = ms.name(),
                .index_ = ms.index()
            });
        });

        sdk_->SetOnRawMessageCallback([=, this](const std::shared_ptr<tc::Message>& msg) {
            if (remote_force_closed_) {
                return;
            }
            plugin_manager_->VisitAllPlugins([=, this](ClientPluginInterface* plugin) {
                plugin->OnMessage(msg);
            });

            // parse it
            this->ProcessNetworkMessage(msg);
        });

        media_record_plugin_ = plugin_manager_->GetMediaRecordPlugin();
        if (!media_record_plugin_) {
            LOGE("media_record_plugin_ is nullptr!!!");
        }
        
        msg_listener_->Listen<SdkMsgChangeMonitorResolutionResult>([=, this](const SdkMsgChangeMonitorResolutionResult& msg) {
            context_->PostUITask([=, this]() {
                // to trigger re-layout
                if (msg.result) {
                    this->move(pos().x()+1, pos().y());

                    TcDialog dialog(tcTr("id_tips"), tcTr("id_change_resolution_success"), this);
                    dialog.exec();

                } else {
                    TcDialog dialog(tcTr("id_tips"), tcTr("id_change_resolution_failed"), this);
                    dialog.exec();
                }
            });

        });

        msg_listener_->Listen<SdkMsgTimer1000>([=, this](const SdkMsgTimer1000& msg) {
            force_update_cursor_ = true;

            plugin_manager_->VisitAllPlugins([=, this](ClientPluginInterface* plugin) {
                // callback
                plugin->On1Second();

                // sync settings
                plugin->SyncClientPluginSettings(ClientPluginSettings {
                    .clipboard_enabled_ = settings_->clipboard_on_,
                });
            });
        });

        msg_listener_->Listen<MsgClientFullscreen>([=, this](const MsgClientFullscreen& msg) {
            context_->PostUITask([=, this]() {
                full_screen_ = true;
                this->UpdateGameViewsStatus();
            });
        });

        msg_listener_->Listen<MsgClientExitFullscreen>([=, this](const MsgClientExitFullscreen& msg) {
            context_->PostUITask([=, this]() {
                full_screen_ = false;
                this->UpdateGameViewsStatus();
            });
        });

        msg_listener_->Listen<MsgClientMediaRecord>([=, this](const MsgClientMediaRecord& msg) {
            if (!sdk_) {
                return;
            }
            if (!media_record_plugin_) {
                return;
            }
            tc::Message m;
            m.set_device_id(settings_->device_id_);
            m.set_stream_id(settings_->stream_id_);
            bool res = context_->GetRecording();
            if (res) {
                LOGI("StartRecord");
                m.set_type(tc::kStartMediaRecordClientSide);
                media_record_plugin_->StartRecord();
            }
            else {
                LOGI("EndRecord");
                m.set_type(tc::kStopMediaRecordClientSide);
                media_record_plugin_->EndRecord();
            }
            sdk_->PostMediaMessage(m.SerializeAsString());
        });

        msg_listener_->Listen<MsgClientModifyFps>([=, this](const MsgClientModifyFps& msg) {
            context_->PostUITask([=, this]() {
                this->SendModifyFpsMessage();
            });
        });

        msg_listener_->Listen<MsgClientMouseEnterView>([=, this](const MsgClientMouseEnterView& msg) {
            context_->PostUITask([=, this]() {
                this->UpdateLocalCursor();
            });
        });

        msg_listener_->Listen<MsgClientMouseLeaveView>([=, this](const MsgClientMouseLeaveView& msg) {
            context_->PostUITask([=, this]() {
                this->UpdateLocalCursor();
            });
        });

        // relay error callback
        msg_listener_->Listen<SdkMsgRelayError>([=, this](const SdkMsgRelayError& msg) {
            if (remote_force_closed_) {
                return;
            }
            context_->PostUITask([=, this]() {
                TcDialog dialog(tcTr("id_error"), msg.msg_.c_str());
                dialog.exec();
            });
        });

        // remote device offline
        msg_listener_->Listen<SdkMsgRelayRemoteDeviceOffline>([=, this](const SdkMsgRelayRemoteDeviceOffline& msg) {
            if (remote_force_closed_) {
                return;
            }
            context_->PostUITask([=, this]() {
                TcDialog dialog(tcTr("id_error"), tcTr("id_remote_device_offline"));
                if (dialog.exec() == kDoneOk) {
                    context_->PostTask([=, this]() {
                        ReconnectInRelayMode();
                    });
                }
            });
        });
    }

    void BaseWorkspace::changeEvent(QEvent* event) {
        is_window_active_ = isActiveWindow() && !(windowState() & Qt::WindowMinimized);
        qDebug() << "window state: " << is_window_active_;
    }

    bool BaseWorkspace::IsActiveNow() const {
        return is_window_active_;
    }

    void BaseWorkspace::closeEvent(QCloseEvent *event) {
        this->raise();             
        this->activateWindow();    
        this->showNormal();
        if (!close_event_occurred_widget_) {
            close_event_occurred_widget_ = this;
        }
        event->ignore();

        ExitClientWithDialog();
        close_event_occurred_widget_ = nullptr;
    }

    void BaseWorkspace::ExitClientWithDialog() {
        QString msg = tcTr("id_exit_client");
        if (auto plugin = plugin_manager_->GetFileTransferPlugin(); plugin) {
             if (plugin->HasProcessingTasks()) {
                msg = tcTr("id_file_transfer_busy") + msg;
            }
        }
        TcDialog dialog(tcTr("id_exit"), msg, this);
        if (dialog.exec() == kDoneOk) {
            if (media_record_plugin_) {
                media_record_plugin_->EndRecord();
            }
            Exit();
        }
    }

    void BaseWorkspace::dragEnterEvent(QDragEnterEvent *event) {
        event->accept();
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        }
    }

    void BaseWorkspace::dragMoveEvent(QDragMoveEvent *event) {
        event->accept();
    }

    void BaseWorkspace::dropEvent(QDropEvent *event) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.isEmpty()) {
            return;
        }
        std::vector<QString> files;
        for (const auto& url : urls) {
            files.push_back(url.toLocalFile());
        }
    }

    void BaseWorkspace::SendWindowsKey(unsigned long vk, bool down) {
        if (game_view_) {
            game_view_->SendKeyEvent(vk, down);
        }
    }

    void BaseWorkspace::resizeEvent(QResizeEvent *event) {
        main_progress_->setGeometry(0, title_bar_height_, event->size().width(), event->size().height());
        UpdateDebugPanelPosition();
        UpdateVideoWidgetSize();
        UpdateFloatButtonIndicatorPosition();
    }

    void BaseWorkspace::UpdateFloatButtonIndicatorPosition() {
        btn_indicator_->setGeometry(0, 0, btn_indicator_->width(), btn_indicator_->height());
    }

    Qt::CursorShape BaseWorkspace::ToQCursorShape(uint32_t cursor_type) {
        if (cursor_type == CursorInfoSync::kIdcArrow) {
            return Qt::ArrowCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcIBeam) {
            return Qt::IBeamCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcWait) {
            return Qt::WaitCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcCross) {
            return Qt::CrossCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcUpArrow) {
            return Qt::UpArrowCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcSize) {
            return Qt::SizeAllCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcIcon) {
            return Qt::BitmapCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcSizeNWSE) {
            return Qt::SizeFDiagCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcSizeNESW) {
            return Qt::SizeBDiagCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcSizeWE) {
            return Qt::SizeHorCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcSizeNS) {
            return Qt::SizeVerCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcSizeAll) {
            return Qt::SizeAllCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcHand) {
            return Qt::PointingHandCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcPin) {
            return Qt::PointingHandCursor;
        }
        else if (cursor_type == CursorInfoSync::kIdcHelp) {
            return Qt::WhatsThisCursor;
        }
        else {
            return Qt::BitmapCursor;
        }
    }

    void BaseWorkspace::UpdateLocalCursor() {
        context_->PostUITask([=, this]() {
            if (GameView::s_mouse_in_) {
                if (QApplication::overrideCursor()) {
                    QApplication::changeOverrideCursor(cursor_);
                }
                else {
                    QApplication::setOverrideCursor(cursor_);
                }
            }
            else {
                QApplication::restoreOverrideCursor();
            }
        });
    }

    void BaseWorkspace::RegisterControllerPanelListeners() {
        msg_listener_->Listen<MsgClientOpenFiletrans>([=, this](const MsgClientOpenFiletrans& msg) {
            context_->PostUITask([=, this]() {
                if (auto plugin = plugin_manager_->GetFileTransferPlugin(); plugin) {
                    plugin->ShowRootWidget();
                }
            });
        });

        msg_listener_->Listen<MsgClientOpenDebugPanel>([=, this](const MsgClientOpenDebugPanel& msg) {
            context_->PostUITask([=, this]() {
                st_panel_->setHidden(false);
            });
        });
    }

    void BaseWorkspace::UpdateDebugPanelPosition() {

    }

    void BaseWorkspace::SendClipboardMessage(const MsgClientClipboard& msg) {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kClipboardInfo);
        m.set_device_id(settings_->device_id_);
        m.set_stream_id(settings_->stream_id_);
        auto sub = m.mutable_clipboard_info();
        sub->set_type((ClipboardType)msg.type_);
        if (msg.type_ == ClipboardType::kClipboardText) {
            sub->set_msg(msg.msg_);
        }
        else if (msg.type_ == ClipboardType::kClipboardFiles) {
            for (const auto& file : msg.files_) {
                auto pf = sub->mutable_files()->Add();
                pf->set_file_name(file.file_name());
                pf->set_full_path(file.full_path());
                pf->set_ref_path(file.ref_path());
                pf->set_total_size(file.total_size());
            }
        }
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void BaseWorkspace::SendSwitchMonitorMessage(const std::string& name) {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kSwitchMonitor);
        m.set_device_id(settings_->device_id_);
        m.set_stream_id(settings_->stream_id_);
        m.mutable_switch_monitor()->set_name(name);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void BaseWorkspace::SendUpdateDesktopMessage() {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kUpdateDesktop);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void BaseWorkspace::SendModifyFpsMessage() {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        int fps = settings_->fps_;
        tc::Message m;
        m.set_type(tc::kModifyFps);
        auto mf = m.mutable_modify_fps();
        mf->set_fps(fps);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void BaseWorkspace::SendHardUpdateDesktopMessage() {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kHardUpdateDesktop);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void BaseWorkspace::SendSwitchWorkModeMessage(SwitchWorkMode::WorkMode mode) {
#if 0 // Deprecated !!
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
#endif
    }

    void BaseWorkspace::SendSwitchFullColorMessage(bool enable) {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kSwitchFullColorMode);
        m.set_device_id(settings_->device_id_);
        m.set_stream_id(settings_->stream_id_);
        auto wm = m.mutable_switch_full_color_mode();
        wm->set_enable(enable);
        sdk_->PostMediaMessage(m.SerializeAsString());
    }

    void BaseWorkspace::SwitchScaleMode(const tc::ScaleMode& mode) {
        settings_->SetScaleMode(mode);
        if (mode == ScaleMode::kFullWindow) {
            SwitchToFullWindow();
        }
        else if (mode == ScaleMode::kKeepAspectRatio) {
            CalculateAspectRatio();
        }
    }

    void BaseWorkspace::CalculateAspectRatio() {
        if (game_view_) {
            game_view_->CalculateAspectRatio();
        }
    }

    void BaseWorkspace::SwitchToFullWindow() {
        if (game_view_) {
            game_view_->SwitchToFullWindow();
        }
    }

    void BaseWorkspace::SendChangeMonitorResolutionMessage(const MsgClientChangeMonitorResolution& msg) {
        if (!sdk_ || remote_force_closed_) {
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

    void BaseWorkspace::UpdateVideoWidgetSize() {
        context_->PostUITask([=, this]() {
            auto scale_mode = settings_->scale_mode_;
            LOGI("UpdateVideoWidgetSize scale_mode: {}", (int)scale_mode);
            SwitchScaleMode(scale_mode);
        });
    }

    void BaseWorkspace::Exit() {
        std::thread t([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            ProcessUtil::KillProcess(QApplication::applicationPid());
        });
        t.detach();
        if (media_record_plugin_) {
            media_record_plugin_->EndRecord();
        }
        if (sdk_) {
            sdk_->Exit();
            sdk_ = nullptr;
        }
        if (context_) {
            context_->Exit();
            context_ = nullptr;
        }
        ProcessUtil::KillProcess(QApplication::applicationPid());
    }

    void BaseWorkspace::UpdateGameViewsStatus() {
        if (!game_view_) {
            return;
        }
        QList<QScreen*> screens = QGuiApplication::screens();
        if (full_screen_) {
            WidgetSelectMonitor(this, screens);
            this->showFullScreen();
            game_view_->showFullScreen();
            tc::QWidgetHelper::SetBorderInFullScreen(this, true);
            tc::QWidgetHelper::SetBorderInFullScreen(game_view_, true);
        }
        else {
            if (this->isMaximized()) {
                this->showMaximized();
                game_view_->showMaximized();
            }
            else {
                this->showNormal();
                game_view_->showNormal();
            }
        }
    }

    void BaseWorkspace::OnGetCaptureMonitorsCount(int monitors_count) {
        monitors_count_ = monitors_count;
    }

    void BaseWorkspace::OnGetCaptureMonitorName(std::string monitor_name) {
        if (!game_view_) {
            return;
        }
        if (kCaptureAllMonitorsSign == monitor_name) {
            if (monitor_index_map_name_.size() > 0) {
                SendSwitchMonitorMessage(monitor_index_map_name_[0]);
            }
            return;
        }
        game_view_->SetMonitorName(monitor_name);
    }

    void BaseWorkspace::InitGameView(const std::shared_ptr<ThunderSdkParams>& params) {
        this->resize(def_window_size_);
        game_view_ = new GameView(context_, sdk_, params, this);
        game_view_->resize(def_window_size_);
        game_view_->show();
        game_view_->SetMainView(true);
        setCentralWidget(game_view_);

        QTimer::singleShot(1, this, [=, this]() {
            QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
            int x = (screenGeometry.width() - this->width()) / 2;
            int y = (screenGeometry.height() - this->height()) / 2;
            this->move(x, y);
        });
    }

    bool BaseWorkspace::eventFilter(QObject* watched, QEvent* event) {
        return QMainWindow::eventFilter(watched, event);
    }

    std::shared_ptr<ThunderSdk> BaseWorkspace::GetThunderSdk() {
        return sdk_;
    }

    std::shared_ptr<ClientContext> BaseWorkspace::GetContext() {
        return context_;
    }

    void BaseWorkspace::WidgetSelectMonitor(QWidget* widget, QList<QScreen*>& screens) {
        QRect widget_geometry = widget->geometry();
        int max_widget_with_screen_visible_area = 0;
        QScreen* target_screen = nullptr;
        for (auto screen : screens) {
            QRect widget_screen_intersection = screen->availableGeometry().intersected(widget_geometry);
            int  widget_with_screen_visible_area = widget_screen_intersection.width() * widget_screen_intersection.height();
            if (widget_with_screen_visible_area > max_widget_with_screen_visible_area) {
                max_widget_with_screen_visible_area = widget_with_screen_visible_area;
                target_screen = screen;
            }
        }
        if (target_screen) {
            widget->windowHandle()->setScreen(target_screen);
        }
    }

    void BaseWorkspace::ReconnectInRelayMode() {
        if (!settings_->IsRelayMode() || remote_force_closed_) {
            return;
        }
        // Reconnect
        // 1. Can I connect relay server?
        {
            LOGI("will get device info in {}:{} for id: {}", settings_->host_, settings_->port_, settings_->full_device_id_);
            auto r = relay::RelayApi::GetRelayDeviceInfo(settings_->host_, settings_->port_, settings_->full_device_id_);
            if (!r.has_value()) {
                context_->PostUITask([=, this]() {
                    TcDialog dialog(tcTr("id_warning"), tcTr("id_cant_get_local_device_info"), this);
                    dialog.exec();
                });
                return;
            }
        }

        // 2. Can I get remote device info ?
        {
            LOGI("will get remote device info in {}:{} for id: {}", settings_->host_, settings_->port_, settings_->full_remote_device_id_);
            auto r = relay::RelayApi::GetRelayDeviceInfo(settings_->host_, settings_->port_, settings_->full_remote_device_id_);
            if (!r.has_value()) {
                context_->PostUITask([=, this]() {
                    TcDialog dialog(tcTr("id_warning"), tcTr("id_cant_get_remote_device_info"), this);
                    dialog.exec();
                });
                return;
            }
        }

        // 3. Start reconnecting
        sdk_->RetryConnection();

        // show dialog
        context_->PostUITask([=, this]() {
            if (retry_conn_dialog_->isHidden()) {
                WidgetHelper::SetTitleBarColor((QWidget*)(retry_conn_dialog_.get()));
                retry_conn_dialog_->Exec();
            }
        });
    }

    void BaseWorkspace::DismissConnectingDialog() {
        context_->PostUITask([=, this]() {
            // dismiss dialog
            if (retry_conn_dialog_ && !retry_conn_dialog_->isHidden()) {
                retry_conn_dialog_->Done();
            }
        });
    }

    void BaseWorkspace::ProcessNetworkMessage(const std::shared_ptr<tc::Message>& msg) {
        if (msg->type() == MessageType::kDisconnectConnection) {
            const auto& sub = msg->disconnect_connection();
            LOGI("DISCONNECT, device id: {}, stream id: {}", sub.device_id(), sub.stream_id());
            remote_force_closed_ = true;
            context_->PostUITask([=, this]() {
                TcDialog dialog(tcTr("id_warning"), tcTr("id_remote_disconnected"), this);
                dialog.exec();
                Exit();
            });
        }
    }
}