//
// Created by hy on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_TAB_SERVER_H
#define TC_SERVER_STEAM_TAB_SERVER_H

#include "tab_base.h"

namespace tc
{
    class TabServer : public TabBase {
    public:
        TabServer(QWidget *parent);
        ~TabServer() override;

        void OnTabShow() override;
        void OnTabHide() override;

    };
}

#endif //TC_SERVER_STEAM_TAB_SERVER_H
