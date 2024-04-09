//
// Created by hy on 2024/4/9.
//

#include "tab_server.h"

namespace tc
{

    TabServer::TabServer(QWidget *parent) : TabBase(parent) {

    }

    TabServer::~TabServer() {

    }

    void TabServer::OnTabShow() {
        TabBase::OnTabShow();
    }

    void TabServer::OnTabHide() {
        TabBase::OnTabHide();
    }

}