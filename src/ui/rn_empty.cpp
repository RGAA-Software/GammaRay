//
// Created by RGAA on 2024-04-11.
//

#include "rn_empty.h"

#include "widgets/no_margin_layout.h"
#include <QPixmap>
#include <QLabel>
#include <QPushButton>

namespace tc
{

    RnEmpty::RnEmpty(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();
        root_layout->addStretch();
        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel(this);
            label->setFixedSize(50, 50);
            QString style = R"(background-image: url(%1);
                        background-repeat: no-repeat;
                        background-position: center;
                    )";
            label->setStyleSheet(style.arg(":/icons/ic_empty.svg"));
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addLayout(layout);
        }

        root_layout->addSpacing(10);

        {
            auto layout = new NoMarginHLayout();
            layout->addStretch();
            auto label = new QLabel(this);
            label->setText(tr("NO RUNNING GAME"));
            label->setStyleSheet("font-size: 20px; font-family: ScreenMatrix;");
            layout->addWidget(label);
            layout->addStretch();
            root_layout->addLayout(layout);
        }
        root_layout->addStretch();
        setLayout(root_layout);
    }

    void RnEmpty::OnTabShow() {

    }

    void RnEmpty::OnTabHide() {

    }

}