//
// Created by RGAA on 28/05/2025.
//

#include "st_security_visitor.h"
#include "no_margin_layout.h"
#include "tc_pushbutton.h"

namespace tc
{

    StSecurityVisitor::StSecurityVisitor(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();

        auto button = new TcPushButton(this);
        button->setText("Visitor");

        root_layout->addWidget(button);

        setLayout(root_layout);
    }

}