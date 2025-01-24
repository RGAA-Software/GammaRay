//
// Created by RGAA on 12/08/2024.
//

#include "key_state_panel.h"
#include "no_margin_layout.h"

namespace tc
{
    constexpr int kBgNormalColor = 0xf2f2f2;
    constexpr int kBgPressedColor = 0xBB0000;
    constexpr int kFontNormalColor = 0x000000;
    constexpr int kFontPressedColor = 0xFFFFFF;

    KeyItem::KeyItem(const QString& txt, QWidget* parent) {
        txt_ = txt;
        setFixedSize(90, 30);
    }

    void KeyItem::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        auto radius = 7;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(pressed_? kBgPressedColor : kBgNormalColor));
        painter.drawRoundedRect(this->rect(), radius, radius);
        painter.setPen(QPen(pressed_? kFontPressedColor : kFontNormalColor));
        painter.drawText(this->rect(), Qt::AlignCenter, txt_);

        if (indicator_) {
            int color = 0x00FF00;
            painter.setPen(QColor(color));
            painter.setBrush(QBrush(color));
            painter.drawRoundedRect(4,4, 6,6, 3,3);
        }
    }

    void KeyItem::UpdateText(const QString& txt) {
        txt_ = txt;
    }

    void KeyItem::UpdateState(bool pressed) {
        pressed_ = pressed;
        update();
    }

    void KeyItem::UpdateIndicator(bool pressed) {
        indicator_ = pressed;
    }

    bool KeyItem::IsPressed() {
        return pressed_;
    }

    // panel
    KeyStatePanel::KeyStatePanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        setFixedWidth(300);
        auto root_layout = new NoMarginVLayout();
        {
            auto layout = new NoMarginHLayout();
            {
                auto item = new KeyItem("ALT");
                alt_item_ = item;
                layout->addWidget(item);
            }
            layout->addSpacing(10);
            {
                auto item = new KeyItem("SHIFT");
                shift_item_ = item;
                layout->addWidget(item);
            }
            layout->addSpacing(10);
            {
                auto item = new KeyItem("CTRL");
                control_item_ = item;
                layout->addWidget(item);
            }
            layout->addStretch();
            root_layout->addLayout(layout);
        }
        root_layout->addSpacing(10);
        {
            auto layout = new NoMarginHLayout();
            {
                auto item = new KeyItem("WIN");
                win_item_ = item;
                layout->addWidget(item);
            }
            layout->addSpacing(10);
            {
                auto item = new KeyItem("CAPS LOCK");
                caps_lock_item_ = item;
                layout->addWidget(item);
            }
            layout->addSpacing(10);
            {
                auto item = new KeyItem("NUM LOCK");
                num_lock_item_ = item;
                layout->addWidget(item);
            }
            layout->addStretch();
            root_layout->addLayout(layout);
        }
        setLayout(root_layout);
    }
}
