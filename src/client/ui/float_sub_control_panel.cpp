//
// Created by RGAA on 17/08/2024.
//

#include "float_sub_control_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "settings.h"
#include <QLabel>

namespace tc
{

    SubControlPanel::SubControlPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        setFixedSize(200, 130);
        auto item_height = 38;
        auto border_spacing = 10;
        auto item_size = QSize(this->width(), item_height);
        auto root_layout = new NoMarginVLayout();

        {
            auto layout = new NoMarginHLayout();
            auto widget = new QWidget(this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new QLabel();
            lbl->setText(tr("Clipboard"));
            layout->addSpacing(border_spacing);
            layout->addWidget(lbl);

            layout->addStretch();

            auto sb = new SwitchButton(this);
            sb->setFixedSize(35, 20);
            sb->SetStatus(Settings::Instance()->clipboard_on_);
            layout->addWidget(sb);
            sb->SetClickCallback([](bool enabled) {
                Settings::Instance()->clipboard_on_ = enabled;
            });

            layout->addSpacing(border_spacing);

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);
        }

        root_layout->addStretch();
        setLayout(root_layout);
    }

    void SubControlPanel::paintEvent(QPaintEvent *event) {
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