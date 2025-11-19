#include "ct_game_view.h"
#include <qsizepolicy.h>
#include <qpalette.h>
#include <QTimer>
#include <QPixmap>
#include <QDateTime>
#include <QStandardPaths>
#include <QMainWindow>
#include <QProcess>
#include <qlabel.h>
#include <qicon.h>
#include <qpointer.h>
#include <qpixmap.h>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "tc_dialog.h"
#include "no_margin_layout.h"
#include "ct_const_def.h"
#include "ui/float_controller.h"
#include "ui/float_controller_panel.h"
#include "ui/svg_lable.h"
#include "ui/media_record_sign_lab.h"
#include "ct_client_context.h"
#include "tc_common_new/log.h"
#include "client/ct_settings.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/file_util.h"
#include "tc_client_sdk_new/sdk_messages.h"
#include "front_render/sdl/ct_sdl_video_widget.h"
#include "front_render/d3d11/ct_d3d11_video_widget.h"
#include "front_render/opengl/ct_opengl_video_widget.h"
#include "front_render/vulkan/ct_vulkan_video_widget.h"

namespace tc
{

    bool GameView::s_mouse_in_ = false;

    GameView::GameView(const std::shared_ptr<ClientContext>& ctx, std::shared_ptr<ThunderSdk>& sdk, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent)
        : ctx_(ctx), sdk_(sdk), params_(params), QWidget(parent) {
        WidgetHelper::SetTitleBarColor(this, this->params_->titlebar_color_);
        settings_ = Settings::Instance();
        msg_listener_ = ctx_->GetMessageNotifier()->CreateListener();
        this->setAttribute(Qt::WA_StyledBackground, true);
        auto beg = TimeUtil::GetCurrentTimestamp();

        this->thread_ = Thread::Make("d3d_render", 120);
        this->thread_->Poll();

    #if TEST_SDL
        if (parent) {
            sdl_video_widget_ = new SDLVideoWidget(ctx, sdk_, 0, RawImageFormat::kRawImageI420, nullptr);
            sdl_video_widget_->setFixedSize(1280, 768);
            sdl_video_widget_->show();
        }
    #endif

#ifdef WIN32
        if (params_->support_vulkan_) {
            LOGI("*** Use vulkan to render frames");
            video_widget_ = new VulkanVideoWidget(ctx, sdk_, 0, RawImageFormat::kRawImageVulkanAVFrame, this);
        }
        else {
            if (params_->d3d11_wrapper_) {
                video_widget_ = new D3D11VideoWidget(ctx, sdk_, 0, RawImageFormat::kRawImageD3D11Texture, this);
                LOGI("*** Use D3D11 to render frames");
            }
            else {
                video_widget_ = new OpenGLVideoWidget(ctx, sdk_, 0, RawImageFormat::kRawImageI420, this);
                LOGI("*** Use OpenGL to render frames");
            }
        }
#else
        video_widget_ = new OpenGLVideoWidget(ctx, sdk_, 0, RawImageFormat::kRawImageI420, this);
#endif
        auto end = TimeUtil::GetCurrentTimestamp();
        LOGI("Create OpenGLWidget used: {}ms", (end-beg));

        auto size_policy = video_widget_->AsWidget()->sizePolicy();
        size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
        size_policy.setVerticalPolicy(QSizePolicy::Expanding);
        video_widget_->AsWidget()->setSizePolicy(size_policy);

        InitFloatController();

        recording_sign_lab_ = new MediaRecordSignLab(ctx, this);
        recording_sign_lab_->move(this->width() * 0.85, 20);
        recording_sign_lab_->hide();

#if 0
        // 创建透明度效果
        QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(recording_sign_lab_);
        recording_sign_lab_->setGraphicsEffect(opacityEffect);

        // 创建动画
        QPropertyAnimation* animation = new QPropertyAnimation(opacityEffect, "opacity", recording_sign_lab_);
        animation->setDuration(1500);
        animation->setStartValue(1.0);  // 完全不透明
        animation->setKeyValueAt(0.5, 0.0); // 中间点完全透明
        animation->setEndValue(1.0); // 结束时不透明
        animation->setLoopCount(-1); // 无限循环
#endif
        msg_listener_->Listen<MsgClientMediaRecord>([=, this](const MsgClientMediaRecord& msg) {
            bool res = ctx_->GetRecording();
            if(res) {
                recording_sign_lab_->show();
                //animation->start(); // 开始动画
            }
            else {
                recording_sign_lab_->hide();
                //animation->stop();
            }
        });

        msg_listener_->Listen<MsgClientSwitchMonitor>([=, this](const MsgClientSwitchMonitor& msg) {
            if (ScaleMode::kKeepAspectRatio == settings_->scale_mode_ && !isHidden()) {
                need_recalculate_aspect_ = true;
            }
        });


        msg_listener_->Listen<MsgClientHidePanel>([=, this](const MsgClientHidePanel& msg) {
            ctx_->PostUITask([=, this]() {
                controller_panel_->Hide();
            });
        });

        msg_listener_->Listen<SdkMsgTimer1000>([=, this](const SdkMsgTimer1000& msg) {
            if (video_widget_) {
                video_widget_->OnTimer1S();
            }
        });

        msg_listener_->Listen<MsgStreamShot>([=, this](const MsgStreamShot& msg) {
            ctx_->PostTask([=, this]() {
                this->SnapshotStream();
            });
        });
    }

    GameView::~GameView() {

    }

    void GameView::resizeEvent(QResizeEvent* event) {
        auto scale_mode = settings_->scale_mode_;
        if (scale_mode == ScaleMode::kFullWindow) {
            SwitchToFullWindow();
        }
        else if (scale_mode == ScaleMode::kKeepAspectRatio) {
            CalculateAspectRatio();
        }

        if (float_controller_) {
            float_controller_->ReCalculatePosition();
        }
        recording_sign_lab_->move(this->width() * 0.85, 20);
        QWidget::resizeEvent(event);
    }

    void GameView::RefreshImage(const std::shared_ptr<RawImage>& image) {
        if (video_widget_) {
            video_widget_->RefreshImage(image);
        }
        UpdateFullColorState(image->full_color_);
    }

    void GameView::UpdateFullColorState(bool full_color) {
        if (!is_main_view_) {
            return;
        }
        if (settings_->IsFullColorEnabled() != full_color) {
            settings_->SetFullColorEnabled(full_color);
            ctx_->SendAppMessage(MsgClientFloatControllerPanelUpdate{
                .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kFullColorStatus
            });
        }
    }

    void GameView::RefreshI420Image(const std::shared_ptr<RawImage>& image) {
        if (video_widget_->GetDisplayImageFormat() != kRawImageI420) {
            video_widget_->SetDisplayImageFormat(kRawImageI420);
        }
        video_widget_->RefreshImage(image);

    #if TEST_SDL
        ctx_->PostUITask([=, this]() {
            if (sdl_video_widget_) {
                sdl_video_widget_->RefreshI420Image(image);
            }
        });
    #endif
    }

    void GameView::RefreshI444Image(const std::shared_ptr<RawImage>& image) {
        if (video_widget_->GetDisplayImageFormat() != kRawImageI444) {
            video_widget_->SetDisplayImageFormat(kRawImageI444);
        }
        video_widget_->RefreshImage(image);
    }

    void GameView::RefreshCapturedMonitorInfo(const SdkCaptureMonitorInfo& mon_info) {
        // 若按比例缩放的情况下，切换了屏幕，屏幕分辨率未必一致，如一个4K,一个2K，故重新计算
        if (need_recalculate_aspect_ && ScaleMode::kKeepAspectRatio == settings_->scale_mode_) {
            const auto& exist_mon_info = video_widget_->GetCaptureMonitorInfo();
            if (mon_info.mon_name_ != exist_mon_info.mon_name_ && !exist_mon_info.mon_name_.empty()) {
                ctx_->PostDelayUITask([=, this]() {
                    this->CalculateAspectRatio();
                }, 100);
                need_recalculate_aspect_ = false;
            }
        }
        video_widget_->RefreshCapturedMonitorInfo(mon_info);
    }

    void GameView::SendKeyEvent(quint32 vk, bool down) {
        video_widget_->SendKeyEvent(vk, down);
    }

    void GameView::SwitchToFullWindow() {
        auto target_title_bar_height = this->isFullScreen() ? 0 : kTitleBarHeight;
        video_widget_->AsWidget()->setGeometry(0, target_title_bar_height, this->width(), this->height() - kTitleBarHeight);
    }

    void GameView::CalculateAspectRatio() {
        auto vw = video_widget_->GetCapturingMonitorWidth();
        auto vh = video_widget_->GetCapturingMonitorHeight();
        // no frame, fill the window
        if (vw <= 0 || vh <= 0) {
            video_widget_->AsWidget()->setGeometry(0, kTitleBarHeight, this->width(), this->height());
            return;
        }

        auto target_title_bar_height = this->isFullScreen() ? 0 : kTitleBarHeight;

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
            target_width = vw * (available_height * 1.0f / vh);
        }
        else {
            // along to width
            target_width = this->width();
            target_height = vh * (this->width() * 1.0f / vw);
        }

        video_widget_->AsWidget()->setGeometry((this->width() - target_width) / 2, (available_height - target_height) / 2 + target_title_bar_height, target_width, target_height);

    }

    void GameView::InitFloatController()
    {
        // float controller
        int shadow_color = 0x999999;
        float_controller_ = new FloatController(ctx_, this);
        float_controller_->installEventFilter(this);
        float_controller_->setFixedSize(40, 40);
        WidgetHelper::AddShadow(float_controller_, shadow_color);
        controller_panel_ = new FloatControllerPanel(ctx_, this);
        WidgetHelper::AddShadow(controller_panel_, shadow_color);
        RegisterControllerPanelListeners();
        controller_panel_->installEventFilter(this);
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
            }
            else {
                controller_panel_->Hide();
            }
        });

        float_controller_->SetOnMoveListener([=, this]() {
            if (!controller_panel_) {
                return;
            }
            controller_panel_->Hide();
        });
    }

    void GameView::RegisterControllerPanelListeners() {
        controller_panel_->SetOnDebugListener([=, this](QWidget* w) {
            ctx_->PostUITask([=]() {
                controller_panel_->Hide();
            });
            this->ctx_->SendAppMessage(MsgClientOpenDebugPanel{});
        });

        controller_panel_->SetOnFileTransListener([=, this](QWidget* w) {
            ctx_->PostUITask([=]() {
                controller_panel_->Hide();
            });
            this->ctx_->SendAppMessage(MsgClientOpenFiletrans{});
        });

        controller_panel_->SetOnMediaRecordListener([=, this](QWidget* w) {
            ctx_->PostUITask([=]() {
                controller_panel_->Hide();
            });
        });
    }

    void GameView::SetMainView(bool main_view) {
        is_main_view_ = main_view;
        if (is_main_view_ && controller_panel_) {
            controller_panel_->SetMainControl();
        }
    }

    void GameView::SetMonitorName(const std::string& mon_name) {
        monitor_name_ = mon_name;
        controller_panel_->SetMonitorName(mon_name);
    }

    void GameView::enterEvent(QEnterEvent* event) {
        s_mouse_in_ = true;
        this->ctx_->SendAppMessage(MsgClientMouseEnterView{});
        QWidget::enterEvent(event);
    }

    void GameView::leaveEvent(QEvent* event) {
        s_mouse_in_ = false;
        this->ctx_->SendAppMessage(MsgClientMouseLeaveView{});
        QWidget::leaveEvent(event);
    }

    bool GameView::eventFilter(QObject* watched, QEvent* event) {
        if (watched == controller_panel_ || watched == float_controller_)
        {
            switch (event->type())
            {
            case QEvent::Enter:
                s_mouse_in_ = false;
                this->ctx_->SendAppMessage(MsgClientMouseLeaveView{});
                break;
            case QEvent::Leave:
                s_mouse_in_ = true;
                this->ctx_->SendAppMessage(MsgClientMouseEnterView{});
                break;
            default:
                break;
            }
        }
        return QWidget::eventFilter(watched, event);
    }

    bool GameView::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
        if (eventType == "windows_generic_MSG") {
            MSG* msg = static_cast<MSG*>(message);
            if (msg->message == WM_ACTIVATE) {
                if (LOWORD(msg->wParam) == WA_INACTIVE) {
                    qDebug() << "Window lost focus!";
                    ctx_->PostTask([this]() {
                        ctx_->SendAppMessage(MsgClientFocusOutEvent{});
                    });
                }
                else {
                    qDebug() << "Window gained focus!";
                }
            }
        }
        return QWidget::nativeEvent(eventType, message, result);
    }

    void GameView::SetActiveStatus(bool active) {
        active_ = active;
    }

    bool GameView::GetActiveStatus() const {
        return active_;
    }

    bool GameView::IsMainView() const {
        return is_main_view_;
    }

    void GameView::SnapshotStream() {
        if (this->isHidden()) {
            return;
        }
        auto image = video_widget_->CaptureImage();
        if (image.isNull()) {
            return;
        }

        auto name = [=, this]() {
            if (!this->windowTitle().isEmpty()) {
                return this->windowTitle();
            }
            if (this->parent()) {
                if (auto pw = ((QMainWindow*)this->parent()); pw) {
                    return pw->windowTitle();
                }
            }
            return QString::fromStdString("Default");
        } ();
        name = name.replace("(", "").replace(")", "").replace(":", "_");
        QString png_name = name + "_" + QString::number(QDateTime::currentSecsSinceEpoch()) + ".png";
        QString pic_path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        QString png_path = pic_path + "/" + png_name;
        if (image.save(png_path)) {
            auto callback = [=]() {
                auto path = png_path;
                FileUtil::SelectFileInExplorer(path.toStdString());
            };
            ctx_->NotifyAppMessage("Snap Success", std::format("Saved to: {}", pic_path.toStdString()).c_str(), callback);
        }
    }

    HWND GameView::GetVideoHwnd() {
        return (HWND)video_widget_->GetRenderWId();
    }

    void GameView::showEvent(QShowEvent* event) {
        QWidget::showEvent(event);
        // 非主窗口创建出来后，并没有显示出来, 需要触发一次 resizeEvent 来更新交换链信息
        if (width() > 0 && height() > 0) {
            resize(width() + 1, height() + 1);
        }
        QTimer::singleShot(60, [self = QPointer<GameView>(this)]() {
            if (!self) {
                return;
            }
            self->resize(self->width() - 1, self->height() - 1);
        });
    }

    std::string GameView::GetRenderTypeName() {
        if (!video_widget_) {
            return "";
        }
        return video_widget_->GetRenderTypeName();
    }
}