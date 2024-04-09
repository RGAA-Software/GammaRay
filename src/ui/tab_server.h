//
// Created by hy on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_TAB_SERVER_H
#define TC_SERVER_STEAM_TAB_SERVER_H

#include "tab_base.h"

#include <QListWidget>
#include <QListWidgetItem>

namespace tc
{
    class TabServer : public TabBase {
    public:
        explicit TabServer(const std::shared_ptr<Context>& ctx, QWidget *parent);
        ~TabServer() override;

        void OnTabShow() override;
        void OnTabHide() override;



    };
}

#endif //TC_SERVER_STEAM_TAB_SERVER_H
