////
//// Created by hy on 2024/1/17.
////
//
//#ifndef TC_SERVER_STEAM_STEAM_MANAGER_H
//#define TC_SERVER_STEAM_STEAM_MANAGER_H
//
//#include <memory>
//#include <QString>
//
//#include "context.h"
//
//#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
//
//#include "entity/game.h"
//
//namespace tc
//{
//
//    class Context;
//
//    // 安装的应用使用信息
//    class InstalledAppIdValue{
//    public:
//        int app_id_;
//        // 应该是app占用的空间大小
//        int app_size_;
//    };
//
//    class InstalledFolder {
//    public:
//        std::string path_;
//        std::vector<InstalledAppIdValue> app_id_value_;
//    };
//
//    class SteamManager {
//    public:
//
//        static std::shared_ptr<SteamManager> Make(const std::shared_ptr<Context>& ctx);
//
//        explicit SteamManager(const std::shared_ptr<Context>& ctx);
//        ~SteamManager();
//
//        bool ScanInstalledGames();
//        std::vector<Game> GetInstalledGames();
//        void DumpGamesInfo();
//        void UpdateAppDetails();
//
//    private:
//        QString ScanInstalledSteamPath();
//        void QueryInstalledApps(HKEY key);
//        void ParseLibraryFolders();
//        void ParseConfigForEachGame();
//        void ScanHeaderImageInAppCache();
//
//    private:
//        std::shared_ptr<Context> context_ = nullptr;
//        QString installed_steam_path_;
//        std::wstring steam_app_base_path_;
//        std::vector<Game> games_;
//        std::vector<InstalledFolder> installed_folders_;
//    };
//
//}
//#endif //TC_SERVER_STEAM_STEAM_MANAGER_H
