//
// Created by RGAA on 2024/4/12.
//

#ifndef TC_SERVER_STEAM_RESOURCE_MANAGER_H
#define TC_SERVER_STEAM_RESOURCE_MANAGER_H

#include <string>
#include <memory>
#include <QString>

namespace tc
{

    class GrContext;

    class ResourceManager {
    public:

        explicit ResourceManager(const std::shared_ptr<GrContext>& ctx);
        void ExtractIconsIfNeeded();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        QString res_folder_path_;
    };

}

#endif //TC_SERVER_STEAM_RESOURCE_MANAGER_H
