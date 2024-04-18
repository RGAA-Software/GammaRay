//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_ST_INPUT_H
#define TC_SERVER_STEAM_ST_INPUT_H

#include "tab_base.h"

namespace tc
{

    class StInput : public TabBase {
    public:
        StInput(const std::shared_ptr<GrContext>& ctx, QWidget *parent);
        ~StInput() = default;

        void OnTabShow() override;
        void OnTabHide() override;

    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H
