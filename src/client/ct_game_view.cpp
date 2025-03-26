#include "ct_game_view.h"
#include <qsizepolicy.h>
#include <qpalette.h>
#include "ct_opengl_video_widget.h"
#include "tc_dialog.h"
#include "no_margin_layout.h"

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
}

GameView::~GameView() {
    
}

void GameView::RefreshI420Image(const std::shared_ptr<RawImage>& image) {
    video_widget_->RefreshI420Image(image);
}

void GameView::RefreshCapturedMonitorInfo(const SdkCaptureMonitorInfo& mon_info) {
    video_widget_->RefreshCapturedMonitorInfo(mon_info);
}

}