//
// Created by RGAA on 2024/3/17.
//

#include "app_shared_info.h"
#include "tc_common_new/log.h"

constexpr auto kAppSharedInfoBuffSize = 1024 * 4;

namespace tc
{

    std::shared_ptr<AppSharedInfo> AppSharedInfo::Make(const std::shared_ptr<Context>& ctx) {
        return std::make_shared<AppSharedInfo>(ctx);
    }

    AppSharedInfo::AppSharedInfo(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
    }

    void AppSharedInfo::WriteData(const std::string& shm_name, const std::string& data) {
        GuaranteeTargetMemory(shm_name);
        auto shm = target_memories_[shm_name];
        if (data.size() > kAppSharedInfoBuffSize) {
            LOGE("Write to : {} failed, over size.", shm_name);
            return;
        }
        memcpy(shm->begin(), data.data(), data.size());
        LOGI("write to shm : {}, data size: {}", shm_name, data.size());
    }

    void AppSharedInfo::GuaranteeTargetMemory(const std::string& shm_name) {
        if (target_memories_.find(shm_name) != target_memories_.end()) {
            return;
        }

        auto shm = std::make_shared<Poco::SharedMemory>(shm_name, kAppSharedInfoBuffSize, Poco::SharedMemory::AccessMode::AM_WRITE);
        target_memories_[shm_name] = shm;
    }

    void AppSharedInfo::Exit() {
        target_memories_.clear();
    }

}