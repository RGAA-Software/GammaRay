#include "ct_game_view.h"
#include <qsizepolicy.h>
#include <qpalette.h>
#include <QTimer>
#include <qlabel.h>
#include <qicon.h>
#include <qpixmap.h>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "ct_opengl_video_widget.h"
#include "ct_sdl_video_widget.h"
#include "tc_dialog.h"
#include "no_margin_layout.h"
#include "ct_const_def.h"
#include "ui/float_controller.h"
#include "ui/float_controller_panel.h"
#include "ui/svg_lable.h"
#include "ct_client_context.h"
#include "tc_common_new/log.h"
#include "client/ct_settings.h"
#include "tc_common_new/time_util.h"

namespace tc {

bool GameView::s_mouse_in_ = false;

GameView::GameView(const std::shared_ptr<ClientContext>& ctx, std::shared_ptr<ThunderSdk>& sdk, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent)
    : ctx_(ctx), sdk_(sdk), params_(params), QWidget(parent) {
    WidgetHelper::SetTitleBarColor(this);
    settings_ = Settings::Instance();
    msg_listener_ = ctx_->GetMessageNotifier()->CreateListener();
    this->setAttribute(Qt::WA_StyledBackground, true);
    auto beg = TimeUtil::GetCurrentTimestamp();
    video_widget_ = new OpenGLVideoWidget(ctx, sdk_, 0, RawImageFormat::kRawImageI420, this);
    if (parent) {
        sdl_video_widget_ = new SDLVideoWidget(ctx, sdk_, 0, RawImageFormat::kRawImageI420, nullptr);
        sdl_video_widget_->setFixedSize(1280, 768);
        sdl_video_widget_->show();
    }
    auto end = TimeUtil::GetCurrentTimestamp();
    LOGI("Create OpenGLWidget used: {}ms", (end-beg));

    auto size_policy = video_widget_->sizePolicy();
    size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
    size_policy.setVerticalPolicy(QSizePolicy::Expanding);
    video_widget_->setSizePolicy(size_policy);

    InitFloatController();

    recording_sign_lab_ = new SvgLable(":/resources/image/recording.svg",  this);
    recording_sign_lab_->setFixedSize(36, 28);
    recording_sign_lab_->setAttribute(Qt::WA_StyledBackground,true);
    recording_sign_lab_->setAlignment(Qt::AlignCenter);
    recording_sign_lab_->move(this->width() * 0.85, 20);
    recording_sign_lab_->hide();

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

    msg_listener_->Listen<MsgClientMediaRecord>([=, this](const MsgClientMediaRecord& msg) {
        bool res = ctx_->GetRecording();
        if(res) {
            recording_sign_lab_->show();
            animation->start(); // 开始动画
        }
        else {
            recording_sign_lab_->hide();
            animation->stop(); 
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
    bool enable_full_color = false;
    if (kRawImageI420 == image->Format()) {
        //LOGI("RefreshImage kRawImageI420");
        RefreshI420Image(image);
    }
    else if (kRawImageI444 == image->Format()) {
        //LOGI("RefreshImage kRawImageI444");
        enable_full_color = true;
        RefreshI444Image(image);
    }

    if (is_main_view_) {
        if (settings_->IsFullColorEnabled() != enable_full_color) {
            settings_->SetFullColorEnabled(enable_full_color);
            ctx_->SendAppMessage(MsgClientFloatControllerPanelUpdate{ .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kFullColorStatus });
        }
    }
}

void GameView::RefreshI420Image(const std::shared_ptr<RawImage>& image) {
    if (video_widget_->GetDisplayImageFormat() != kRawImageI420) {
        video_widget_->SetDisplayImageFormat(kRawImageI420);
    }
    video_widget_->RefreshI420Image(image);

    ctx_->PostUITask([=, this]() {
        if (sdl_video_widget_) {
            sdl_video_widget_->RefreshI420Image(image);
        }
    });
}

void GameView::RefreshI444Image(const std::shared_ptr<RawImage>& image) {
    if (video_widget_->GetDisplayImageFormat() != kRawImageI444) {
        video_widget_->SetDisplayImageFormat(kRawImageI444);
    }
    video_widget_->RefreshI444Image(image);
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
    video_widget_->setGeometry(0, target_title_bar_height, this->width(), this->height() - kTitleBarHeight);
}

void GameView::CalculateAspectRatio() {
    auto vw = video_widget_->GetCapturingMonitorWidth();
    auto vh = video_widget_->GetCapturingMonitorHeight();
    // no frame, fill the window
    if (vw <= 0 || vh <= 0) {
        video_widget_->setGeometry(0, kTitleBarHeight, this->width(), this->height());
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

    video_widget_->setGeometry((this->width() - target_width) / 2, (available_height - target_height) / 2 + target_title_bar_height, target_width, target_height);

}

void GameView::InitFloatController()
{
    // float controller
    int shadow_color = 0x999999;
    float_controller_ = new FloatController(ctx_, this);
    float_controller_->installEventFilter(this);
    float_controller_->setFixedSize(50, 50);
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

void GameView::SetMonitorName(const std::string mon_name) {
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

}