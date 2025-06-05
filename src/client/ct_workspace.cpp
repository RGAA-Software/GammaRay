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
#include "client/clipboard/ct_clipboard_manager.h"
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

    Workspace::Workspace(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent) : QMainWindow(parent) {
        this->context_ = ctx;
        this->context_->InitNotifyManager(this);
        this->settings_ = Settings::Instance();

        //setWindowFlags(windowFlags() | Qt::ExpandedClientAreaHint | Qt::NoTitleBarBackgroundHint);

        // plugins
        plugin_manager_ = ClientPluginManager::Make(ctx);
        context_->SetPluginManager(plugin_manager_);
        plugin_manager_->LoadAllPlugins();
        plugin_manager_->RegisterPluginEventsCallback();
        plugin_manager_->DumpPluginInfo();

        auto beg = TimeUtil::GetCurrentTimestamp();

        WidgetHelper::SetTitleBarColor(this);

        origin_title_name_ = QMainWindow::tr("GammaRay Streamer") + "[" + params->stream_name_.c_str() + "]";
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

        sdk_ = ThunderSdk::Make(ctx->GetMessageNotifier());
        sdk_->Init(params, nullptr, DecoderRenderType::kFFmpegI420);

        // init game views
        {
            auto beg = TimeUtil::GetCurrentTimestamp();
            InitGameViews(params);
            auto end = TimeUtil::GetCurrentTimestamp();
            LOGI("Init game views used: {}ms", (end-beg));
        }
        main_progress_ = new MainProgress(sdk_, context_, this);
        main_progress_->show();

        // button indicator
        int shadow_color = 0x999999;
        btn_indicator_ = new FloatButtonStateIndicator(ctx, this);
        btn_indicator_->hide();
        WidgetHelper::AddShadow(btn_indicator_, shadow_color);

        // debug panel
        st_panel_ = new CtStatisticsPanel(context_, nullptr);
        st_panel_->resize(1366, 768);
        //WidgetHelper::AddShadow(st_panel_, 0x999999);
        st_panel_->hide();

        // notification handle
        // notification_handler_ = new FloatNotificationHandle(context_, this);
        // notification_handler_->SetPixmap(":resources/image/ic_mail.svg");
        // notification_handler_->SetOnClickListener([=, this](QWidget* w) {
        //     if (notification_panel_->isHidden()) {
        //         notification_panel_->show();
        //     } else {
        //         notification_panel_->hide();
        //     }
        //     UpdateNotificationHandlePosition();
        // });

        // notification panel
        //notification_panel_ = new NotificationPanel(ctx, this);
        //notification_panel_->hide();

        // message listener
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();

        // sdk
        RegisterSdkMsgCallbacks();
        sdk_->Start();

        msg_listener_->Listen<ExitAppMessage>([=, this](const ExitAppMessage& msg) {
            context_->PostUITask([=, this]() {
                this->ExitClientWithDialog();
            });
        });

        msg_listener_->Listen<ClipboardMessage>([=, this](const ClipboardMessage& msg) {
            this->SendClipboardMessage(msg);
        });

        msg_listener_->Listen<SwitchMonitorMessage>([=, this](const SwitchMonitorMessage& msg) {
            this->SendSwitchMonitorMessage(msg.name_);
            this->SendUpdateDesktopMessage();
        });

        msg_listener_->Listen<SwitchWorkModeMessage>([=, this](const SwitchWorkModeMessage& msg) {
            this->SendSwitchWorkModeMessage(msg.mode_);
        });

        msg_listener_->Listen<SwitchScaleModeMessage>([=, this](const SwitchScaleModeMessage& msg) {
            this->SwitchScaleMode(msg.mode_);
        });

        msg_listener_->Listen<SwitchFullColorMessage>([=, this](const SwitchFullColorMessage& msg) {
            this->SendSwitchFullColorMessage(msg.enable_);
        });

        // step 1
        msg_listener_->Listen<SdkMsgNetworkConnected>([=, this](const SdkMsgNetworkConnected& msg) {
            this->SendSwitchWorkModeMessage(settings_->work_mode_);
            this->SendUpdateDesktopMessage();
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
            context_->PostUITask([=]() {
                if (EMultiMonDisplayMode::kSeparate == multi_display_mode_) {
                    if (monitors_count_ > 1) {
                        setWindowTitle(origin_title_name_ + QStringLiteral(" (Desktop:%1)").arg(QString::number(1)));
                    }
                    else {
                        setWindowTitle(origin_title_name_);
                    }
                }
                else if(EMultiMonDisplayMode::kTab == multi_display_mode_) {
                    setWindowTitle(origin_title_name_);
                }
            });
            this->SendUpdateDesktopMessage();
        });

        RegisterControllerPanelListeners();

#ifdef TC_ENABLE_FILE_TRANSMISSION
        file_trans_interface_ = FileTransInterface::Make(sdk_);
#endif // TC_ENABLE_FILE_TRANSMISSION

        QTimer::singleShot(100, [=, this](){
            //原有文件传输,已用file_trans_interface_替代
            //file_transfer_ = std::make_shared<FileTransferChannel>(context_);
            //file_transfer_->Start();
        });

        // connect to GammaRay Panel
        panel_client_ = std::make_shared<CtPanelClient>(context_);
        panel_client_->Start();

        {
            auto end = TimeUtil::GetCurrentTimestamp();
            LOGI("Init .3 used: {}ms", (end-beg));
        }
    }

    Workspace::~Workspace() {

    }

    void Workspace::Init() {
        // clipboard manager
        clipboard_mgr_ = std::make_shared<ClipboardManager>(shared_from_this());
        clipboard_mgr_->Start();
    }

    void Workspace::RegisterSdkMsgCallbacks() {
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

        sdk_->SetOnAudioFrameDecodedCallback([=, this](const std::shared_ptr<Data>& data, int samples, int channels, int bits) {
            //LOGI("data size: {}, samples: {}, channel: {}, bits: {}, audio on: {}", data->Size(), samples, channels, bits, settings_->IsAudioEnabled());

#if 0       // save pcm file , use ffplay.exe -ar 48000 -ac 2 -f s16le -i .\audio_48000_2.pcm 
            std::string pcm_file_name_ = std::format(".\\audio_{}_{}.pcm", samples, channels);
            static FILE* pcm_file = fopen(pcm_file_name_.c_str(), "wb");
            fwrite(data->DataAddr(), 1, data->Size(), pcm_file);
            fflush(pcm_file);
#endif

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

        sdk_->SetOnAudioSpectrumCallback([=](const tc::RendererAudioSpectrum& spectrum) {

        });

        sdk_->SetOnCursorInfoCallback([=, this](const CursorInfoSync& cursor_info) {
            this->UpdateLocalCursor(cursor_info.type());
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
            if (clipboard_mgr_) {
                clipboard_mgr_->OnRemoteClipboardMessage(msg);
            }
        });

        sdk_->SetOnServerConfigurationCallback([=, this](const ServerConfiguration& config) {
            CaptureMonitorMessage msg;
            msg.capturing_monitor_name_ = config.capturing_monitor_name();
            //LOGI("capturing monitor name: {}", msg.capturing_monitor_name_);
            int monitor_index = 0;
            for (const auto& item : config.monitors_info()) {
                std::string monitor_name = item.name();
                //LOGI("monitor name: {}", item.name());
                monitor_index_map_name_[monitor_index] = monitor_name;
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

            LOGI("capturing monitors count: {}", monitor_index);

            context_->SendAppMessage(msg);

            int monitors_count = config.monitors_info().size();
            context_->PostUITask([=, this]() {
                OnGetCaptureMonitorName(config.capturing_monitor_name());
                OnGetCaptureMonitorsCount(monitors_count);
            });
        });

        sdk_->SetOnMonitorSwitchedCallback([=, this](const MonitorSwitched& ms) {
            context_->SendAppMessage(MonitorSwitchedMessage {
                .name_ = ms.name(),
                .index_ = ms.index()
            });
        });

        sdk_->SetOnRawMessageCallback([=, this](const std::shared_ptr<tc::Message>& msg) {
            if (file_trans_interface_) {
                file_trans_interface_->OnProtoMessage(msg);
            }

            // test beg //
            if (false && msg->type() == tc::kFileTransDataPacket) {
                // test beg //
                static std::ofstream tst_file("2.test.recv.zip", std::ios::binary);
                std::string data = msg->file_trans_data_packet().data();
                tst_file.write(data.data(), data.size());
                tst_file.flush();
                LOGI("2.Write size: {}", data.size());
                // test end //
            }
            // test end //

            plugin_manager_->VisitAllPlugins([=, this](ClientPluginInterface* plugin) {
                plugin->OnMessage(msg);
            });

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
//                    auto box = SizedMessageBox::MakeInfoOkBox(tr("Info"), tr("Changing resolution success."));
//                    box->exec();

                    TcDialog dialog(tr("Tips"), tr("Changing resolution success."), this);
                    dialog.exec();

                } else {
//                    auto box = SizedMessageBox::MakeErrorOkBox(tr("Error"), tr("Changing resolution failed, please check your server's monitor."));
//                    box->exec();

                    TcDialog dialog(tr("Error"), tr("Changing resolution failed, please check your server's monitor."), this);
                    dialog.exec();
                }
            });

        });

        msg_listener_->Listen<SdkMsgTimer1000>([=, this](const SdkMsgTimer1000& msg) {
            force_update_cursor_ = true;

            plugin_manager_->VisitAllPlugins([=, this](ClientPluginInterface* plugin) {
                plugin->On1Second();

                // test begin
                // plugin->DispatchAppEvent(std::make_shared<ClientAppTestEvent>());
                // test end
            });
        });

        msg_listener_->Listen<SdkMsgClipboardReqBuffer>([=, this](const SdkMsgClipboardReqBuffer& buffer) {
            auto req_index = buffer.req_buffer_.req_index();
            auto req_start = buffer.req_buffer_.req_start();
            auto req_size = buffer.req_buffer_.req_size();
            auto full_filename = buffer.req_buffer_.full_name();

            auto file = File::OpenForReadB(full_filename);
            DataPtr data = nullptr;
            if (file->Exists()) {
                uint64_t read_size = 0;
                data = file->Read(req_start, req_size, read_size);
            }

            tc::Message msg;
            msg.set_device_id(settings_->device_id_);
            msg.set_stream_id(settings_->stream_id_);
            msg.set_type(MessageType::kClipboardRespBuffer);
            auto sub = msg.mutable_cp_resp_buffer();
            sub->set_full_name(full_filename);
            sub->set_req_size(req_size);
            sub->set_req_start(req_start);
            sub->set_req_index(req_index);
            if (data) {
                sub->set_read_size(data->Size());
                sub->set_buffer(data->AsString());
            }

            sdk_->PostFileTransferMessage(msg.SerializeAsString());

            //LOGI("Req: {}, offset: {}, req size: {}", full_filename, req_start, req_size);
        });

        msg_listener_->Listen<FullscreenMessage>([=, this](const FullscreenMessage& msg) {
            context_->PostUITask([=, this]() {
                full_screen_ = true;
                this->UpdateGameViewsStatus();
            });
        });

        msg_listener_->Listen<ExitFullscreenMessage>([=, this](const ExitFullscreenMessage& msg) {
            context_->PostUITask([=, this]() {
                full_screen_ = false;
                this->UpdateGameViewsStatus();
            });
        });

        msg_listener_->Listen<MediaRecordMsg>([=, this](const MediaRecordMsg& msg) {
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
                m.set_type(tc::kStartMediaRecordClientSide);

                LOGI("StartRecord");

                media_record_plugin_->StartRecord();
            }
            else {
                m.set_type(tc::kStopMediaRecordClientSide);


                LOGI("EndRecord");
                media_record_plugin_->EndRecord();
            }
            sdk_->PostMediaMessage(m.SerializeAsString());
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

    void Workspace::ExitClientWithDialog() {
        QString msg = "Do you want to stop controlling of remote PC ?";
        if (file_trans_interface_) {
            if (file_trans_interface_->HasTransTask()) {
                msg = "File transfers are still in progress." + msg;
            }
        }
        TcDialog dialog(tr("Stop"), msg, this);
        if (dialog.exec() == kDoneOk) {


            if (media_record_plugin_) {
                media_record_plugin_->EndRecord();
            }

            Exit();
        }
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
        //if (file_transfer_) {
        //    file_transfer_->SendFiles(files);
        //}
    }

    void Workspace::SendWindowsKey(unsigned long vk, bool down) {
        if (game_views_.size() > 0) {
            if (game_views_[kMainGameViewIndex]) {
                game_views_[kMainGameViewIndex]->SendKeyEvent(vk, down);
            }
        }
    }

    void Workspace::resizeEvent(QResizeEvent *event) {
        main_progress_->setGeometry(0, title_bar_height_, event->size().width(), event->size().height());
        //UpdateNotificationHandlePosition();
        UpdateDebugPanelPosition();
        UpdateVideoWidgetSize();
        UpdateFloatButtonIndicatorPosition();
    }

    // void Workspace::UpdateNotificationHandlePosition() {
    //     int notification_panel_width = 0;
    //     int offset_border = 8;
    //     int handle_offset = 0;
    //     if (!notification_panel_->isHidden()) {
    //         notification_panel_width = notification_panel_->width();
    //         handle_offset = offset_border;
    //     }
    //     notification_panel_->setGeometry(this->width()-notification_panel_->width() - offset_border, offset_border, notification_panel_->width(), this->height() - 2*offset_border);
    //     notification_handler_->setGeometry(this->width()-notification_handler_->width()/2 - notification_panel_width-handle_offset, 100, notification_handler_->width(), notification_handler_->height());
    // }

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

        msg_listener_->Listen<OpenFiletransMsg>([=, this](const OpenFiletransMsg& msg) {
            context_->PostUITask([=, this]() {
                file_trans_interface_->OnClickedFileTrans();
            });
        });

        msg_listener_->Listen<OpenDebugPanelMsg>([=, this](const OpenDebugPanelMsg& msg) {
            context_->PostUITask([=, this]() {
                //st_panel_->setHidden(!st_panel_->isHidden());
                st_panel_->setHidden(false);
            });
        });
    }

    void Workspace::UpdateDebugPanelPosition() {
//        int offset = 120;
//        st_panel_->resize(this->width()-offset, this->height()-offset);
//        st_panel_->move(offset/2, offset/2);
    }

    void Workspace::SendClipboardMessage(const ClipboardMessage& msg) {
        if (!sdk_) {
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

    void Workspace::SendUpdateDesktopMessage() {
        if (!sdk_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kUpdateDesktop);
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

    void Workspace::SendSwitchFullColorMessage(bool enable) {
        if (!sdk_) {
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

    void Workspace::SwitchScaleMode(const tc::ScaleMode& mode) {
        settings_->SetScaleMode(mode);
        if (mode == ScaleMode::kFullWindow) {
            SwitchToFullWindow();
        }
        else if (mode == ScaleMode::kKeepAspectRatio) {
            CalculateAspectRatio();
        }
    }

    void Workspace::CalculateAspectRatio() {
        for (auto game_view : game_views_) {
            if (game_view) {
                game_view->CalculateAspectRatio();
            }
        }
    }

    void Workspace::SwitchToFullWindow() {
        for (auto game_view : game_views_) {
            if (game_view) {
                game_view->SwitchToFullWindow();
            }
        }
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
        //if (file_transfer_) {
        //   file_transfer_->Exit();
        //}
        if (file_trans_interface_) {
            file_trans_interface_->Exit();
        }
        ProcessUtil::KillProcess(QApplication::applicationPid());
        qApp->exit(0);
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
                            game_view->showFullScreen();
                            tc::QWidgetHelper::SetBorderInFullScreen(this, true);
                        }
                        else {
                            WidgetSelectMonitor(game_view, screens);
                            game_view->showFullScreen();
                            tc::QWidgetHelper::SetBorderInFullScreen(game_view, true);
                        }
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

    void Workspace::InitGameViews(const std::shared_ptr<ThunderSdkParams>& params) {
        
        for (int index = 0; index < kMaxGameViewCount; ++index) {
            GameView* game_view = nullptr;
            if (0 == index) {
                game_view = new GameView(context_, sdk_, params, this);    // main view
                game_view->resize(1080, 680);
                game_view->show();
                game_view->SetMainView(true);
                setCentralWidget(game_view);
            }
            else {
                game_view = new GameView(context_, sdk_, params, nullptr); // extend view
                game_view->resize(1080, 680);
                game_view->hide();
                game_view->SetMainView(false);
                game_view->installEventFilter(this);
                game_view->setWindowTitle(origin_title_name_ + QStringLiteral(" (Desktop:%1)").arg(QString::number(index + 1)));
            }
            game_view->SetMonitorIndex(index);
            game_views_.push_back(game_view);
        }
        QTimer::singleShot(200, this, [=, this]() {
            {
                QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
                int x = (screenGeometry.width() - this->width()) / 2;
                int y = (screenGeometry.height() - this->height()) / 2;
                this->resize(1080, 680);
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

    std::shared_ptr<ThunderSdk> Workspace::GetThunderSdk() {
        return sdk_;
    }

    std::shared_ptr<ClientContext> Workspace::GetContext() {
        return context_;
    }

    void Workspace::WidgetSelectMonitor(QWidget* widget, QList<QScreen*>& screens) {
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
}