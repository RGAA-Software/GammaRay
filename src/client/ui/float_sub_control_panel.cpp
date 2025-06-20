//
// Created by RGAA on 17/08/2024.
//

#include "float_sub_control_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "client/ct_settings.h"
#include "client/ct_client_context.h"
#include "client/ct_app_message.h"
#include "tc_label.h"
#include <QLabel>

namespace tc
{

    SubControlPanel::SubControlPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        setFixedSize(200, 170);
        auto item_height = 38;
        auto border_spacing = 10;
        auto item_size = QSize(this->width(), item_height);
        auto root_layout = new NoMarginVLayout();

        //Clipboard
        {
            auto layout = new NoMarginHLayout();
            auto widget = new QWidget(this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new TcLabel();
            lbl->SetTextId("id_clipboard");
            lbl->setStyleSheet(R"(font-weight:bold;)");
            layout->addSpacing(border_spacing*2);
            layout->addWidget(lbl);

            layout->addStretch();

            auto sb = new SwitchButton(this);
            clibpboard_btn_ = sb;
            sb->setFixedSize(35, 20);
            sb->SetStatus(Settings::Instance()->clipboard_on_);
            layout->addWidget(sb);
            sb->SetClickCallback([=,this](bool enabled) {
                Settings::Instance()->SetClipboardEnabled(enabled);
                this->context_->SendAppMessage(FloatControllerPanelUpdateMessage{.update_type_ = FloatControllerPanelUpdateMessage::EUpdate::kClipboardSharedStatus});
            });

            layout->addSpacing(border_spacing);

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);
        }

        // Only Viewing
        {
            auto layout = new NoMarginHLayout();
            auto widget = new QWidget(this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new TcLabel();
            lbl->SetTextId("id_only_viewing");
            lbl->setStyleSheet(R"(font-weight:bold;)");
            layout->addSpacing(border_spacing*2);
            layout->addWidget(lbl);

            layout->addStretch();

            auto sb = new SwitchButton(this);
            sb->setFixedSize(35, 20);
            sb->SetStatus(Settings::Instance()->only_viewing_);
            layout->addWidget(sb);
            sb->SetClickCallback([=,this](bool enabled) {
                Settings::Instance()->only_viewing_ = enabled;
            });

            layout->addSpacing(border_spacing);

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);
        }

        //Ctrl + Alt + Delete
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new TcLabel();
            lbl->setText(tr("Ctrl + Alt + Delete"));
            lbl->setStyleSheet(R"(font-weight:bold;)");
            layout->addSpacing(border_spacing*2);
            layout->addWidget(lbl);
            layout->addStretch();

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);

            widget->SetOnClickListener([=, this](QWidget* w) {
                RequestCtrlAltDelete();
            });
        }

        // refresh
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setLayout(layout);
            widget->setFixedSize(item_size);
            layout->addWidget(widget);

            auto lbl = new TcLabel();
            lbl->SetTextId("id_refresh_screen");
            lbl->setStyleSheet(R"(font-weight:bold;)");
            layout->addSpacing(border_spacing * 2);
            layout->addWidget(lbl);
            layout->addStretch();

            root_layout->addSpacing(5);
            root_layout->addWidget(widget);

            widget->SetOnClickListener([=, this](QWidget* w) {
                RequestRefreshDesktop();
            });
        
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

    void SubControlPanel::RequestCtrlAltDelete() {
        context_->SendAppMessage(MsgCtrlAltDelete{});
    }

    void SubControlPanel::UpdateStatus(const FloatControllerPanelUpdateMessage& msg) {
        if (FloatControllerPanelUpdateMessage::EUpdate::kClipboardSharedStatus == msg.update_type_) {
            clibpboard_btn_->SetStatus(Settings::Instance()->clipboard_on_);
        }
    }

    void SubControlPanel::RequestRefreshDesktop() {
        context_->SendAppMessage(MsgHardUpdateDesktop{});
    }
}