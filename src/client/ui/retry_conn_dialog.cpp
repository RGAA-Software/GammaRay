//
// Created by RGAA on 23/06/2025.
//

#include "retry_conn_dialog.h"
#include "tc_label.h"
#include "widget_helper.h"
#include "no_margin_layout.h"
#include "tc_qt_widget/loadings/winstyle/win10circleloadingwidget.h"
#include "tc_qt_widget/loadings/winstyle/win10horizontalloadingwidget.h"

namespace tc
{

    RetryConnDialog::RetryConnDialog(const QString &title, QWidget *parent) : TcCustomTitleBarDialog(title, parent) {
        setFixedSize(480, 320);
        WidgetHelper::SetTitleBarColor(this);
        {
            int size = 50;
            auto lbl_icon = new QLabel(this);
            lbl_icon->setFixedSize(size, size);
            lbl_icon->setScaledContents(true);
            lbl_icon->setStyleSheet(R"(
                border: none;
                border-image: url(:/resources/tc_trans_icon_blue.png);
                background-repeat: no-repeat;
                background-position: center;
            )");
            root_layout_->addSpacing(40);

            auto layout = new NoMarginHLayout();
            layout->addStretch();
            layout->addWidget(lbl_icon);
            layout->addStretch();

            root_layout_->addLayout(layout);
        }

        {
            QString pre_msg = tcTr("id_client_network_disconnected");
            auto lbl_title = new QLabel(pre_msg + std::format(" <span style=\"color:#2979ff;\">{}</span>", "...").c_str());
            lbl_title->setFixedWidth(this->width());
            lbl_title->setAlignment(Qt::AlignCenter);
            lbl_title->setStyleSheet("font-size: 15px; font-weight:bold; color: #555555;");
            root_layout_->addSpacing(25);
            root_layout_->addWidget(lbl_title);
        }

        {
            auto layout = new NoMarginHLayout;
            h_loading_widget_ = new Win10CircleLoadingWidget(this);
            //h_loading_widget_ = new Win10HorizontalLoadingWidget(this);
            //h_loading_widget_->setFixedSize(250, 60);
            h_loading_widget_->setFixedSize(45, 45);
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

            root_layout_->addSpacing(25);
            root_layout_->addLayout(layout);
            root_layout_->addStretch();
        }
    }

    void RetryConnDialog::Exec() {
        h_loading_widget_->start();
        this->exec();
    }

    void RetryConnDialog::Done() {
        h_loading_widget_->stop();
        this->done(0);
    }

}