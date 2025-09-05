//
// Created by RGAA on 17/08/2024.
//

#include "float_sub_mode_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "client/ct_settings.h"
#include "client/ct_client_context.h"
#include "client/ct_app_message.h"
#include "tc_common_new/log.h"
#include <QLabel>

namespace tc
{

    SubModePanel::SubModePanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        int offset = 5;
        setFixedSize(210, 138);
        auto item_height = 38;
        auto border_spacing = 10;
        auto item_size = QSize(this->width() - 2*offset, item_height);
        auto root_layout = new NoMarginVLayout();
        root_layout->setContentsMargins(offset, offset, offset, offset);

        settings_ = Settings::Instance();

        {
            auto layout = new NoMarginHLayout();
            auto widget = new QWidget(this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new QLabel();
            lbl->setText(tr("Work Mode"));
            lbl->setStyleSheet(R"(font-weight:bold;)");
            layout->addSpacing(border_spacing*2);
            layout->addWidget(lbl);

            layout->addStretch();

            auto sb = new SwitchButton(this);
            sb_work_ = sb;
            sb->setFixedSize(35, 20);
            sb->SetStatus(settings_->work_mode_ == SwitchWorkMode::kWork);
            layout->addWidget(sb);
            sb->SetClickCallback([=, this](bool enabled) {
                SwitchWorkMode::WorkMode mode = enabled ? SwitchWorkMode::kWork : SwitchWorkMode::kGame;
                settings_->SetWorkMode(mode);
                sb_game_->SetStatus(!enabled);
                context_->SendAppMessage(MsgClientSwitchWorkMode {
                    .mode_ = mode,
                });
                context_->SendAppMessage(MsgClientFloatControllerPanelUpdate{.update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kWorkMode});
            });

            layout->addSpacing(border_spacing);

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);
        }
        {
            auto layout = new NoMarginHLayout();
            auto widget = new QWidget(this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new QLabel();
            lbl->setText(tr("Game Mode"));
            lbl->setStyleSheet(R"(font-weight:bold;)");
            layout->addSpacing(border_spacing*2);
            layout->addWidget(lbl);

            layout->addStretch();

            auto sb = new SwitchButton(this);
            sb_game_ = sb;
            sb->setFixedSize(35, 20);
            sb->SetStatus(settings_->work_mode_ == SwitchWorkMode::kGame);
            layout->addWidget(sb);
            sb->SetClickCallback([=, this](bool enabled) {
                SwitchWorkMode::WorkMode mode = enabled ? SwitchWorkMode::kGame : SwitchWorkMode::kWork;
                settings_->SetWorkMode(mode);
                sb_work_->SetStatus(!enabled);
                context_->SendAppMessage(MsgClientSwitchWorkMode {
                    .mode_ = mode,
                });
                context_->SendAppMessage(MsgClientFloatControllerPanelUpdate{ .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kWorkMode });
            });

            layout->addSpacing(border_spacing);

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);
        }

        root_layout->addStretch();
        setLayout(root_layout);
    }

    void SubModePanel::paintEvent(QPaintEvent *event) {
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

    void SubModePanel::UpdateStatus(const MsgClientFloatControllerPanelUpdate& msg) {
        if (MsgClientFloatControllerPanelUpdate::EUpdate::kWorkMode == msg.update_type_) {
            sb_game_->SetStatus(SwitchWorkMode::kGame == settings_->work_mode_);
            sb_work_->SetStatus(SwitchWorkMode::kWork == settings_->work_mode_);
        }
    }
}