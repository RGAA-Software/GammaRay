//
// Created by hy on 2024/4/9.
//

#include <boost/format.hpp>
#include "tab_server.h"
#include "widgets/main_item_delegate.h"

#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollbar>
#include <QMenu>
#include <QAction>
#include <boost/format.hpp>
#include <utility>

namespace tc
{

    TabServer::TabServer(const std::shared_ptr<Context>& ctx, QWidget *parent) : TabBase(ctx, parent) {

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