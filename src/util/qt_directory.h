//
// Created by hy on 2024/6/4.
//

#ifndef GAMMARAY_QT_DIRECTORY_H
#define GAMMARAY_QT_DIRECTORY_H

#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QString>
#include <string>

namespace tc
{

    class QtDirectory {
    public:
        static void OpenDir(const std::string& path);
        static void CreateDir(const std::string& path);
    };

}


#endif //GAMMARAY_QT_DIRECTORY_H
