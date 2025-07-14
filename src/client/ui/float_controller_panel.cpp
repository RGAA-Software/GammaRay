//
// Created by RGAA on 4/07/2024.
//

#include "float_controller_panel.h"
#include "float_icon.h"
#include "tc_common_new/message_notifier.h"
#include "tc_qt_widget/tc_dialog.h"
#include "client/ct_client_context.h"
#include "client/ct_app_message.h"
#include "sized_msg_box.h"
#include "client/ct_settings.h"
#include "no_margin_layout.h"
#include "background_widget.h"
#include "float_sub_control_panel.h"
#include "tc_common_new/log.h"
#include "computer_icon.h"
#include "float_sub_mode_panel.h"
#include "float_sub_display_panel.h"
#include "ct_app_message.h"
#include "tc_dialog.h"
#include "ct_const_def.h"
#include "tc_label.h"

namespace tc
{

    FloatControllerPanel::FloatControllerPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setFixedSize(kInitialWidth, 285);
        this->setStyleSheet("background:#00000000;");
        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargins(root_layout);
        int border_spacing = 5;
        QSize btn_size = QSize(30, 30);
        root_layout->addSpacing(border_spacing);
        {
            auto layout = new QHBoxLayout();

            //屏幕索引切换按钮
            for (int i = 0; i < kMaxGameViewCount; i++) {
                auto ci = new ComputerIcon(ctx, i, this);
                ci->setFixedSize(QSize(26, 26));
                ci->UpdateSelectedState(true);
                ci->Hide();
                layout->addSpacing(3);
                layout->addWidget(ci);
                computer_icons_.push_back(ci);

                ci->SetOnClickListener([=, this](auto w) {
                    bool recording = context_->GetRecording();
                    if (recording) {
                        TcDialog dialog(tr("Tips"), tr("Currently, screen recording is in progress. Switching display is prohibited. If you want to switch displays, please stop the screen recording..."), nullptr);
                        auto pos = mapToGlobal(this->parentWidget()->pos()); 
                        pos.setX(pos.x() + this->parentWidget()->width() / 2 - dialog.width() / 2);
                        pos.setY(pos.y() + this->parentWidget()->height() / 2 - dialog.height() / 2);
                        dialog.move(pos);
                        dialog.exec();
                        return;
                    }
                    HideAllSubPanels();
                    SwitchMonitor(ci);
                });
            }

            //分屏按钮
            {
                auto split_screen_btn = new FloatIcon(ctx, this);
                split_screen_btn_ = split_screen_btn;
                split_screen_btn->setFixedSize(btn_size);
                split_screen_btn->SetIcons(":resources/image/separate_monitor.svg", ":resources/image/separate_monitor.svg");
                layout->addWidget(split_screen_btn);

                split_screen_btn->SetOnClickListener([=, this](QWidget* w) {
                    if (!context_->full_functionality_) {
                        TcDialog dialog(tr("Tips"), tr("You need to upgrade to the Super Edition to use the multi-screen display feature."), nullptr);
                        auto pos = mapToGlobal(this->parentWidget()->pos());
                        pos.setX(pos.x() + this->parentWidget()->width() / 2 - dialog.width() / 2);
                        pos.setY(pos.y() + this->parentWidget()->height() / 2 - dialog.height() / 2);
                        dialog.move(pos);
                        dialog.exec();
                        return;
                    }

                    bool recording = context_->GetRecording();
                    if (recording) {
                        TcDialog dialog(tr("Tips"), tr("Currently, screen recording is in progress. Switching display is prohibited. If you want to switch displays, please stop the screen recording.."), nullptr);
                        auto pos = mapToGlobal(this->parentWidget()->pos());
                        pos.setX(pos.x() + this->parentWidget()->width() / 2 - dialog.width() / 2);
                        pos.setY(pos.y() + this->parentWidget()->height() / 2 - dialog.height() / 2);
                        dialog.move(pos);
                        dialog.exec();
                        return;
                    }
                    CaptureAllMonitor();
                });
            }

            layout->addStretch();

            //声音
            WidgetHelper::ClearMargins(layout);
            {
                auto btn = new FloatIcon(ctx, this);
                audio_btn_ = btn;
                btn->setFixedSize(btn_size);
                btn->SetIcons(":resources/image/ic_volume_off.svg", ":resources/image/ic_volume_on.svg");
                layout->addWidget(btn);

                auto settings = Settings::Instance();
                if (settings->IsAudioEnabled()) {
                    btn->SwitchToSelectedState();
                } else {
                    btn->SwitchToNormalState();
                }

                btn->SetOnClickListener([=, this](QWidget* w) {
                    if (settings->IsAudioEnabled()) {
                        btn->SwitchToNormalState();
                        settings->SetAudioEnabled(false);
                    } else {
                        btn->SwitchToSelectedState();
                        settings->SetAudioEnabled(true);
                    }
                    context_->SendAppMessage(MsgClientFloatControllerPanelUpdate{.update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kAudioStatus});
                });
            }
            {
                auto btn = new FloatIcon(ctx, this);
                btn->setFixedSize(btn_size);
                btn->SetIcons(":resources/image/ic_minimize.svg", "");
                layout->addSpacing(border_spacing);
                layout->addWidget(btn);
                btn->SetOnClickListener([=, this](QWidget* w) {
                    auto top_widget = this->window();
                    if (top_widget) {
                        top_widget->showMinimized();
                    }
                });
            }
            {
                auto btn = new FloatIcon(ctx, this);
                full_screen_btn_ = btn;
                btn->setFixedSize(btn_size);
                btn->SetIcons(":resources/image/ic_fullscreen.svg", ":resources/image/ic_fullscreen_exit.svg");
                layout->addSpacing(border_spacing);
                layout->addWidget(btn);
                btn->SetOnClickListener([=, this](QWidget* w) {
                    auto top_widget = this->window();
                    if (!top_widget) {
                        return;
                    }
                    if (top_widget->isFullScreen()) {
                        top_widget->showNormal();
                        context_->SendAppMessage(MsgClientExitFullscreen{});
                    } else {
                        top_widget->showFullScreen();
                        context_->SendAppMessage(MsgClientFullscreen{});
                        HideAllSubPanels();
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
//                    auto msg_box = SizedMessageBox::MakeOkCancelBox(tr("Stop"), tr("Do you want to STOP the control of remote PC ?"));
//                    if (msg_box->exec() == 0) {
//                        context_->SendAppMessage(MsgClientExitApp {});
//                    }

                    TcDialog dialog(tr("Warning"), tr("Do you want to stop the control of remote PC?"), nullptr);
                    dialog.exec();

                });
            }
            layout->addSpacing(border_spacing);
            root_layout->addLayout(layout);
        }

        auto icon_size = QSize(40, 40);
        int item_left_spacing = border_spacing;
        // work mode
#if 0   // 暂时不启用 work mode, 改用直接设置帧率
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
#endif
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

            auto text = new TcLabel();
            text->SetTextId("id_control");
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

            root_layout->addSpacing(border_spacing);
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

            auto text = new TcLabel();
            text->SetTextId("id_display");
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
                ((SubDisplayPanel*)panel)->SetCaptureMonitorName(monitor_name_);
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

            auto text = new TcLabel();
            text->SetTextId("id_file_transfer");
            text->setStyleSheet(R"(font-weight: bold;)");
            //layout->addSpacing(border_spacing);
            layout->addWidget(text);

            layout->addStretch();
            root_layout->addWidget(widget);

            widget->SetOnClickListener([=, this](QWidget* w) {
                if (file_trans_listener_) {
                    file_trans_listener_(widget);
                }
            });
        }

        // media record
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setFixedSize(this->width(), icon_size.height());
            widget->setLayout(layout);

            auto icon = new QLabel(this);
            icon->setFixedSize(icon_size);
            icon->setStyleSheet(R"( background-image: url(:resources/image/ic_media_record.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addSpacing(item_left_spacing);
            layout->addWidget(icon);

            auto text = new TcLabel();
            text->SetTextId("id_screen_recording");
            media_record_lab_ = text;
            text->setStyleSheet(R"(font-weight: bold;)");
            layout->addWidget(text);

            layout->addStretch();
            root_layout->addWidget(widget);

            widget->SetOnClickListener([=, this](QWidget* w) {
                bool res = context_->GetRecording();
                context_->SetRecording(!res);
                if (!res) {
                    media_record_lab_->setText(tcTr("id_stop_recording"));
                    text->setStyleSheet(R"(font-weight: bold; color: #dc3545;)");
                }
                else {
                    media_record_lab_->setText(tcTr("id_screen_recording"));
                    text->setStyleSheet(R"(font-weight: bold;)");
                }
                context_->SendAppMessage(MsgClientFloatControllerPanelUpdate{ .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kMediaRecordStatus });
                context_->SendAppMessage(MsgClientMediaRecord{});
                context_->SendAppMessage(MsgClientHidePanel{});
            });
        }

        // debug
        {
            auto layout = new NoMarginHLayout();
            auto widget = new BackgroundWidget(ctx, this);
            widget->setFixedSize(this->width(), icon_size.height());
            widget->setLayout(layout);

            auto icon = new QLabel(this);
            icon->setFixedSize(icon_size);
            icon->setStyleSheet(R"( background-image: url(:resources/image/ic_statistics.svg);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
            layout->addSpacing(item_left_spacing);
            layout->addWidget(icon);

            auto text = new TcLabel();
            text->SetTextId("id_statistics");
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

            auto text = new TcLabel();
            text->SetTextId("id_exit");
            text->setStyleSheet(R"(font-weight: bold;)");
            //layout->addSpacing(border_spacing);
            layout->addWidget(text);

            layout->addStretch();
            root_layout->addWidget(widget);

            widget->SetOnClickListener([=, this](QWidget* w) {
                context_->SendAppMessage(MsgClientExitApp {});
            });
        }
        root_layout->addStretch();
        setLayout(root_layout);

     
        msg_listener_->Listen<MsgClientMousePressed>([=, this](const MsgClientMousePressed& msg) {
            this->Hide();
        });

        msg_listener_->Listen<MsgClientCaptureMonitor>([=, this](const MsgClientCaptureMonitor& msg) {
            this->capture_monitor_ = msg;
            context_->PostUITask([=, this]() {
                UpdateCaptureMonitorInfo();
                //继续向下层panel传递显示器信息
                auto panel = GetSubPanel(SubPanelType::kDisplay);
                if (!panel) {
                    panel = (BaseWidget*)(new SubDisplayPanel(ctx, (QWidget*)this->parent()));
                    sub_panels_[SubPanelType::kDisplay] = panel;
                    WidgetHelper::AddShadow(panel, 0xbbbbbb);
                }
                ((SubDisplayPanel*)panel)->SetCaptureMonitorName(monitor_name_);
                ((SubDisplayPanel*)panel)->UpdateMonitorInfo(this->capture_monitor_);
            });
        });

        msg_listener_->Listen<MsgClientMonitorSwitched>([=, this](const MsgClientMonitorSwitched& msg) {
            context_->PostUITask([=, this]() {
                UpdateCapturingMonitor(msg.name_, msg.index_);
            });
        });

        msg_listener_->Listen<MsgClientFullscreen>([=, this](const MsgClientFullscreen& msg) {
            if (full_screen_btn_) {
                full_screen_btn_->SwitchToSelectedState();
            }
        });

        msg_listener_->Listen<MsgClientExitFullscreen>([=, this](const MsgClientExitFullscreen& msg) {
            if (full_screen_btn_) {
                full_screen_btn_->SwitchToNormalState();
            }
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
        //LOGI("UpdateCaptureMonitorInfo, capturing monitor: {}", capture_monitor_.capturing_monitor_name_);
        if (capture_monitor_.monitors_.size() > 1) {
            split_screen_btn_->show();
        }
        else {
            split_screen_btn_->hide();
        }

        int default_appropriate_icons_count = 3;
        if (capture_monitor_.monitors_.size() <= default_appropriate_icons_count) {
            setFixedWidth(kInitialWidth);
        }
        else {
            setFixedWidth(kInitialWidth + (capture_monitor_.monitors_.size() - default_appropriate_icons_count) * 32);
        }
        int index = 0;
        for (const auto& mon : capture_monitor_.monitors_) {
            if (index >= kMaxGameViewCount) {
                break;
            }
            auto ci = computer_icons_[index];
            ci->Show();
            ci->SetMonitorName(mon.name_);
            if (mon.name_ == capture_monitor_.capturing_monitor_name_) {
                ci->UpdateSelectedState(true);
            }
            else {
                ci->UpdateSelectedState(false);
            }
            index++;
        }
        for (auto i = capture_monitor_.monitors_.size(); i < computer_icons_.size(); i++) {
            auto ci = computer_icons_[i];
            ci->Hide();
        }

        // 当远端只有一个屏幕的时候，屏幕图标始终为选中状态
        if (1 == capture_monitor_.monitors_.size()) {
            computer_icons_[0]->UpdateSelectedState(true);
        }
    }

    void FloatControllerPanel::SwitchMonitor(ComputerIcon* w) {
        context_->SendAppMessage(MsgClientSwitchMonitor {
            .name_ = w->GetMonitorName(),
        });
    }

    void FloatControllerPanel::UpdateCapturingMonitor(const std::string& name, int cur_cap_mon_index) {
        if (kCaptureAllMonitorsSign == name) {
            context_->SendAppMessage(MsgClientMultiMonDisplayMode{
                .mode_ = EMultiMonDisplayMode::kSeparate,
            });
            for (const auto& w : computer_icons_) {
                w->UpdateSelectedState(false);
            }
            return;
        }

        context_->SendAppMessage(MsgClientMultiMonDisplayMode{
            .mode_ = EMultiMonDisplayMode::kTab,
            .current_cap_mon_index_ = cur_cap_mon_index
        });

        for (const auto& w: computer_icons_) {
            if (w->GetMonitorName() == name) {
                w->UpdateSelectedState(true);
            } else {
                w->UpdateSelectedState(false);
            }
        }
    }

    void FloatControllerPanel::CaptureAllMonitor() {
        context_->SendAppMessage(MsgClientSwitchMonitor{
           .name_ = kCaptureAllMonitorsSign,
        });
    }

    void FloatControllerPanel::UpdateStatus(const MsgClientFloatControllerPanelUpdate& msg) {
        if (MsgClientFloatControllerPanelUpdate::EUpdate::kAudioStatus == msg.update_type_) {
            auto settings = Settings::Instance();
            if (settings->IsAudioEnabled()) {
                audio_btn_->SwitchToSelectedState();
            }
            else {
                audio_btn_->SwitchToNormalState();
            }
        }
        else if (MsgClientFloatControllerPanelUpdate::EUpdate::kMediaRecordStatus == msg.update_type_) {
            bool res = context_->GetRecording();
            if (res) {
                media_record_lab_->setText(tcTr("id_stop_recording"));
            }
            else {
                media_record_lab_->setText(tcTr("id_screen_recording"));
            }
        }
    }

    void FloatControllerPanel::SetMainControl() {
        is_main_control_ = true;
    }

    void FloatControllerPanel::SetMonitorName(const std::string& mon_name) {
        monitor_name_ = mon_name;
    }
}