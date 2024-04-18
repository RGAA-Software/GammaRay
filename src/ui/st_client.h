//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_ST_CLIENT_H
#define TC_SERVER_STEAM_ST_CLIENT_H

#include "tab_base.h"

namespace tc
{

    class StClient : public TabBase {
    public:
        StClient(const std::shared_ptr<GrContext>& ctx, QWidget *parent);
        ~StClient() = default;

        void OnTabShow() override;
        void OnTabHide() override;

    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H
