//
// Created by RGAA on 13/11/2025.
//

#include "skin_official.h"

void* GetInstance() {
    static tc::SkinOfficial impl;
    return (void*)&impl;
}

namespace tc
{

    QString SkinOfficial::GetSkinName() {
        return "Official";
    }

}