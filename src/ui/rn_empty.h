//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_RNST_EMPTY_H
#define TC_SERVER_STEAM_RNST_EMPTY_H

#include "tab_base.h"

namespace tc
{

    class RnEmpty : public TabBase {
    public:
        RnEmpty(const std::shared_ptr<Context>& ctx, QWidget *parent);
        ~RnEmpty() = default;

        void OnTabShow() override;
        void OnTabHide() override;

    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H
