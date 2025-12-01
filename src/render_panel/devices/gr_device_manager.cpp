//
// Created by RGAA on 27/11/2025.
//

#include "gr_device_manager.h"
#include <format>
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "tc_spvr_client/spvr_device.h"
#include "tc_spvr_client/spvr_device_api.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_application.h"
#include "tc_label.h"
#include "tc_dialog.h"

namespace tc
{

    GrDeviceManager::GrDeviceManager(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
    }

    Result<std::shared_ptr<spvr::SpvrDevice>, spvr::SpvrApiError> GrDeviceManager::RequestNewDevice(const std::string& def_device_name, const std::string& info) {
        auto host = settings_->GetSpvrServerHost();
        auto port = settings_->GetSpvrServerPort();
        auto appkey = grApp->GetAppkey();
        auto r = spvr::SpvrDeviceApi::RequestNewDevice(host, port, appkey, def_device_name, info);
        return r;
    }

    bool GrDeviceManager::UpdateDesktopLink(const std::string& desktop_link, const std::string& desktop_link_raw) {
        auto host = settings_->GetSpvrServerHost();
        auto port = settings_->GetSpvrServerPort();
        auto appkey = grApp->GetAppkey();
        auto device_id = settings_->GetDeviceId();
        auto r = spvr::SpvrDeviceApi::UpdateDesktopLink(host, port, appkey, device_id, desktop_link, desktop_link_raw);
        if (r.has_value()) {
            LOGI("UpdateDesktopLink success for device: {} ", device_id);
            return true;
        }
        else {
            auto err = r.error();
            LOGE("UpdateDesktop link failed, err: {}, msg: {}", (int)err, spvr::SpvrApiErrorAsString(err));
            return false;
        }
    }

    Result<std::shared_ptr<spvr::SpvrDevice>, spvr::SpvrApiError> GrDeviceManager::UpdateDeviceName(const std::string& device_name) {
        auto host = settings_->GetSpvrServerHost();
        auto port = settings_->GetSpvrServerPort();
        auto appkey = grApp->GetAppkey();
        auto device_id = settings_->GetDeviceId();
        auto r = spvr::SpvrDeviceApi::UpdateDeviceName(host, port, appkey, device_id, device_name);
        return r;
    }

}