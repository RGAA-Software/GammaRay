//
// Created by RGAA on 2/09/2024.
//

#include "float_button_state_indicator.h"
#include "key_state_panel.h"
#include "no_margin_layout.h"
#include "client_context.h"

namespace tc
{

    FloatButtonStateIndicator::FloatButtonStateIndicator(const std::shared_ptr<ClientContext>& ctx, QWidget* parent)
        : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        auto layout = new NoMarginHLayout();
        auto size = QSize(65, 30);
        {
            auto item = new KeyItem("ALT");
            item->setFixedSize(size);
            alt_item_ = item;
            layout->addWidget(item);
        }
        {
            auto item = new KeyItem("SHIFT");
            item->setFixedSize(size);
            shift_item_ = item;
            layout->addWidget(item);
        }
        {
            auto item = new KeyItem("CTRL");
            item->setFixedSize(size);
            control_item_ = item;
            layout->addWidget(item);
        }
        {
            auto item = new KeyItem("WIN");
            item->setFixedSize(size);
            win_item_ = item;
            layout->addWidget(item);
        }
        setLayout(layout);
    }

    void FloatButtonStateIndicator::UpdateOnHeartBeat(const OnHeartBeat& hb) {
        context_->PostUITask([=, this]() {
            if (!hb.alt_pressed() && !hb.shift_pressed() && !hb.control_pressed() && !hb.win_pressed()) {
                this->hide();
            } else {
                this->show();
            }
            if (hb.alt_pressed()) {
                alt_item_->show();
            } else {
                alt_item_->hide();
            }
            if (hb.shift_pressed()) {
                shift_item_->show();
            } else {
                shift_item_->hide();
            }
            if (hb.control_pressed()) {
                control_item_->show();
            } else {
                control_item_->hide();
            }
            if (hb.win_pressed()) {
                win_item_->show();
            } else {
                win_item_->hide();
            }
            alt_item_->UpdateState(hb.alt_pressed());
            shift_item_->UpdateState(hb.shift_pressed());
            control_item_->UpdateState(hb.control_pressed());
            win_item_->UpdateState(hb.win_pressed());

            setFixedSize(QSize(65*GetPressedCount(), 30));
        });
    }

    void FloatButtonStateIndicator::paintEvent(QPaintEvent *event) {
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

    int FloatButtonStateIndicator::GetPressedCount() {
        int count = 0;
        if (alt_item_->IsPressed()) {
            count++;
        }
        if (shift_item_->IsPressed()) {
            count++;
        }
        if (control_item_->IsPressed()) {
            count++;
        }
        if (win_item_->IsPressed()) {
            count++;
        }
        return count;
    }

}