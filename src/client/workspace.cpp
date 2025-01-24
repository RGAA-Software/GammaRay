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
#include "workspace.h"
#include "thunder_sdk.h"
#include "opengl_video_widget.h"
#include "client_context.h"
#include "tc_common_new/data.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "audio_player.h"
#include "ui/float_controller.h"
#include "ui/float_controller_panel.h"
#include "app_message.h"
#include "settings.h"
#include "ui/float_notification_handle.h"
#include "ui/notification_panel.h"
#include "transfer/file_transfer.h"
#include "ui/sized_msg_box.h"
#include "ui/debug_panel.h"
#include "clipboard_manager.h"
#include "ui/no_margin_layout.h"
#include "tc_client_sdk_new/sdk_messages.h"
#include "tc_common_new/process_util.h"
#include "ui/float_button_state_indicator.h"

namespace tc
{

    Workspace::Workspace(const std::shared_ptr<ClientContext>& ctx, const ThunderSdkParams& params, QWidget* parent) {
        this->context_ = ctx;
        this->settings_ = Settings::Instance();
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

        setCentralWidget(root_widget);

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
        float_controller_->setFixedSize(55, 55);
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
            this->SendSwitchMonitorMessage(msg.index_, msg.name_);
        });

        msg_listener_->Listen<SwitchWorkModeMessage>([=, this](const SwitchWorkModeMessage& msg) {
            this->SendSwitchWorkModeMessage(msg.mode_);
        });

        msg_listener_->Listen<SwitchScaleModeMessage>([=, this](const SwitchScaleModeMessage& msg) {
            this->SwitchScaleMode(msg.mode_);
        });

        msg_listener_->Listen<MsgWsConnected>([=, this](const MsgWsConnected& msg) {
            this->SendSwitchWorkModeMessage(settings_->work_mode_);
        });

        msg_listener_->Listen<MsgWsDisconnected>([=, this](const MsgWsDisconnected& msg) {

        });

        msg_listener_->Listen<MsgChangeMonitorResolution>([=, this](const MsgChangeMonitorResolution& msg) {
            this->SendChangeMonitorResolutionMessage(msg);
        });

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
        sdk_->SetOnVideoFrameDecodedCallback([=, this](const std::shared_ptr<RawImage>& image, const CaptureMonitorInfo& info) {
            if (!has_frame_arrived_) {
                has_frame_arrived_ = true;
                UpdateVideoWidgetSize();
            }
            video_widget_->RefreshCapturedMonitorInfo(info);
            video_widget_->RefreshI420Image(image);
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

        sdk_->SetOnServerConfigurationCallback([=, this](const ServerConfiguration& config) {
            CaptureMonitorMessage msg;
            msg.capturing_monitor_index_ = config.current_capturing_index();
            LOGI("capturing: {}", msg.capturing_monitor_index_);
            for (const auto& item : config.monitor_info()) {
                LOGI("idx: {}, name: {}", item.index(), item.name());
                std::vector<CaptureMonitorMessage::Resolution> resolutions;
                for (auto& res : item.resolutions()) {
                    resolutions.push_back(CaptureMonitorMessage::Resolution {
                        .width_ = res.width(),
                        .height_ = res.height(),
                    });
                }
                msg.monitors_.push_back(CaptureMonitorMessage::CaptureMonitor {
                    .index_ = item.index(),
                    .name_ = item.name(),
                    .resolutions_ = resolutions,
                });
            }
            context_->SendAppMessage(msg);
        });

        sdk_->SetOnMonitorSwitchedCallback([=, this](const MonitorSwitched& ms) {
            context_->SendAppMessage(MonitorSwitchedMessage {
                .index_ = ms.index(),
                .name_ = ms.name(),
            });
        });

        msg_listener_->Listen<MsgChangeMonitorResolutionResult>([=, this](const MsgChangeMonitorResolutionResult& msg) {
            context_->PostUITask([=, this]() {
                // to trigger re-layout
                if (msg.result) {
                    this->move(pos().x()+1, pos().y());
                    auto box = SizedMessageBox::MakeInfoOkBox(tr("Info"), tr("Changing resolution success."));
                    box->exec();
                } else {
                    auto box = SizedMessageBox::MakeErrorOkBox(tr("Error"), tr("Changing resolution failed, please check your server's monitor."));
                    box->exec();
                }
            });

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
        auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Stop"), tr("Do you want to STOP the control of remote PC ?"));
        if (msg_box->exec() == 0) {
            Exit();
        } else {
            event->ignore();
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
        UpdateNotificationHandlePosition();
        UpdateDebugPanelPosition();
        UpdateVideoWidgetSize();
        UpdateFloatButtonIndicatorPosition();
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
        if (cursor_type_ == type) {
            return;
        }
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
        m.mutable_clipboard_info()->set_msg(msg);
        sdk_->PostBinaryMessage(m.SerializeAsString());
    }

    void Workspace::SendSwitchMonitorMessage(int index, const std::string& name) {
        if (!sdk_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kSwitchMonitor);
        m.mutable_switch_monitor()->set_index(index);
        m.mutable_switch_monitor()->set_name(name);
        sdk_->PostBinaryMessage(m.SerializeAsString());
    }

    void Workspace::SendSwitchWorkModeMessage(SwitchWorkMode::WorkMode mode) {
        if (!sdk_) {
            return;
        }
        settings_->SetWorkMode(mode);
        tc::Message m;
        m.set_type(tc::kSwitchWorkMode);
        auto wm = m.mutable_work_mode();
        wm->set_mode(mode);
        sdk_->PostBinaryMessage(m.SerializeAsString());
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
            video_widget_->setGeometry(0, 0, this->width(), this->height());
            return;
        }
        float h_ratio = vw * 1.0f / this->width();
        float v_ratio = vh * 1.0f / this->height();
        int target_width = 0;
        int target_height = 0;
        if (h_ratio > v_ratio) {
            // use width
            target_width = this->width();
            target_height = vh * (this->width()*1.0f/vw);
        } else {
            // use height
            target_height = this->height();
            target_width = vw * (this->height()*1.0f/vh);
        }

        video_widget_->setGeometry((this->width()-target_width)/2, (this->height()-target_height)/2, target_width, target_height);
    }

    void Workspace::SwitchToFullWindow() {
        video_widget_->setGeometry(0, 0, this->width(), this->height());
    }

    void Workspace::SendChangeMonitorResolutionMessage(const MsgChangeMonitorResolution& msg) {
        if (!sdk_) {
            return;
        }
        tc::Message m;
        m.set_type(tc::kChangeMonitorResolution);
        auto cmr = m.mutable_change_monitor_resolution();
        cmr->set_monitor_name(msg.monitor_name_);
        cmr->set_target_width(msg.width_);
        cmr->set_target_height(msg.height_);
        sdk_->PostBinaryMessage(m.SerializeAsString());
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
        qApp->exit(0);
    }

}