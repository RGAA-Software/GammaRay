//
// Created by RGAA on 13/11/2025.
//

#include "skin_opensource.h"

void* GetInstance() {
    static tc::SkinOpenSource impl;
    return (void*)&impl;
}

namespace tc
{

//    QString SkinOpenSource::GetSkinName() {
//        return "OpenSource";
//    }

}