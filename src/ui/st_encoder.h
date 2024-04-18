//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_ST_ENCODER_H
#define TC_SERVER_STEAM_ST_ENCODER_H

#include "tab_base.h"

namespace tc
{

    class StEncoder : public TabBase {
    public:
        StEncoder(const std::shared_ptr<GrContext>& ctx, QWidget *parent);
        ~StEncoder() = default;

        void OnTabShow() override;
        void OnTabHide() override;

    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H
