//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_ST_NETWORK_H
#define TC_SERVER_STEAM_ST_NETWORK_H

#include "tab_base.h"

namespace tc
{

    class StNetwork : public TabBase {
    public:
        StNetwork(const std::shared_ptr<GrContext>& ctx, QWidget *parent);
        ~StNetwork() = default;

        void OnTabShow() override;
        void OnTabHide() override;

    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H
