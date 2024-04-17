//
// Created by RGAA on 2024-04-11.
//

#include <QLabel>
#include "rn_app.h"

#include "widgets/no_margin_layout.h"

namespace tc
{

    RnApp::RnApp(const std::shared_ptr<Context>& ctx, QWidget *parent) : TabBase(ctx, parent) {
        //setStyleSheet("background: #908876;");

        auto root_layout = new NoMarginVLayout();
        auto place_holder = new QLabel();
        place_holder->setFixedWidth(1300);
        root_layout->addWidget(place_holder);
        setLayout(root_layout);

    }

    void RnApp::OnTabShow() {

    }

    void RnApp::OnTabHide() {

    }

}