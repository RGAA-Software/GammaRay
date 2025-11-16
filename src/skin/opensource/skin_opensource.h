//
// Created by RGAA on 13/11/2025.
//

#ifndef GAMMARAYPREMIUM_SKIN_OFFICIAL_H
#define GAMMARAYPREMIUM_SKIN_OFFICIAL_H

#include "skin/interface/skin_interface.h"

namespace tc
{
    class SkinOpenSource /*: public SkinInterface*/ {
    public:
        //QString GetSkinName() override;
    };
}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAYPREMIUM_SKIN_OFFICIAL_H
