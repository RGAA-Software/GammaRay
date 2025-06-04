//
// Created by RGAA on 30/04/2025.
//

#include "st_plugin_item_widget.h"
#include "no_margin_layout.h"
#include "st_plugins.h"
#include "tc_render_panel_message.pb.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_app_messages.h"
#include <QLabel>
#include <QPushButton>

namespace tc
{

    const QString kDisplayPluginEnabled = "Enabled";
    const QString kDisplayPluginDisabled = "Disabled";

    StPluginItemWidget::StPluginItemWidget(const std::shared_ptr<GrApplication>& app,
                       const std::shared_ptr<PluginItemInfo>& item_info,
                       int index,
                       QWidget* parent) : QWidget(parent) {
        app_ = app;
        context_ = app->GetContext();
        item_info_ = item_info;

        this->setObjectName("StPluginItemWidget");
        this->setStyleSheet("#StPluginItemWidget {background:#ffffffff;}");

        auto root_layout = new NoMarginVLayout();

        auto content_layout = new NoMarginHLayout();
        root_layout->addStretch();
        root_layout->addLayout(content_layout);
        root_layout->addStretch();

        content_layout->addSpacing(20);

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(R"(font-weight: 700;)");
            lbl->setFixedWidth(20);
            lbl->setText(std::to_string(index+1).c_str());
            content_layout->addWidget(lbl);
            content_layout->addSpacing(20);
        }

        // icon
        {
            auto icon = new QLabel(this);
            icon->setFixedSize(30, 30);
            QString style = R"(background-image: url(:resources/image/ic_plugin.svg);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
            icon->setStyleSheet(style);
            content_layout->addWidget(icon);
            content_layout->addSpacing(20);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(R"(font-weight: 700;)");
            lbl->setFixedWidth(180);
            lbl->setText(item_info->info_->name().c_str());
            content_layout->addWidget(lbl);
            content_layout->addSpacing(20);
        }

        {
            auto lbl = new QLabel(this);
            lbl->setStyleSheet(R"(font-size: 12px;)");
            lbl->setFixedWidth(400);
            lbl->setText(item_info->info_->desc().c_str());
            content_layout->addWidget(lbl);
            content_layout->addSpacing(20);
        }

        {
            auto lbl = new QLabel(this);
            lbl_enabled_ = lbl;
            lbl->setFixedWidth(120);
            content_layout->addWidget(lbl);
            content_layout->addSpacing(20);
            UpdatePluginStatus(item_info->info_->enabled());
        }

        content_layout->addStretch();

        auto size = QSize(75, 30);
        {
            auto btn = new QPushButton(this);
            btn->setProperty("class", "danger");
            btn->setFixedSize(size);
            btn->setText("Disable");
            content_layout->addWidget(btn);
            content_layout->addSpacing(10);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                SwitchPluginStatusInner(false);
            });
        }
        {
            auto btn = new QPushButton(this);
            btn->setFixedSize(size);
            btn->setText("Enable");
            content_layout->addWidget(btn);
            content_layout->addSpacing(10);
            connect(btn, &QPushButton::clicked, this, [=, this]() {
                SwitchPluginStatusInner(true);
            });
        }

        content_layout->addSpacing(10);

        setLayout(root_layout);
    }

    void StPluginItemWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        QPen pen;
        pen.setStyle(Qt::PenStyle::DashDotDotLine);
        pen.setColor(0xdddddd);
        painter.setPen(pen);
        int offset = 3;
        painter.drawRoundedRect(QRect(offset, offset, this->width()-offset*2, this->height()-offset*2), 5, 5);

        QWidget::paintEvent(event);
    }

    void StPluginItemWidget::UpdateStatus() {
        if (!lbl_enabled_) {
            return;
        }

        bool need_update = false;
        auto enabled = item_info_->info_->enabled();
        if (enabled) {
            if (lbl_enabled_->text() != kDisplayPluginEnabled) {
                need_update = true;
            }
        }
        else {
            if (lbl_enabled_->text() != kDisplayPluginDisabled) {
                need_update = true;
            }
        }
        if (need_update) {
            UpdatePluginStatus(enabled);
        }

    }

    void StPluginItemWidget::UpdatePluginStatus(bool enabled) {
        if (enabled) {
            lbl_enabled_->setText(kDisplayPluginEnabled);
            lbl_enabled_->setStyleSheet(R"(font-weight: 700; color: #555555;)");
        }
        else {
            lbl_enabled_->setText(kDisplayPluginDisabled);
            lbl_enabled_->setStyleSheet(R"(font-weight: 700; color: #ff2200;)");
        }
    }

    void StPluginItemWidget::SwitchPluginStatusInner(bool enabled) {
        tcrp::RpMessage pt_msg;
        pt_msg.set_type(tcrp::RpMessageType::kRpCommandRenderer);
        auto sub = pt_msg.mutable_command_renderer();
        sub->set_command(enabled ? tcrp::RpPanelCommand::kEnablePlugin : tcrp::RpPanelCommand::kDisablePlugin);
        sub->set_plugin_id(item_info_->id_);
        app_->PostMessage2Renderer(pt_msg.SerializeAsString());
    }

}