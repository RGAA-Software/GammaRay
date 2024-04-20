//
// Created by RGAA on 2024-04-11.
//

#include <QLabel>
#include "rn_app.h"

#include "widgets/no_margin_layout.h"
#include "stat_chart.h"

namespace tc
{

    RnApp::RnApp(const std::shared_ptr<GrContext>& ctx, QWidget *parent) : TabBase(ctx, parent) {
        auto root_layout = new NoMarginVLayout();
        auto place_holder = new QLabel();
        place_holder->setFixedWidth(1300);
        root_layout->addWidget(place_holder);

        {
            auto chart = new StatChart(ctx, {"Capture+Encode", "Decode"}, this);
            chart->setFixedSize(1100, 300);
            root_layout->addWidget(chart);
        }

        root_layout->addStretch();

        setLayout(root_layout);
    }

    void RnApp::OnTabShow() {

    }

    void RnApp::OnTabHide() {

    }

}