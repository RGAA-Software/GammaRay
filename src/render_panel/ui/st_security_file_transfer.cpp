//
// Created by RGAA on 28/05/2025.
//

#include "st_security_file_transfer.h"
#include "no_margin_layout.h"
#include "tc_pushbutton.h"

namespace tc
{

    StSecurityFileTransfer::StSecurityFileTransfer(const std::shared_ptr<GrApplication>& app, QWidget *parent) : TabBase(app, parent) {
        auto root_layout = new NoMarginVLayout();

        auto button = new TcPushButton(this);
        button->setText("File Transfer");

        root_layout->addWidget(button);

        setLayout(root_layout);
    }

}