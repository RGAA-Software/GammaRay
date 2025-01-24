//
// Created by RGAA on 17/08/2024.
//

#include "float_sub_mode_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "settings.h"
#include "client_context.h"
#include "app_message.h"
#include <QLabel>

namespace tc
{

    SubModePanel::SubModePanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        setFixedSize(200, 130);
        auto item_height = 38;
        auto border_spacing = 10;
        auto item_size = QSize(this->width(), item_height);
        auto root_layout = new NoMarginVLayout();

        settings_ = Settings::Instance();

        {
            auto layout = new NoMarginHLayout();
            auto widget = new QWidget(this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new QLabel();
            lbl->setText(tr("Work Mode"));
            layout->addSpacing(border_spacing);
            layout->addWidget(lbl);

            layout->addStretch();

            auto sb = new SwitchButton(this);
            sb_work_ = sb;
            sb->setFixedSize(35, 20);
            sb->SetStatus(settings_->work_mode_ == SwitchWorkMode::kWork);
            layout->addWidget(sb);
            sb->SetClickCallback([=, this](bool enabled) {
                settings_->work_mode_ = enabled ? SwitchWorkMode::kWork : SwitchWorkMode::kGame;
                sb_game_->SetStatus(!enabled);
                context_->SendAppMessage(SwitchWorkModeMessage {
                    .mode_ = settings_->work_mode_,
                });
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
            layout->addSpacing(border_spacing);
            layout->addWidget(lbl);

            layout->addStretch();

            auto sb = new SwitchButton(this);
            sb_game_ = sb;
            sb->setFixedSize(35, 20);
            sb->SetStatus(settings_->work_mode_ == SwitchWorkMode::kGame);
            layout->addWidget(sb);
            sb->SetClickCallback([=, this](bool enabled) {
                settings_->work_mode_ = enabled ? SwitchWorkMode::kGame : SwitchWorkMode::kWork;
                sb_work_->SetStatus(!enabled);
                context_->SendAppMessage(SwitchWorkModeMessage {
                    .mode_ = settings_->work_mode_,
                });
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
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0xffffff));
        int offset = 0;
        int radius = 5;
        painter.drawRoundedRect(offset, offset, this->width()-offset*2, this->height()-offset*2, radius, radius);
        BaseWidget::paintEvent(event);
    }

}