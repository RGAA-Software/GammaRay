#include "win_desktop_manager.h"
#include <string>
#include <filesystem>
#include <shlobj.h>
#include "win_message_loop.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"

namespace tc
{

    std::shared_ptr<WinDesktopManager> WinDesktopManager::Make(const std::shared_ptr<RdContext>& ctx) {
        return std::make_shared<WinDesktopManager>(ctx);
    }

    WinDesktopManager::WinDesktopManager(const std::shared_ptr<RdContext>& ctx) {
        message_loop_ = WinMessageLoop::Make(ctx);
        if(message_loop_) {
            message_loop_->Start();
        }
    }

    WinDesktopManager::~WinDesktopManager() {
        if (message_loop_) {
            message_loop_->Stop();
        }
    }

    void WinDesktopManager::UpdateDesktop() {
#if 0   // 测试刷新时候，会偶尔卡住
        LOGI("UpdateDesktop ...");
        ::PostMessageW(GetDesktopWindow(), WM_KEYDOWN, VK_F5, 0);
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
#endif
#if 0
        const int kPathSize = 4096;
        std::wstring wallpaper_path;
        wallpaper_path.resize(kPathSize);
        std::fill(wallpaper_path.begin(), wallpaper_path.end(), L'\0');
        if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, kPathSize, wallpaper_path.data(), 0)) {
            //LOGI("UpdateDesktop wallpaper_path is : {}", StringExt::ToUTF8(wallpaper_path));
            if (std::filesystem::exists(wallpaper_path)) {
                //LOGI("UpdateDesktop wallpaper_path is : {}, exists", StringExt::ToUTF8(wallpaper_path));
                SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, wallpaper_path.data(), SPIF_SENDCHANGE);
                return;
            }
        }
        // 测试发现 壁纸文件删除后, 使用SHChangeNotify 刷新桌面，桌面背景也会便黑色
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
#endif
    }
}
