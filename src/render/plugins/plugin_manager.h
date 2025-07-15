//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_PLUGIN_MANAGER_H
#define GAMMARAY_PLUGIN_MANAGER_H

#include <memory>
#include <string>
#include <map>
#include <QLibrary>
#include "plugin_ids.h"
#include "tc_common_new/concurrent_hashmap.h"
#include "plugin_interface/gr_plugin_settings_info.h"

namespace tc
{

    class RdContext;
    class RdSettings;
    class RdApplication;
    class PluginEventRouter;
    class GrPluginInterface;
    class GrStreamPlugin;
    class GrVideoEncoderPlugin;
    class GrNetPlugin;
    class GrMonitorCapturePlugin;
    class GrDataProviderPlugin;
    class GrAudioEncoderPlugin;
    class GrFrameCarrierPlugin;
    class GrFrameProcessorPlugin;
    class GrConnectedClientInfo;

    class PluginManager {
    public:
        static std::shared_ptr<PluginManager> Make(const std::shared_ptr<RdApplication>& app);

        explicit PluginManager(const std::shared_ptr<RdApplication>& app);

        void LoadAllPlugins();
        void RegisterPluginEventsCallback();
        void ReleaseAllPlugins();
        void ReleasePlugin(const std::string& name);

        GrPluginInterface* GetPluginById(const std::string& id);
        GrVideoEncoderPlugin* GetFFmpegEncoderPlugin();
        GrVideoEncoderPlugin* GetNvencEncoderPlugin();
        GrVideoEncoderPlugin* GetAmfEncoderPlugin();
        GrMonitorCapturePlugin* GetDDACapturePlugin();
        GrMonitorCapturePlugin* GetGdiCapturePlugin();
        GrDataProviderPlugin* GetMockVideoStreamPlugin();
        GrDataProviderPlugin* GetAudioCapturePlugin();
        GrAudioEncoderPlugin* GetAudioEncoderPlugin();
        GrPluginInterface* GetFileTransferPlugin();
        GrPluginInterface* GetClipboardPlugin();
        GrPluginInterface* GetRtcPlugin();
        GrNetPlugin* GetUdpPlugin();
        GrNetPlugin* GetRelayPlugin();
        GrFrameCarrierPlugin* GetFrameCarrierPlugin();
        GrFrameProcessorPlugin* GetFrameResizePlugin();
        GrPluginInterface* GetEventsReplayerPlugin();
        int64_t GetQueuingMediaMsgCountInNetPlugins();
        int64_t GetQueuingFtMsgCountInNetPlugins();
        int GetTotalConnectedClientsCount();
        std::vector<std::shared_ptr<GrConnectedClientInfo>> GetConnectedClientsInfo();

        void VisitAllPlugins(const std::function<void(GrPluginInterface*)>&& visitor);
        void VisitStreamPlugins(const std::function<void(GrStreamPlugin*)>&& visitor);
        void VisitUtilPlugins(const std::function<void(GrPluginInterface*)>&& visitor);
        void VisitEncoderPlugins(const std::function<void(GrVideoEncoderPlugin*)>&& visitor);
        void VisitNetPlugins(const std::function<void(GrNetPlugin*)>&& visitor);
        void DumpPluginInfo();

        void On1Second();

        // from render panel -> render
        void SyncPluginSettingsInfo(const GrPluginSettingsInfo& info);

        // is GDI
        bool IsGDIMonitorCapturePlugin(GrMonitorCapturePlugin* plugin);
        // is DDA
        bool IsDDAMonitorCapturePlugin(GrMonitorCapturePlugin* plugin);

    private:
        RdSettings* settings_ = nullptr;
        std::shared_ptr<RdApplication> app_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::map<std::string, GrPluginInterface*> plugins_;
        std::shared_ptr<PluginEventRouter> evt_router_ = nullptr;
    };

}

#endif //GAMMARAY_PLUGIN_MANAGER_H
