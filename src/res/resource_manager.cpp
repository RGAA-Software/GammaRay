//
// Created by hy on 2024/4/12.
//

#include "resource_manager.h"
#include "gr_context.h"

#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QApplication>

#include "tc_common_new/log.h"

namespace tc
{

    ResourceManager::ResourceManager(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
        res_folder_path_ = QApplication::applicationDirPath() +  "/resources";
        QDir res_dir(res_folder_path_);
        if (!res_dir.exists()) {
            if (!res_dir.mkdir(res_folder_path_)) {
                LOGE("create folder failed: {}", res_folder_path_.toStdString());
            }
        }
    }

    void ResourceManager::ExtractIconsIfNeeded() {
        context_->PostTask([this] {
            for (int i = 1; i <= 30; i++) {
                QString target_path = res_folder_path_ + "/" + std::format("{}.png", i).c_str();
                if (QFile::exists(target_path)) {
                    //LOGI("{}.png already exist.", i);
                    continue;
                }
                auto png_name = std::format(":/icons/{}.png", i);
                QImage image;
                image.load(png_name.c_str());
                auto pixmap = QPixmap::fromImage(image);
                pixmap.save(target_path);
            }
        });
    }

}