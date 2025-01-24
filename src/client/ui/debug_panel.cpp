//
// Created by RGAA on 12/08/2024.
//

#include "debug_panel.h"
#include "key_state_panel.h"
#include "client_context.h"
#include "no_margin_layout.h"
#include <QLabel>
#include "float_icon.h"

namespace tc
{

    DebugPanel::DebugPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        auto root_layout = new NoMarginHLayout();
        root_layout->addSpacing(20);
        {
            auto layout = new NoMarginVLayout();
            layout->addSpacing(20);
            {
                auto lbl = new QLabel(this);
                lbl->setStyleSheet(R"(font-size: 16px; font-weight: bold;)");
                lbl->setText(tr("Server Keyboard State"));
                layout->addWidget(lbl);
            }
            layout->addSpacing(10);

            key_state_panel_ = new KeyStatePanel(ctx, this);
            layout->addWidget(key_state_panel_);
            root_layout->addLayout(layout);
            layout->addStretch();
        }
        root_layout->addStretch();
        setLayout(root_layout);
        this->setStyleSheet("background:#00000000;");

        // close button
        auto btn = new FloatIcon(ctx, this);
        close_btn_ = btn;
        btn->setFixedSize(QSize(30, 30));
        btn->SetIcons(":resources/image/ic_close.svg", "");
        btn->SetOnClickListener([=, this](QWidget* w) {
            this->hide();
        });
    }

    void DebugPanel::resizeEvent(QResizeEvent *event) {
        int close_btn_offset = 10;
        close_btn_->setGeometry(this->width() - close_btn_->width() - close_btn_offset,
                                close_btn_offset, close_btn_->width(), close_btn_->height());
    }

    void DebugPanel::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        painter.setBrush(QBrush(0xffffff));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(this->rect(), 7, 7);
    }

    void DebugPanel::UpdateOnHeartBeat(const OnHeartBeat& hb) {
        context_->PostUITask([=, this]() {
            key_state_panel_->alt_item_->UpdateState(hb.alt_pressed());
            key_state_panel_->shift_item_->UpdateState(hb.shift_pressed());
            key_state_panel_->control_item_->UpdateState(hb.control_pressed());
            key_state_panel_->win_item_->UpdateState(hb.win_pressed());
            key_state_panel_->caps_lock_item_->UpdateState(hb.caps_lock_pressed());
            key_state_panel_->num_lock_item_->UpdateState(hb.num_lock_pressed());
            key_state_panel_->caps_lock_item_->UpdateIndicator(hb.caps_lock_state()==1);
            key_state_panel_->num_lock_item_->UpdateIndicator(hb.num_lock_state()==1);
        });
    }
}