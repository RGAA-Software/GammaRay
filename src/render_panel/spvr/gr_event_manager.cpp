//
// Created by RGAA on 23/01/2026.
//

#include "gr_event_manager.h"
#include "tc_spvr_client/spvr_event.h"
#include "tc_spvr_client/spvr_event_api.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_application.h"
#include "render_panel/user/gr_user_manager.h"
#include "tc_common_new/log.h"

namespace tc
{

    GrEventManager::GrEventManager(const std::shared_ptr<GrContext>& context) {
        context_ = context;
        settings_ = GrSettings::Instance();
    }

    bool GrEventManager::AddCpuEvent(int cpu_usage) {
        const auto host = settings_->GetSpvrServerHost();
        const auto port = settings_->GetSpvrServerPort();
        const auto appkey = grApp->GetAppkey();
        if (host.empty() || port <= 0 || appkey.empty()) {
            return false;
        }
        if (!user_mgr_) {
            user_mgr_ = grApp->GetUserManager();
            if (!user_mgr_) {
                LOGE("No user manager!");
                return false;
            }
        }

        std::string device_id = settings_->GetDeviceId();
        std::string device_ip = context_->GetFirstAvailableIp();
        std::string device_name = settings_->GetDeviceName();
        std::string uid = user_mgr_->GetUserId();
        std::string username = user_mgr_->GetUsername();
        const auto event = SpvrEvent::CpuOverload(device_id, device_ip, device_name, uid, username, cpu_usage);
        if (const auto r = SpvrEventApi::AddEvent(host, port, appkey, event); r.has_value()) {
            return true;
        }
        return false;
    }

    bool GrEventManager::AddMemoryEvent(int memory_usage) {
        const auto host = settings_->GetSpvrServerHost();
        const auto port = settings_->GetSpvrServerPort();
        const auto appkey = grApp->GetAppkey();
        if (host.empty() || port <= 0 || appkey.empty()) {
            return false;
        }
        if (!user_mgr_) {
            user_mgr_ = grApp->GetUserManager();
            if (!user_mgr_) {
                LOGE("No user manager!");
                return false;
            }
        }

        std::string device_id = settings_->GetDeviceId();
        std::string device_ip = context_->GetFirstAvailableIp();
        std::string device_name = settings_->GetDeviceName();
        std::string uid = user_mgr_->GetUserId();
        std::string username = user_mgr_->GetUsername();
        const auto event = SpvrEvent::MemoryOverload(device_id, device_ip, device_name, uid, username, memory_usage);
        if (const auto r = SpvrEventApi::AddEvent(host, port, appkey, event); r.has_value()) {
            return true;
        }
        return false;
    }

    bool GrEventManager::AddDiskEvent(int disk_usage, const std::string& disk_path) {
        const auto host = settings_->GetSpvrServerHost();
        const auto port = settings_->GetSpvrServerPort();
        const auto appkey = grApp->GetAppkey();
        if (host.empty() || port <= 0 || appkey.empty()) {
            return false;
        }
        if (!user_mgr_) {
            user_mgr_ = grApp->GetUserManager();
            if (!user_mgr_) {
                LOGE("No user manager!");
                return false;
            }
        }

        std::string device_id = settings_->GetDeviceId();
        std::string device_ip = context_->GetFirstAvailableIp();
        std::string device_name = settings_->GetDeviceName();
        std::string uid = user_mgr_->GetUserId();
        std::string username = user_mgr_->GetUsername();
        const auto event = SpvrEvent::DiskOverload(device_id, device_ip, device_name, uid, username, disk_usage, disk_path);
        if (const auto r = SpvrEventApi::AddEvent(host, port, appkey, event); r.has_value()) {
            return true;
        }
        return false;
    }

    bool GrEventManager::AddGpuEvent(int gpu_usage, const std::string& gpu_id, const std::string& gpu_name) {
        const auto host = settings_->GetSpvrServerHost();
        const auto port = settings_->GetSpvrServerPort();
        const auto appkey = grApp->GetAppkey();
        if (host.empty() || port <= 0 || appkey.empty()) {
            return false;
        }
        if (!user_mgr_) {
            user_mgr_ = grApp->GetUserManager();
            if (!user_mgr_) {
                LOGE("No user manager!");
                return false;
            }
        }

        std::string device_id = settings_->GetDeviceId();
        std::string device_ip = context_->GetFirstAvailableIp();
        std::string device_name = settings_->GetDeviceName();
        std::string uid = user_mgr_->GetUserId();
        std::string username = user_mgr_->GetUsername();
        const auto event = SpvrEvent::GpuOverload(device_id, device_ip, device_name, uid, username, gpu_usage, gpu_id, gpu_name);
        if (const auto r = SpvrEventApi::AddEvent(host, port, appkey, event); r.has_value()) {
            return true;
        }
        return false;
    }

}
