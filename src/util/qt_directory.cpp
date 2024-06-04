//
// Created by hy on 2024/6/4.
//
#include "qt_directory.h"

#include <format>

namespace tc
{

    void QtDirectory::OpenDir(const std::string& path) {
        auto target_path = std::format("file:///{}", path);
        QDesktopServices::openUrl(QUrl(target_path.c_str()));
    }

}