//
// Created by RGAA on 17/08/2024.
//

#include "float_3rd_resolution_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "settings.h"
#include "client_context.h"
#include "app_message.h"
#include "single_selected_list.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include <QLabel>
#include <format>

namespace tc
{

    ThirdResolutionPanel::ThirdResolutionPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        setFixedSize(200, 350);
        auto item_height = 38;
        auto border_spacing = 10;
        auto item_size = QSize(this->width(), item_height);
        auto root_layout = new NoMarginVLayout();

        settings_ = Settings::Instance();

        listview_ = new SingleSelectedList(this);
        listview_->setFixedSize(this->size());

        listview_->SetOnItemClickListener([=, this](int idx, QWidget*) {
            auto item = listview_->GetItems().at(idx);
            auto split_size = item->name_.split("x");
            if (split_size.size() < 2) {
                return;
            }
            int width = split_size.at(0).toInt();
            int height = split_size.at(1).toInt();
            if (width <= 0 || height <= 0) {
                LOGE("Error monitor resolution size: {}", item->name_.toStdString());
                return;
            }
            auto monitor_name = context_->GetCapturingMonitorInfo().mon_name_;
            if (monitor_name.empty()) {
                LOGE("Target monitor name is empty");
                return;
            }
            context_->SendAppMessage(MsgChangeMonitorResolution {
                .monitor_name_ = monitor_name,
                .width_ = width,
                .height_ = height,
            });
        });

        root_layout->addWidget(listview_);

        root_layout->addStretch();
        setLayout(root_layout);

        CreateMsgListener();
        msg_listener_->Listen<MsgMonitorChanged>([=, this](const MsgMonitorChanged& msg) {
            context_->PostUITask([=, this]() {
                this->SelectCapturingMonitorSize();
            });
        });
    }

    void ThirdResolutionPanel::paintEvent(QPaintEvent *event) {
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

    void ThirdResolutionPanel::Hide() {
        BaseWidget::Hide();
    }

    void ThirdResolutionPanel::Show() {
        BaseWidget::Show();
        this->SelectCapturingMonitorSize();
    }

    void ThirdResolutionPanel::SelectCapturingMonitorSize() {
        auto monitor_info = context_->GetCapturingMonitorInfo();
        if (monitor_info.Width() <= 0 || monitor_info.Height() <= 0) {
            return;
        }
        listview_->SelectByName(std::format("{}x{}", monitor_info.Width(), monitor_info.Height()));
    }

    void ThirdResolutionPanel::UpdateMonitor(const CaptureMonitorMessage::CaptureMonitor& m) {
        monitor_ = m;
        std::vector<SingleItemPtr> items;
        for (const auto& res : monitor_.resolutions_) {
            items.push_back(std::make_shared<SingleItem>(SingleItem { .name_ = std::format("{}x{}", res.width_, res.height_).c_str() }));
        }
        listview_->UpdateItems(items);
    }

}