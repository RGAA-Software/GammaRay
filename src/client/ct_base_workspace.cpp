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
#include "plugin_interface/ct_media_record_plugin_interface.h"
#include "tc_qt_widget/notify/notifymanager.h"
#include "tc_relay_client/relay_api.h"
#include "tc_message_new/proto_converter.h"
#include "tc_message_new/proto_message_maker.h"
#include "tc_common_new/win32/d3d11_wrapper.h"
#include "front_render/opengl/ct_opengl_video_widget.h"
#include "front_render/vulkan/pl_vulkan.h"
#include "hw_info/hw_info.h"
#include "hw_info/hw_info_parser.h"
#include "hw_info/hw_info_widget.h"
#include "network/ct_spvr_client.h"
#include "skin/skin_loader.h"
#include "skin/interface/skin_interface.h"
#include "ct_game_overlay.h"

namespace tc
{

    std::shared_ptr<BaseWorkspace> gWorkspace;

    BaseWorkspace::BaseWorkspace(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent) : QMainWindow(parent) {
        this->context_ = ctx;
        this->context_->InitNotifyManager(this);
        this->settings_ = Settings::Instance();
        this->params_ = params;
        cursor_ = QCursor(Qt::ArrowCursor);
        retry_conn_dialog_ = std::make_shared<RetryConnDialog>(tcTr("id_warning"));
        QTimer::singleShot(1000, [self = QPointer<BaseWorkspace>(this)]() {
            if (!self) {
                return;
            }
            self->raise();
            self->activateWindow();
        });
        pl_vulkan_ = PlVulkan::Make();

        overlay_widget_ = new OverlayWidget(this);
        overlay_widget_->resize(this->size());
        overlay_widget_->SetOpacity(0.5);
        overlay_widget_->SetWatermarkCount(0);
        overlay_widget_->hide();
        QTimer::singleShot(1000, this, [=, this]() {
            if (overlay_widget_) {
                UpdateOverlayWidgetPos();
                if (this->isHidden()) {
                    overlay_widget_->hide();
                }
                else {
                    overlay_widget_->show();
                }
            }
        });

        //SetWindowDisplayAffinity((HWND)winId(), WDA_EXCLUDEFROMCAPTURE);
    }

    void BaseWorkspace::Init() {
        gWorkspace = shared_from_this();
        // plugins
        InitPluginsManager();

        // skin
        skin_ = SkinLoader::LoadSkin();

        auto beg = TimeUtil::GetCurrentTimestamp();

        InitTheme();

#ifdef WIN32
        if (!settings_->force_software_) {
            gen_d3d11_device_ = GenerateD3DDevice();
        }
        if (gen_d3d11_device_) {
            for (const auto &[adapter_uid, wrapper]: d3d11_devices_) {
                // TODO: find the primary or using d3d11 device
                this->params_->d3d11_wrapper_ = wrapper;
                LOGI("Using the D3D11Device, ID: {}", wrapper->adapter_uid_);
                break;
            }
        }
        else {
            LOGW("!!Can't use D3D11 to render!!");
        }
#endif

        sdk_ = ThunderSdk::Make(this->context_->GetMessageNotifier());
        sdk_->Init(this->params_, nullptr, DecoderRenderType::kFFmpegI420);

        if (!settings_->device_id_.empty() && !settings_->spvr_host_.empty() && settings_->spvr_port_ > 0 && !settings_->appkey_.empty()) {
            LOGI("Will start spvr client, device_id: {}, remote device_id: {}", settings_->device_id_, settings_->remote_device_id_);
            spvr_client_ = std::make_shared<CtSpvrClient>(sdk_,
                                                          context_,
                                                          settings_->spvr_host_,
                                                          settings_->spvr_port_,
                                                          settings_->device_id_,
                                                          settings_->remote_device_id_,
                                                          settings_->appkey_);
            spvr_client_->Start();
        }

        // init game views
        InitGameView(this->params_);

        // vulkan 
        if (this->params_->support_vulkan_) {
            this->params_->vulkan_hw_device_ctx_ = pl_vulkan_->GetHwDeviceCtx();
        }

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
        st_panel_->UpdateClientRenderTypeName(render_type_name_);
        st_panel_->resize(def_window_size_);
        st_panel_->hide();

        hw_info_widget_ = new HWInfoWidget(true, nullptr);
        hw_info_widget_->setWindowTitle(tcTr("id_remote_hw"));
        hw_info_widget_->resize(QSize(1260, 880));
        hw_info_widget_->hide();
    }

    void BaseWorkspace::InitTheme() {
        WidgetHelper::SetTitleBarColor(this, this->params_->titlebar_color_);

        if (this->params_->stream_name_.empty()) {
            origin_title_name_ = tcTr("id_gr_client");
        }
        else {
            origin_title_name_ = tcTr("id_gr_client") + "[" + this->params_->stream_name_.c_str() + "]";
        }
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
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
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

        // step 1t
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
                    if (retry_conn_dialog_->Exec() == -1) {
                        ProcessUtil::KillProcess(QApplication::applicationPid());
                    }
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

            // process watermark
            context_->PostUITask([=, this]() {
                if (settings_->show_watermark_) {
                    overlay_widget_->SetWatermarkCount(10);
                }
                else {
                    overlay_widget_->SetWatermarkCount(0);
                }
            });

            DismissConnectingDialog();
        });

        msg_listener_->Listen<MsgClientChangeMonitorResolution>([=, this](const MsgClientChangeMonitorResolution& msg) {
            this->SendChangeMonitorResolutionMessage(msg);
        });

        msg_listener_->Listen<MsgClientCtrlAltDelete>([=, this](const MsgClientCtrlAltDelete& msg) {
            if (auto buffer = ProtoMessageMaker::MakeCtrlAltDelete(settings_->device_id_, settings_->stream_id_); buffer) {
                sdk_->PostMediaMessage(buffer);
            }
        });

        msg_listener_->Listen<MsgClientHardUpdateDesktop>([=, this](const MsgClientHardUpdateDesktop& msg) {
            this->SendHardUpdateDesktopMessage();
        });

        msg_listener_->Listen<MsgExitControlledEndExe>([=, this](const MsgExitControlledEndExe& msg) {
            this->SendExitControlledEndMessage();
        });

        msg_listener_->Listen<MsgSetHWInfoPanelVisibility>([=, this](const MsgSetHWInfoPanelVisibility& msg) {
            context_->PostUITask([=, this]() {
                hw_info_widget_->show();
            });
        });

        msg_listener_->Listen<MsgHWInfo>([=, this](const MsgHWInfo& msg) {
            if (!msg.info_) {
                return;
            }
            context_->PostUITask([=, this]() {
                hw_info_widget_->OnSysInfoCallback(msg.info_);
                if (!msg.info_->networks_.empty()) {
                    for (const auto& nt : msg.info_->networks_) {
                        auto name = StringUtil::ToLowerCpy(nt.name_);
                        if (name.find("wsl") != std::string::npos
                            || name.find("wmware") != std::string::npos
                            || name.find("virtualbox") != std::string::npos) {
                            continue;
                        }
                        settings_->max_transmit_speed_ = nt.max_transmit_speed_;
                        settings_->max_receive_speed_ = nt.max_receive_speed_;
                    }
                }
            });
        });

        msg_listener_->Listen<SdkMsgVideoDecodeInit>([=, this](const SdkMsgVideoDecodeInit& msg) {
            context_->PostUITask([=, this]() {
                bool notify_user = false;
                QString video_info;
                if (msg.hard_ware_) {
                    return;
                }
                if (msg.width_ > 1920 || msg.format_ == EImageFormat::kI444) {
                    video_info = QString::number(msg.width_) + "x" + QString::number(msg.height_);
                    if (msg.format_ == EImageFormat::kI444) {
                        video_info = video_info + " YUV444";
                    }
                    notify_user = true;
                }
                if (!notify_user) {
                    return;
                }
                video_info = " (" + video_info + ") ";
                context_->NotifyAppWarningMessage(tcTr("id_warning"), tcTr("id_cpu_decode_warning") + video_info);
            });
        });
    }

    BaseWorkspace::~BaseWorkspace() {

    }

    void BaseWorkspace::RegisterSdkMsgCallbacks() {
#if 0
        sdk_->SetOnVideoFrameDecodedCallback([=, this](const std::shared_ptr<RawImage>& image, const SdkCaptureMonitorInfo& info) {
            if (!has_frame_arrived_) {
                has_frame_arrived_ = true;
                UpdateVideoWidgetSize();
            }

            if (game_view_) {
                game_view_->RefreshCapturedMonitorInfo(info);
                game_view_->RefreshImage(image);
            }

            context_->UpdateCapturingMonitorInfo(info);
        });
#endif

        // save pcm file , use ffplay.exe -ar 48000 -ac 2 -f s16le -i .\audio_48000_2.pcm
        sdk_->SetOnAudioFrameDecodedCallback([=, this](std::shared_ptr<Data> data, int samples, int channels, int bits) {
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

        sdk_->SetOnAudioSpectrumCallback([=](std::shared_ptr<tc::Message> msg) {

        });

        sdk_->SetOnCursorInfoCallback([=, this](std::shared_ptr<tc::Message> msg) {
            const auto& cursor_info = msg->cursor_info_sync();
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
                const QImage image((uchar*)cursor_bitmap_data_.data(), cursor_info.width(), cursor_info.height(), QImage::Format_RGBA8888);
                const QPixmap pixmap = QPixmap::fromImage(image);
                QCursor cursor(pixmap, cursor_info.hotspot_x(), cursor_info.hotspot_y());
                cursor_ = cursor;
                this->UpdateLocalCursor();
            }
        });

        sdk_->SetOnHeartBeatCallback([=, this](std::shared_ptr<tc::Message> msg) {
            if (st_panel_) {
                st_panel_->UpdateOnHeartBeat(msg);
            }
            if (btn_indicator_ && settings_->develop_mode_) {
                btn_indicator_->UpdateOnHeartBeat(msg);
            }
        });

        sdk_->SetOnClipboardCallback([=, this](std::shared_ptr<tc::Message> msg) {
            // See: RawMessageCallback
        });

        sdk_->SetOnServerConfigurationCallback([=, this](std::shared_ptr<tc::Message> in_msg) {
            monitor_index_map_name_.clear();
            const auto& config = in_msg->config();

            MsgClientCaptureMonitor msg;
            msg.capturing_monitor_name_ = config.capturing_monitor_name();
            LOGI("capturing monitor name: {}", msg.capturing_monitor_name_);
            int monitor_index = 0;
            for (const auto& item : config.monitors_info()) {
                const std::string& monitor_name = item.name();
                //LOGI("monitor name: {}, width: {}, height: {}", item.name(), item.current_width(), item.current_height());
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
                    //当前显示器分辨率
                    .current_width_ = item.current_width(),
                    .current_height_ = item.current_height(),
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

        sdk_->SetOnMonitorSwitchedCallback([=, this](std::shared_ptr<tc::Message> msg) {
            const auto& ms = msg->monitor_switched();
            context_->SendAppMessage(MsgClientMonitorSwitched {
                .name_ = ms.name(),
                .index_ = ms.index()
            });
        });

        sdk_->SetOnRawMessageCallback([=, this](std::shared_ptr<tc::Message> msg) {
            if (remote_force_closed_) {
                return;
            }
            plugin_manager_->VisitAllPlugins([=, this](ClientPluginInterface* plugin) {
                plugin->OnMessage(msg);
            });

            // parse it
            this->ProcessNetworkMessage(msg);
        });

        sdk_->SetOnVideoFrameDecodeThreadDiscardedCallback([=]() ->void {
            bool need_handle = true;
            auto cur_time = TimeUtil::GetCurrentTimestamp();
            if (cur_time - last_reduce_fps_time_ < 3000) {
                need_handle = false;
            }
            last_reduce_fps_time_ = cur_time;
            if (!need_handle) {
                return;
            }
            if (!settings_) {
                return;
            }
            int cur_fps = settings_->GetFps();
            bool find = false;
            int index = 0;
            for (auto fps : fps_array_) {
                if (cur_fps == fps) {
                    find = true;
                    break;
                }
                ++index;
            }
            if (!find || index == 0) {
                return;
            }
            int new_fps = fps_array_[index - 1];
            LOGI("new fps is {}", new_fps);
            settings_->SetFps(new_fps);
            context_->SendAppMessage(MsgClientModifyFps{
                .fps_ = new_fps,
            });
            context_->SendAppMessage(MsgClientFloatControllerPanelUpdate{ .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kFps });
            context_->PostUITask([=, this]() {
                context_->NotifyAppWarningMessage(tcTr("id_warning"), tcTr("id_auto_reduce_fps_warning") + QString(" (")
                    +  QString::number(cur_fps) + " => " + QString::number(new_fps) + QString(" )"));
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
                    .max_transmit_speed_ = settings_->max_transmit_speed_,
                    .max_receive_speed_ = settings_->max_receive_speed_,
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
            if (auto buffer = tc::ProtoAsData(&m); buffer) {
                sdk_->PostMediaMessage(buffer);
            }
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

        msg_listener_->Listen<MsgClientFocusOutEvent>([=, this](const MsgClientFocusOutEvent& msg) {
            if (!sdk_ || remote_force_closed_) {
                return;
            }
            tc::Message m;
            m.set_type(tc::kFocusOutEvent);
            m.set_device_id(settings_->device_id_);
            m.set_stream_id(settings_->stream_id_);
            if (auto buffer = tc::ProtoAsData(&m); buffer) {
                sdk_->PostMediaMessage(buffer);
            }
        });

        // relay error callback
        msg_listener_->Listen<SdkMsgRelayError>([=, this](const SdkMsgRelayError& msg) {
            if (remote_force_closed_) {
                return;
            }
            //TODO: record it in event center
            //context_->PostUITask([=, this]() {
            //    TcDialog dialog(tcTr("id_error"), msg.msg_.c_str());
            //    dialog.exec();
            //});
        });

        // remote device offline
        msg_listener_->Listen<SdkMsgRelayRemoteDeviceOffline>([=, this](const SdkMsgRelayRemoteDeviceOffline& msg) {
            if (remote_force_closed_) {
                return;
            }
            context_->PostDelayUITask([=, this]() {
                TcDialog dialog(tcTr("id_error"), tcTr("id_remote_device_offline"));
                if (dialog.exec() == kDoneOk) {
                    context_->PostTask([=, this]() {
                        ReconnectInRelayMode();
                    });
                }
                else {
                    // exit
                    ProcessUtil::KillProcess(QApplication::applicationPid());
                }
            }, 1000);
        });
    }

    void BaseWorkspace::changeEvent(QEvent* event) {
        is_window_active_ = isActiveWindow() && !(windowState() & Qt::WindowMinimized);
        qDebug() << "window state: " << is_window_active_;
        QMainWindow::changeEvent(event);
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
        if (overlay_widget_) {
            overlay_widget_->resize(event->size());
        }
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
                LOGI("SendClipboardMessage, file: {}", file.file_name());
            }
        }
        if (auto buffer = tc::ProtoAsData(&m); buffer) {
            sdk_->PostMediaMessage(buffer);
        }
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
        if (auto buffer = tc::ProtoAsData(&m); buffer) {
            sdk_->PostMediaMessage(buffer);
        }
    }

    void BaseWorkspace::SendUpdateDesktopMessage() {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kUpdateDesktop);
        if (auto buffer = tc::ProtoAsData(&m); buffer) {
            sdk_->PostMediaMessage(buffer);
        }
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
        if (auto buffer = tc::ProtoAsData(&m); buffer) {
            sdk_->PostMediaMessage(buffer);
        }
    }

    void BaseWorkspace::SendExitControlledEndMessage() {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kExitControlledEnd);
        if (auto buffer = tc::ProtoAsData(&m); buffer) {
            sdk_->PostMediaMessage(buffer);
        }
    }

    void BaseWorkspace::SendHardUpdateDesktopMessage() {
        if (!sdk_ || remote_force_closed_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kHardUpdateDesktop);
        if (auto buffer = tc::ProtoAsData(&m); buffer) {
            sdk_->PostMediaMessage(buffer);
        }
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
        if (auto buffer = tc::ProtoAsData(&m); buffer) {
            sdk_->PostMediaMessage(buffer);
        }
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
        if (auto buffer = tc::ProtoAsData(&m); buffer) {
            sdk_->PostMediaMessage(buffer);
        }
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
        if (spvr_client_) {
            spvr_client_->Exit();
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
//        game_view_ = new GameView(context_, sdk_, params, this);
//        game_view_->resize(def_window_size_);
//        game_view_->show();
//        game_view_->SetMainView(true);
//        setCentralWidget(game_view_);
//
//        QTimer::singleShot(1, this, [=, this]() {
//            QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
//            int x = (screenGeometry.width() - this->width()) / 2;
//            int y = (screenGeometry.height() - this->height()) / 2;
//            this->move(x, y);
//        });
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
            LOGI("will get device info in {}:{} for id: {}", settings_->relay_host_, settings_->relay_port_, settings_->full_device_id_);
            auto r = relay::RelayApi::GetRelayDeviceInfo(settings_->relay_host_, settings_->relay_port_, settings_->full_device_id_, settings_->relay_appkey_);
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
            LOGI("will get remote device info in {}:{} for id: {}", settings_->relay_host_, settings_->relay_port_, settings_->full_remote_device_id_);
            auto r = relay::RelayApi::GetRelayDeviceInfo(settings_->relay_host_, settings_->relay_port_, settings_->full_remote_device_id_, settings_->relay_appkey_);
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

    void BaseWorkspace::DismissConnectingDialog() const {
        context_->PostUITask([=, this]() {
            // dismiss dialog
            if (retry_conn_dialog_ && !retry_conn_dialog_->isHidden()) {
                retry_conn_dialog_->Done(0);
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
            context_->SendAppMessage(MsgStopTheWorld{});
            ExitSdk();
        }
        else if (msg->type() == MessageType::kHardwareInfo) {
            context_->PostTask([=, this]() {
                const auto& hw_info = msg->hw_info().hw_info();
                const auto freq = msg->hw_info().current_cpu_freq();
                auto sys_info = HWInfoParser::ParseHWInfo(msg->hw_info().hw_info(), freq);
                if (!sys_info) {
                    return;
                }
                context_->SendAppMessage(MsgHWInfo {
                    .info_ = sys_info,
                });
                //LOGI("SysInfo: {}", to_string(*sys_info.get()));
            });
        }
    }

    bool BaseWorkspace::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
#ifdef WIN32
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_ACTIVATE) {
            if (LOWORD(msg->wParam) == WA_INACTIVE) {
                qDebug() << "Window lost focus!";
                context_->PostTask([this]() {
                    context_->SendAppMessage(MsgClientFocusOutEvent{});
                });
            }
            else {
                qDebug() << "Window gained focus!";
            }
        }
        else if (msg->message == WM_EXITSIZEMOVE) {
            UpdateOverlayWidgetPos();
        }
#endif
        return QMainWindow::nativeEvent(eventType, message, result);
    }

    bool BaseWorkspace::GenerateD3DDevice() {
        auto new_device_wrapper = std::make_shared<D3D11DeviceWrapper>();
        const D3D_FEATURE_LEVEL supportedFeatureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

        ComPtr<IDXGIFactory1> factory1 = nullptr;
        ComPtr<IDXGIAdapter1> adapter = nullptr;

        HRESULT res = NULL;
        int adapter_index = 0;
        res = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(factory1.GetAddressOf()));
        if (res != S_OK) {
            LOGE("CreateDXGIFactory1 failed");
            return false;
        }

        ComPtr<IDXGIOutput> output;
        UINT adapterIndex = 0;
        uint32_t my_adapter_uid = 0;

        while(factory1->EnumAdapters1(adapterIndex, adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND) {
            UINT outputIndex = 0;
            while(adapter->EnumOutputs(outputIndex, output.GetAddressOf()) != DXGI_ERROR_NOT_FOUND) {
                DXGI_OUTPUT_DESC desc;
                output->GetDesc(&desc);

                // 判断 hwnd 是否属于这个输出
                auto hwnd = (HWND)this->winId();
                RECT rect;
                GetWindowRect(hwnd, &rect);
                RECT intersect;
                if (IntersectRect(&intersect, &rect, &desc.DesktopCoordinates)) {
                    // 找到窗口所在的显示器输出
                    // 这里 adapterIndex 对应的 Adapter 就是窗口的 GPU
                    DXGI_ADAPTER_DESC adapter_desc;
                    adapter->GetDesc(&adapter_desc);
                    my_adapter_uid = adapter_desc.AdapterLuid.LowPart;
                    LOGI("Found the display adapter...: {}", my_adapter_uid);
                    break;
                }
                ++outputIndex;
            }
            ++adapterIndex;
        }

        while (true) {
            res = factory1->EnumAdapters1(adapter_index, adapter.GetAddressOf());
            if (res != S_OK) {
                LOGE("EnumAdapters1 index:{} failed\n", adapter_index);
                return !d3d11_devices_.empty();
            }

            DXGI_ADAPTER_DESC1 desc1;
            adapter->GetDesc1(&desc1);

            if (desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                LOGI("Skip software adapter: {}", StringUtil::ToUTF8(desc1.Description).c_str());
                ++adapter_index;
                continue;
            }

            auto adapter_uid = desc1.AdapterLuid.LowPart;

            D3D_FEATURE_LEVEL featureLevel;
            res = D3D11CreateDevice(adapter.Get(),
                                    D3D_DRIVER_TYPE_UNKNOWN,
                                    nullptr,
                                    D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                    supportedFeatureLevels,
                                    ARRAYSIZE(supportedFeatureLevels),
                                    D3D11_SDK_VERSION,
                                    &new_device_wrapper->d3d11_device_, &featureLevel, &new_device_wrapper->d3d11_device_context_);

            if (res != S_OK || !new_device_wrapper->d3d11_device_) {
                LOGE("D3D11CreateDevice failed: {}", res);
                return !d3d11_devices_.empty();
            } else {
                if (featureLevel < D3D_FEATURE_LEVEL_11_0) {
                    LOGE("Skip, Feature level < 11 {}");
                    ++adapter_index;
                    continue;
                }
                if (adapter_uid != my_adapter_uid) {
                    LOGE("Skip, I want the: {}, but now: {}", my_adapter_uid, adapter_uid);
                    ++adapter_index;
                    continue;
                }

                auto driver_name = StringUtil::ToUTF8(desc1.Description);
                if (driver_name.find("Microsoft Basic") != std::string::npos) {
                    LOGW("Skip, this is a microsoft basic render: {}", driver_name);
                    ++adapter_index;
                    continue;
                }

                new_device_wrapper->adapter_uid_ = adapter_uid;
                LOGI("++ Adapter Index:{}, UID: {}, Name: {}", adapter_index,  adapter_uid, driver_name);
                LOGI("++ D3D11CreateDevice mDevice = {}", (void *) new_device_wrapper->d3d11_device_.Get());
                d3d11_devices_[adapter_uid] = new_device_wrapper;
            }
            ++adapter_index;
        }

        if (d3d11_devices_.empty()) {
            LOGW("Can't create any D3D11Device/D3D11DeviceContext!");
        }
        return !d3d11_devices_.empty();
    }

    std::shared_ptr<D3D11DeviceWrapper> BaseWorkspace::GetD3D11DeviceWrapper(uint64_t adapter_uid) {
        if (d3d11_devices_.find(adapter_uid) == d3d11_devices_.end()) {
            return nullptr;
        }
        return d3d11_devices_[adapter_uid];
    }

    void BaseWorkspace::PostMediaMessage(std::shared_ptr<Data> msg) {
        sdk_->PostMediaMessage(msg);
    }

    void BaseWorkspace::PostFileTransferMessage(std::shared_ptr<Data> msg) {
        sdk_->PostFileTransferMessage(msg);
    }

    SkinInterface* BaseWorkspace::GetSkin() {
        return skin_;
    }

    void BaseWorkspace::ExitSdk() {
        if (sdk_) {
            sdk_->Exit();
            sdk_ = nullptr;
        }
    }

    void BaseWorkspace::moveEvent(QMoveEvent* event) {
        QMainWindow::moveEvent(event);
        if (overlay_widget_) {
            overlay_widget_->move(this->pos());
        }
    }

    void BaseWorkspace::showEvent(QShowEvent* event) {
        QMainWindow::showEvent(event);
        if (overlay_widget_) {
            overlay_widget_->show();
        }
    }

    void BaseWorkspace::hideEvent(QHideEvent* event) {
        QMainWindow::hideEvent(event);
        if (overlay_widget_) {
            overlay_widget_->hide();
        }
    }

    void BaseWorkspace::mouseReleaseEvent(QMouseEvent* event) {
        QMainWindow::mouseReleaseEvent(event);
    }

    void BaseWorkspace::UpdateOverlayWidgetPos() {
        if (overlay_widget_) {
            QPoint global_pos = mapToGlobal(QPoint(0, 0));
            overlay_widget_->resize(this->size());
            overlay_widget_->move(global_pos);
        }
    }
}