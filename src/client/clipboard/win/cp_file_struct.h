//
// Created by RGAA on 8/04/2025.
//

#ifndef GAMMARAY_CP_FILE_STRUCT_H
#define GAMMARAY_CP_FILE_STRUCT_H

#include <QString>
#include <cstdint>
#include "tc_message.pb.h"

namespace tc
{

    class ClipboardFileWrapper {
    public:
        ClipboardFile file_;
    };

}

#endif //GAMMARAY_CP_FILE_STRUCT_H
