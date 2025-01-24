//
// Created by RGAA on 17/08/2024.
//

#include "float_3rd_scale_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "settings.h"
#include "client_context.h"
#include "app_message.h"
#include "single_selected_list.h"
#include <QLabel>
#include <memory>

namespace tc
{

    ThirdScalePanel::ThirdScalePanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        setFixedSize(200, 130);
        auto root_layout = new NoMarginVLayout();

        settings_ = Settings::Instance();

        listview_ = new SingleSelectedList(this);
        listview_->setFixedSize(this->size());
        listview_->UpdateItems({
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = "Keep Aspect Ratio",
                   .icon_path_ = "",
            }),
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = "Full Window",
                   .icon_path_ = "",
            }),
            // std::make_shared<SingleItem>(SingleItem {
            //        .name_ = "Original Size",
            //        .icon_path_ = "",
            // }),
        });
        root_layout->addWidget(listview_);
        setLayout(root_layout);

        int target_index = 0;
        if (settings_->scale_mode_ == ScaleMode::kKeepAspectRatio) {
            target_index = 0;
        } else if (settings_->scale_mode_ == ScaleMode::kFullWindow) {
            target_index = 1;
        } else if (settings_->scale_mode_ == ScaleMode::kOriginSize) {
            target_index = 2;
        }
        listview_->Select(target_index);

        listview_->SetOnItemClickListener([=, this](int idx, QWidget* w) {
            context_->SendAppMessage(SwitchScaleModeMessage{
                .mode_ = [idx]() {
                    if (idx == 0) { return ScaleMode::kKeepAspectRatio;}
                    else if (idx == 1) {return ScaleMode::kFullWindow;}
                    else if (idx == 2) {return ScaleMode::kOriginSize;}
                    return ScaleMode::kFullWindow;
                }(),
            });
        });
    }

    void ThirdScalePanel::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0xffffff));
        int offset = 0;
        int radius = 5;
        painter.drawRoundedRect(offset, offset, this->width()-offset*2, this->height()-offset*2, radius, radius);
        BaseWidget::paintEvent(event);
    }

    void ThirdScalePanel::UpdateScaleMode(ScaleMode mode) {
        settings_->scale_mode_ = mode;

    }

}