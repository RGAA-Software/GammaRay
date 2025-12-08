//
// Created by RGAA on 18/11/2025.
//

#ifndef GAMMARAYPREMIUM_USERMANAGER_H
#define GAMMARAYPREMIUM_USERMANAGER_H

#include <memory>
#include <string>
#include <vector>

namespace spvr
{
    class SpvrUserDevice;
}

namespace tc
{

    class GrContext;
    class GrSettings;

    class GrUserManager {
    public:
        explicit GrUserManager(const std::shared_ptr<GrContext>& ctx);
        bool Register(const std::string& username, const std::string& password);
        bool Login(const std::string& username, const std::string& password, bool show_dialog = true);
        bool Logout();
        bool ModifyUsername(const std::string& username);
        bool ModifyPassword(const std::string& new_password);
        bool UpdateAvatar(const std::string& avatar_path);
        // user - device
        std::vector<std::shared_ptr<spvr::SpvrUserDevice>> QueryBindDevices(int page, int page_size, bool show_dialog);
        std::shared_ptr<spvr::SpvrUserDevice> AddDeviceForUser(const std::string& device_id);
        std::shared_ptr<spvr::SpvrUserDevice> RemoveDeviceFromUser(const std::string& device_id);

        bool IsLoggedIn();
        std::string GetUserId();
        std::string GetUsername();
        std::string GetPassword();
        std::string GetAvatarPath();
        void Clear();

    private:
        void SaveUserInfo(const std::string& uid, const std::string& username, const std::string& password, const std::string& avatar_path);
        void UpdateUsername(const std::string& username);
        void UpdatePassword(const std::string& password);
        void UpdateAvatarPath(const std::string& avatar_path);

        static std::string KeyUid();
        static std::string KeyUsername();
        static std::string KeyPassword();
        static std::string KeyAvatarPath();

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;

    };

}

#endif //GAMMARAYPREMIUM_USERMANAGER_H
