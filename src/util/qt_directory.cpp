//
// Created by hy on 2024/6/4.
//
#include "qt_directory.h"
#include <QDir>

#include <format>

namespace tc
{

    void QtDirectory::OpenDir(const std::string& path) {
        auto target_path = std::format("file:///{}", path);
        QDesktopServices::openUrl(QUrl(target_path.c_str()));
    }

    void QtDirectory::CreateDir(const std::string& path) {
        QDir dir(path.c_str());
        if (!dir.exists()) {
            dir.mkpath(path.c_str());
        }
    }

}