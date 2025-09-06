//
// Created by RGAA on 17/08/2024.
//

#include "float_3rd_resolution_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "client/ct_settings.h"
#include "client/ct_client_context.h"
#include "client/ct_app_message.h"
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
        int offset = 5;
        setFixedSize(210, 360);
        auto item_height = 35;
        auto border_spacing = 10;
        auto item_size = QSize(this->width(), item_height);
        auto root_layout = new NoMarginVLayout();
        root_layout->setContentsMargins(offset, offset, offset, offset);
        settings_ = Settings::Instance();

        listview_ = new SingleSelectedList(this);
        listview_->setFixedSize(QSize(this->width() - 2*offset, this->height()-2*offset));

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

            auto monitor_name = monitor_.name_;
            if (monitor_name.empty()) {
                LOGE("Target monitor name is empty");
                return;
            }
            context_->SendAppMessage(MsgClientChangeMonitorResolution{
                .monitor_name_ = monitor_name,
                .width_ = width,
                .height_ = height,
            });
        });

        root_layout->addWidget(listview_);
        root_layout->addStretch();
        setLayout(root_layout);

        msg_listener_->Listen<MsgClientMonitorChanged>([=, this](const MsgClientMonitorChanged& msg) {
            context_->PostUITask([=, this]() {
                this->SelectCapturingMonitorSize();
            });
        });
    }

    void ThirdResolutionPanel::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        QPen pen(0xaaaaaa);
        pen.setWidth(2);
        pen.setStyle(Qt::PenStyle::DotLine);
        painter.setPen(pen);

        painter.setBrush(QColor(0xffffff));
        int offset = 0;
        int radius = 2;
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
        auto res_name = std::format("{}x{}", monitor_.current_width_, monitor_.current_height_);
        listview_->SelectByName(res_name);
        LOGI("Capturing monitor name is {}, size is {}x{}", monitor_.name_, monitor_.current_width_, monitor_.current_height_);
    }

    void ThirdResolutionPanel::UpdateMonitor(const MsgClientCaptureMonitor::CaptureMonitor& m) {
        monitor_ = m;
        std::vector<SingleItemPtr> items;
        for (const auto& res : monitor_.resolutions_) {
            items.push_back(std::make_shared<SingleItem>(SingleItem { .name_ = std::format("{}x{}", res.width_, res.height_).c_str() }));
        }
        listview_->UpdateItems(items);

        SelectCapturingMonitorSize();
    }

}