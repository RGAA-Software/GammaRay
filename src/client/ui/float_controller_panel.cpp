//
// Created by RGAA on 4/07/2024.
//

#include "float_controller_panel.h"
#include "float_icon.h"
#include "tc_common_new/message_notifier.h"
#include "client_context.h"
#include "app_message.h"
#include "sized_msg_box.h"
#include "settings.h"
#include "no_margin_layout.h"
#include "background_widget.h"
#include "float_sub_control_panel.h"
#include "tc_common_new/log.h"
#include "computer_icon.h"
#include "float_sub_mode_panel.h"
#include "float_sub_display_panel.h"

namespace tc
{

    FloatControllerPanel::FloatControllerPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setFixedSize(230, 285);
        this->setStyleSheet("background:#00000000;");
        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargin(root_layout);
        int border_spacing = 5;
        QSize btn_size = QSize(30, 30);
        root_layout->addSpacing(border_spacing);
        {
            auto layout = new QHBoxLayout();

            for (int i = 0; i < 4; i++) {
                auto ci = new ComputerIcon(ctx, i, this);
                ci->setFixedSize(QSize(26, 26));
                ci->UpdateSelectedState(true);
                layout->addSpacing(3);
                layout->addWidget(ci);
                computer_icons_.insert({i, ci});

                ci->SetOnClickListener([=, this](auto w) {
                    HideAllSubPanels();
                    SwitchMonitor(ci);
                });
            }

            layout->addStretch();
            WidgetHelper::ClearMargin(layout);
            {
                auto btn = new FloatIcon(ctx, this);
                btn->setFixedSize(btn_size);
                btn->SetIcons(":resources/image/ic_volume_on.svg", ":resources/image/ic_volume_off.svg");
                layout->addWidget(btn);

                auto settings = Settings::Instance();
                if (settings->IsAudioEnabled()) {
                    btn->SwitchToNormalState();
                } else {
                    btn->SwitchToSelectedState();
                }

                btn->SetOnClickListener([=, this](QWidget* w) {
                    if (settings->IsAudioEnabled()) {
                        btn->SwitchToSelectedState();
                        settings->SetTempAudioEnabled(false);
                    } else {
                        btn->SwitchToNormalState();
                        settings->SetTempAudioEnabled(true);
                    }
                });
            }
            {
                auto btn = new FloatIcon(ctx, this);
                btn->setFixedSize(btn_size);
                btn->SetIcons(":resources/image/ic_minimize.svg", "");
                layout->addSpacing(border_spacing);
                layout->addWidget(btn);
                btn->SetOnClickListener([=, this](QWidget* w) {
                    parent->showMinimized();
                });
            }
            {
                auto btn = new FloatIcon(ctx, this);
                btn->setFixedSize(btn_size);
                btn->SetIcons(":resources/image/ic_fullscreen.svg", ":resources/image/ic_fullscreen_exit.svg");
                layout->addSpacing(border_spacing);
                layout->addWidget(btn);
                btn->SetOnClickListener([=, this](QWidget* w) {
                    if (parent->isFullScreen()) {
                        parent->showNormal();
                        btn->SwitchToNormalState();
                    } else {
                        parent->showFullScreen();
                        btn->SwitchToSelectedState();
                    }
                    this->hide();
                });
            }
            if (0) {
                auto btn = new FloatIcon(ctx, this);
                btn->setFixedSize(btn_size);
                btn->SetIcons(":resources/image/ic_close.svg", "");
                layout->addSpacing(border_spacing);
                layout->addWidget(btn);
                btn->SetOnClickListener([=, this](QWidget* w) {
                    auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Stop"), tr("Do you want to STOP the control of remote PC ?"));
                    if (msg_box->exec() == 0) {
                        context_->SendAppMessage(ExitAppMessage {});
                    }
                });
            }
            layout->addSpacing(border_spacing);
            root_layout->addLayout(layout);
        }

        auto icon_size = QSize(40, 40);
        int item_left_spacing = border_spacing;
        // work mode
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setFixedSize(this->width(), icon_size.height());
            widget->setLayout(layout);
            root_layout->addSpacing(border_spacing);

            auto icon = new QLabel(this);
            icon->setFixedSize(icon_size);
            icon->setStyleSheet(R"( background-image: url(:resources/image/ic_mode.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addSpacing(item_left_spacing);
            layout->addWidget(icon);

            auto text = new QLabel();
            text->setText(tr("Mode"));
            text->setStyleSheet(R"(font-weight: bold;)");
            //layout->addSpacing(border_spacing);
            layout->addWidget(text);

            layout->addStretch();

            auto icon_right = new QLabel(this);
            icon_right->setFixedSize(icon_size);
            icon_right->setStyleSheet(R"( background-image: url(:resources/image/ic_arrow_right_2.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addWidget(icon_right);
            layout->addSpacing(border_spacing);

            root_layout->addWidget(widget);
            // click
            widget->SetOnClickListener([=, this](auto w) {
                auto panel = GetSubPanel(SubPanelType::kWorkMode);
                if (!panel) {
                    panel = (BaseWidget*)(new SubModePanel(ctx, (QWidget*)this->parent()));
                    sub_panels_[SubPanelType::kWorkMode] = panel;
                    WidgetHelper::AddShadow(panel, 0xbbbbbb);
                }
                auto item_pos = this->mapTo((QWidget*)this->parent(), w->pos());
                HideAllSubPanels();
                panel->setGeometry(this->pos().x() + this->width(), item_pos.y(), panel->width(), panel->height());
                panel->show();
            });
        }
        // control
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setFixedSize(this->width(), icon_size.height());
            widget->setLayout(layout);

            auto icon = new QLabel(this);
            icon->setFixedSize(icon_size);
            icon->setStyleSheet(R"( background-image: url(:resources/image/ic_control.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addSpacing(item_left_spacing);
            layout->addWidget(icon);

            auto text = new QLabel();
            text->setText(tr("Control"));
            text->setStyleSheet(R"(font-weight: bold;)");
            //layout->addSpacing(border_spacing);
            layout->addWidget(text);

            layout->addStretch();

            auto icon_right = new QLabel(this);
            icon_right->setFixedSize(icon_size);
            icon_right->setStyleSheet(R"( background-image: url(:resources/image/ic_arrow_right_2.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addWidget(icon_right);
            layout->addSpacing(border_spacing);

            root_layout->addWidget(widget);

            // click
            widget->SetOnClickListener([=, this](auto w) {
                auto panel = GetSubPanel(SubPanelType::kControl);
                if (!panel) {
                    panel = (BaseWidget*)(new SubControlPanel(ctx, (QWidget*)this->parent()));
                    sub_panels_[SubPanelType::kControl] = panel;
                    WidgetHelper::AddShadow(panel, 0xbbbbbb);
                }
                auto item_pos = this->mapTo((QWidget*)this->parent(), w->pos());
                HideAllSubPanels();
                panel->setGeometry(this->pos().x() + this->width(), item_pos.y(), panel->width(), panel->height());
                panel->show();
            });
        }
        // Display
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setFixedSize(this->width(), icon_size.height());
            widget->setLayout(layout);

            auto icon = new QLabel(this);
            icon->setFixedSize(icon_size);
            icon->setStyleSheet(R"( background-image: url(:resources/image/ic_desktop.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addSpacing(item_left_spacing);
            layout->addWidget(icon);

            auto text = new QLabel();
            text->setText(tr("Display"));
            text->setStyleSheet(R"(font-weight: bold;)");
            //layout->addSpacing(border_spacing);
            layout->addWidget(text);
            layout->addStretch();

            auto icon_right = new QLabel(this);
            icon_right->setFixedSize(icon_size);
            icon_right->setStyleSheet(R"( background-image: url(:resources/image/ic_arrow_right_2.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addWidget(icon_right);
            layout->addSpacing(border_spacing);

            root_layout->addWidget(widget);

            // click
            widget->SetOnClickListener([=, this](auto w) {
                auto panel = GetSubPanel(SubPanelType::kDisplay);
                if (!panel) {
                    panel = (BaseWidget*)(new SubDisplayPanel(ctx, (QWidget*)this->parent()));
                    sub_panels_[SubPanelType::kDisplay] = panel;
                    WidgetHelper::AddShadow(panel, 0xbbbbbb);
                }
                auto item_pos = this->mapTo((QWidget*)this->parent(), w->pos());
                HideAllSubPanels();
                ((SubDisplayPanel*)panel)->UpdateMonitorInfo(this->capture_monitor_);
                panel->setGeometry(this->pos().x() + this->width(), item_pos.y(), panel->width(), panel->height());
                panel->show();
            });

        }
        // file transfer
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setFixedSize(this->width(), icon_size.height());
            widget->setLayout(layout);

            auto icon = new QLabel(this);
            icon->setFixedSize(icon_size);
            icon->setStyleSheet(R"( background-image: url(:resources/image/ic_file_transfer.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addSpacing(item_left_spacing);
            layout->addWidget(icon);

            auto text = new QLabel();
            text->setText(tr("File Transfer ="));
            text->setStyleSheet(R"(font-weight: bold;)");
            //layout->addSpacing(border_spacing);
            layout->addWidget(text);

            layout->addStretch();
            root_layout->addWidget(widget);
        }
        // debug
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setFixedSize(this->width(), icon_size.height());
            widget->setLayout(layout);

            auto icon = new QLabel(this);
            icon->setFixedSize(icon_size);
            icon->setStyleSheet(R"( background-image: url(:resources/image/ic_debug_off.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addSpacing(item_left_spacing);
            layout->addWidget(icon);

            auto text = new QLabel();
            text->setText(tr("Debug"));
            text->setStyleSheet(R"(font-weight: bold;)");
            //layout->addSpacing(border_spacing);
            layout->addWidget(text);

            layout->addStretch();
            root_layout->addWidget(widget);

            widget->SetOnClickListener([=, this](QWidget* w) {
                if (debug_listener_) {
                    debug_listener_(widget);
                }
            });
        }
        // Close
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setFixedSize(this->width(), icon_size.height());
            widget->setLayout(layout);

            auto icon = new QLabel(this);
            icon->setFixedSize(icon_size);
            icon->setStyleSheet(R"( background-image: url(:resources/image/ic_close.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addSpacing(item_left_spacing);
            layout->addWidget(icon);

            auto text = new QLabel();
            text->setText(tr("Exit"));
            text->setStyleSheet(R"(font-weight: bold;)");
            //layout->addSpacing(border_spacing);
            layout->addWidget(text);

            layout->addStretch();
            root_layout->addWidget(widget);

            widget->SetOnClickListener([=, this](QWidget* w) {
                auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Exit"), tr("Do you want to stop controlling of remote PC ?"));
                if (msg_box->exec() == 0) {
                    context_->SendAppMessage(ExitAppMessage {});
                }
            });
        }
        root_layout->addStretch();
        setLayout(root_layout);

        CreateMsgListener();
        msg_listener_->Listen<MousePressedMessage>([=, this](const MousePressedMessage& msg) {
            this->Hide();
        });

        msg_listener_->Listen<CaptureMonitorMessage>([=, this](const CaptureMonitorMessage& msg) {
            this->capture_monitor_ = msg;
            context_->PostUITask([=, this]() {
                UpdateCaptureMonitorInfo();
            });
        });

        msg_listener_->Listen<MonitorSwitchedMessage>([=, this](const MonitorSwitchedMessage& msg) {
            context_->PostUITask([=, this]() {
                UpdateCapturingMonitor(msg.index_, msg.name_);
            });
        });
    }

    void FloatControllerPanel::paintEvent(QPaintEvent *event) {
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

    BaseWidget* FloatControllerPanel::GetSubPanel(const SubPanelType& type) {
        if (sub_panels_.count(type) > 0) {
            return sub_panels_[type];
        }
        return nullptr;
    }

    void FloatControllerPanel::HideAllSubPanels() {
        for (const auto& [k, v] : sub_panels_) {
            v->Hide();
        }
    }

    void FloatControllerPanel::Hide() {
        this->hide();
        this->HideAllSubPanels();
    }

    void FloatControllerPanel::UpdateCaptureMonitorInfo() {
        // hide extra icons
        for (const auto& [idx, icon] : computer_icons_) {
            bool find = false;
            for (const auto& mon : capture_monitor_.monitors_) {
                if (mon.index_ == idx) {
                    find = true;
                }
            }
            if (!find) {
                icon->hide();
            }
        }
        // update monitor info
        for (const auto& item: capture_monitor_.monitors_) {
            if (computer_icons_.count(item.index_) > 0) {
                bool selected = false;
                if (item.index_ == capture_monitor_.capturing_monitor_index_) {
                    selected = true;
                }
                computer_icons_[item.index_]->SetMonitorName(item.name_);
                computer_icons_[item.index_]->UpdateSelectedState(selected);
            }
        }
    }

    void FloatControllerPanel::SwitchMonitor(ComputerIcon* w) {
        context_->SendAppMessage(SwitchMonitorMessage {
            .index_ = w->GetMonitorIndex(),
            .name_ = w->GetMonitorName(),
        });
    }

    void FloatControllerPanel::UpdateCapturingMonitor(int index, const std::string& name) {
        for (const auto& [idx, w]: computer_icons_) {
            if (idx == index) {
                w->UpdateSelectedState(true);
            } else {
                w->UpdateSelectedState(false);
            }
        }
    }

    int FloatControllerPanel::GetCurrentMonitorIndex() {
        for (const auto& [idx, w]: computer_icons_) {
            if (w->IsSelected()) {
                return idx;
            }
        }
        return 0;
    }
}