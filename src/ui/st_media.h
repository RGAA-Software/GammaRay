//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_ST_MEDIA_H
#define TC_SERVER_STEAM_ST_MEDIA_H

#include "tab_base.h"

namespace tc
{

    class StMedia : public TabBase {
    public:
        StMedia(const std::shared_ptr<Context>& ctx, QWidget *parent);
        ~StMedia() = default;

        void OnTabShow() override;
        void OnTabHide() override;

    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H
