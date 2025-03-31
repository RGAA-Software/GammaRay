#include "ct_game_view.h"
#include <qsizepolicy.h>
#include <qpalette.h>
#include "ct_opengl_video_widget.h"
#include "tc_dialog.h"
#include "no_margin_layout.h"
#include "ct_const_def.h"

namespace tc {
	
GameView::GameView(const std::shared_ptr<ClientContext>& ctx, std::shared_ptr<ThunderSdk>& sdk, const ThunderSdkParams& params, QWidget* parent) 
    : ctx_(ctx), sdk_(sdk), params_(params), QWidget(parent) {

    auto main_vbox_layout = new NoMarginVLayout();
    main_vbox_layout->setAlignment(Qt::AlignCenter);
    main_vbox_layout->setSpacing(0);
    setLayout(main_vbox_layout);

    video_widget_ = new OpenGLVideoWidget(ctx, sdk_, 0, RawImageFormat::kI420, this);
    //video_widget_->setMouseTracking(true);
    //video_widget_->setFocusPolicy(Qt::StrongFocus);
    //video_widget_->setFocus();
    //video_widget_->setAttribute(Qt::WA_AcceptTouchEvents);
    //video_widget_->setAttribute(Qt::WA_Hover);
    //video_widget_->setAttribute(Qt::WA_InputMethodEnabled);
    //video_widget_->setAttribute(Qt::WA_KeyCompression);
    //video_widget_->setAttribute(Qt::WA_NoSystemBackground);
    
    auto size_policy = video_widget_->sizePolicy();
    size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
    size_policy.setVerticalPolicy(QSizePolicy::Expanding);
    video_widget_->setSizePolicy(size_policy);

    main_vbox_layout->addWidget(video_widget_);

    auto cur_size_policy = this->sizePolicy();
    cur_size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
    cur_size_policy.setVerticalPolicy(QSizePolicy::Expanding);
    this->setSizePolicy(cur_size_policy);
}

GameView::~GameView() {
    
}

void GameView::RefreshI420Image(const std::shared_ptr<RawImage>& image) {
    video_widget_->RefreshI420Image(image);
}

void GameView::RefreshCapturedMonitorInfo(const SdkCaptureMonitorInfo& mon_info) {
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
    video_widget_->setGeometry((this->width() - target_width) / 2, (available_height - target_height) / 2 + target_title_bar_height, target_width, target_height);

}

}