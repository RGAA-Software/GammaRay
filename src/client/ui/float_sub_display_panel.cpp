//
// Created by RGAA on 17/08/2024.
//

#include "float_sub_display_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "settings.h"
#include "float_3rd_scale_panel.h"
#include "float_3rd_resolution_panel.h"
#include "client_context.h"
#include "tc_common_new/log.h"
#include <QLabel>

namespace tc
{

    SubDisplayPanel::SubDisplayPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        setFixedSize(200, 130);
        auto item_height = 38;
        int border_spacing = 5;
        auto item_size = QSize(this->width(), item_height);
        auto root_layout = new NoMarginVLayout();
        auto icon_size = QSize(40, 40);
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new QLabel();
            lbl->setText(tr("Scale"));
            lbl->setStyleSheet(R"(font-weight:bold;)");
            layout->addSpacing(border_spacing*2);
            layout->addWidget(lbl);

            layout->addStretch();

            auto icon_right = new QLabel(this);
            icon_right->setFixedSize(icon_size);
            icon_right->setStyleSheet(R"( background-image: url(:resources/image/ic_arrow_right_2.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addWidget(icon_right);
            layout->addSpacing(border_spacing);
            layout->addSpacing(border_spacing);

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);
            //
            widget->SetOnClickListener([=, this](auto w) {
                auto panel = GetSubPanel(SubDisplayType::kScale);
                if (!panel) {
                    panel = (BaseWidget*)(new ThirdScalePanel(ctx, (QWidget*)this->parent()));
                    sub_panels_[SubDisplayType::kScale] = panel;
                    WidgetHelper::AddShadow(panel, 0xbbbbbb);
                }
                auto item_pos = this->mapTo((QWidget*)this->parent(), w->pos());
                HideAllSubPanels();
                panel->setGeometry(this->pos().x() + this->width(), item_pos.y(), panel->width(), panel->height());
                panel->Show();
            });
        }
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new QLabel();
            lbl->setText(tr("Resolution"));
            lbl->setStyleSheet(R"(font-weight:bold;)");
            layout->addSpacing(border_spacing*2);
            layout->addWidget(lbl);

            layout->addStretch();

            auto icon_right = new QLabel(this);
            icon_right->setFixedSize(icon_size);
            icon_right->setStyleSheet(R"( background-image: url(:resources/image/ic_arrow_right_2.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addWidget(icon_right);
            layout->addSpacing(border_spacing);
            layout->addSpacing(border_spacing);

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);

            widget->SetOnClickListener([=, this](auto w) {
                auto panel = GetSubPanel(SubDisplayType::kResolution);
                if (!panel) {
                    panel = (BaseWidget*)(new ThirdResolutionPanel(ctx, (QWidget*)this->parent()));
                    sub_panels_[SubDisplayType::kResolution] = panel;
                    WidgetHelper::AddShadow(panel, 0xbbbbbb);
                }
                auto item_pos = this->mapTo((QWidget*)this->parent(), w->pos());
                HideAllSubPanels();
                int capturing_monitor_index = context_->GetCapturingMonitorIndex();
                if (monitors_.monitors_.empty() || capturing_monitor_index < 0 || capturing_monitor_index >= monitors_.monitors_.size()) {
                    LOGE("Error monitor index, capturing: {}, total monitor size: {}", capturing_monitor_index, monitors_.monitors_.size());
                    return;
                }
                ((ThirdResolutionPanel*)panel)->UpdateMonitor(monitors_.monitors_.at(capturing_monitor_index));
                panel->setGeometry(this->pos().x() + this->width(), item_pos.y(), panel->width(), panel->height());
                panel->Show();
            });

        }
        root_layout->addStretch();
        setLayout(root_layout);
    }

    void SubDisplayPanel::paintEvent(QPaintEvent *event) {
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

    BaseWidget* SubDisplayPanel::GetSubPanel(const SubDisplayType& type) {
        if (sub_panels_.count(type) > 0) {
            return sub_panels_[type];
        }
        return nullptr;
    }

    void SubDisplayPanel::HideAllSubPanels() {
        for (const auto& [k, v] : sub_panels_) {
            v->Hide();
        }
    }

    void SubDisplayPanel::Show() {
        BaseWidget::Show();
    }

    void SubDisplayPanel::Hide() {
        BaseWidget::Hide();
        HideAllSubPanels();
    }

    void SubDisplayPanel::UpdateMonitorInfo(const CaptureMonitorMessage& m) {
        monitors_ = m;
        // sort it
        for (auto& monitor : monitors_.monitors_) {
            std::sort(monitor.resolutions_.begin(), monitor.resolutions_.end(), [](const auto& left, const auto& right) {
                return left.width_ > right.width_;
            });
        }
    }

}