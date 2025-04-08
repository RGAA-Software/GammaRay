//
// Created by RGAA on 8/04/2025.
//

#ifndef GAMMARAY_CP_FILE_STRUCT_H
#define GAMMARAY_CP_FILE_STRUCT_H

#include <QString>
#include <cstdint>

namespace tc
{

    struct FileDetailInfo {
        QString mFileName;
        QString mRemotePath;
        uint64_t mFileSize = 0;
    };

}

#endif //GAMMARAY_CP_FILE_STRUCT_H
