//
// Created by RGAA on 21/05/2025.
//

#include "start_stream_loading.h"
#include <QResizeEvent>
#include <QPainter>
#include "tc_label.h"
#include "widget_helper.h"
#include "no_margin_layout.h"
#include "tc_common_new/uid_spacer.h"
#include "tc_spvr_client/spvr_stream.h"
#include "client/ct_stream_item_net_type.h"
#include "tc_qt_widget/loadings/winstyle/win10circleloadingwidget.h"
#include "tc_qt_widget/loadings/winstyle/win10horizontalloadingwidget.h"

namespace tc
{

    StartStreamLoading::StartStreamLoading(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<spvr::SpvrStream>& item, const std::string& network_type)
        : QDialog(nullptr) {

        setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);
        //this->setStyleSheet("background:#00000000;");
        setFixedSize(480, 320);
        auto root_layout = new NoMarginVLayout;

        {
            int size = 70;
            auto lbl_icon = new QLabel(this);
            lbl_icon->setFixedSize(size, size);
            lbl_icon->setScaledContents(true);
            lbl_icon->setStyleSheet(R"(
                border: none;
                border-image: url(:/resources/tc_trans_icon_blue.png);
                background-repeat: no-repeat;
                background-position: center;
            )");
            root_layout->addSpacing(60);

            auto layout = new NoMarginHLayout();
            layout->addStretch();
            layout->addWidget(lbl_icon);
            layout->addStretch();

            root_layout->addLayout(layout);
        }

        {
            auto stream_name = [=]() -> std::string {
                if (!item->stream_name_.empty()) {
                    return item->stream_name_;
                }
                else if (!item->remote_device_id_.empty()) {
                    return item->remote_device_id_;
                }
                else if (!item->stream_host_.empty()) {
                    return item->stream_host_;
                }
                else {
                    return "Remote Device";
                }
            }();

            std::string nt_type = "Unknown";
            if (network_type == kStreamItemNtTypeWebSocket) {
                nt_type = tcTr("id_direct").toStdString();
            }
            else if (network_type == kStreamItemNtTypeRelay) {
                nt_type = tcTr("id_relay").toStdString();
            }

            QString pre_msg = tcTr("id_start_streaming");
            auto lbl_title = new QLabel(pre_msg + std::format(R"((<span style="color:#2979ff; font-weight:bold;">{}</span>) <span style="color:#2979ff;">{}</span>)", nt_type, tc::SpaceId(stream_name)).c_str());
            lbl_title->setFixedWidth(this->width());
            lbl_title->setAlignment(Qt::AlignCenter);
            lbl_title->setStyleSheet("font-size: 15px; font-weight:bold; color: #555555;");
            root_layout->addSpacing(20);
            root_layout->addWidget(lbl_title);
        }

        {
            auto layout = new NoMarginHLayout;
            h_loading_widget_ = new Win10HorizontalLoadingWidget(this);
            h_loading_widget_->setFixedSize(250, 60);
            h_loading_widget_->show();
            h_loading_widget_->setBackgroundColor(QColor("#ffffff"));
            h_loading_widget_->setItemLength(8);
            h_loading_widget_->setExtendDuration(240);
            h_loading_widget_->setItemCount(6);
            h_loading_widget_->setDuration(1000);
            QList<QColor> colors;
            colors << QColor("#2962FF") << QColor("#2979FF")
                   << QColor("#448AFF") << QColor("#82B1FF")
                   << QColor("#90CAF9") << QColor("#A0DAF9");
            h_loading_widget_->setItemColors(colors);
            h_loading_widget_->updateFrameData();
            h_loading_widget_->start();

            layout->addStretch();
            layout->addWidget(h_loading_widget_);
            layout->addStretch();

            root_layout->addSpacing(15);
            root_layout->addLayout(layout);
            root_layout->addStretch();
        }

        setLayout(root_layout);

        //
        WidgetHelper::AddShadow(this, 0x666666, 30);
    }

    void StartStreamLoading::resizeEvent(QResizeEvent *event) {
        QWidget::resizeEvent(event);
    }

    void StartStreamLoading::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(0xffffff));

        int offset = 30;
        int radius = 7;
        QRect rect(offset, offset, this->width() - offset*2, this->height()-offset*2);
        painter.drawRoundedRect(rect, radius, radius);
    }

}