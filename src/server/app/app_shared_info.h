//
// Created by RGAA on 2024/3/17.
//

#ifndef TC_APPLICATION_APP_SHARED_INFO_H
#define TC_APPLICATION_APP_SHARED_INFO_H

#include <map>
#include <memory>
#include <functional>
#include <string>

#include <Poco/NamedEvent.h>
#include <Poco/SharedMemory.h>
#include <Poco/NamedMutex.h>

#include "tc_capture_new/capture_message.h"

namespace tc
{

    class Context;

    // write information to shared memory, so the dll can read the shm after it is injected.
    class AppSharedInfo {
    public:
        static std::shared_ptr<AppSharedInfo> Make(const std::shared_ptr<Context>& ctx);

        explicit AppSharedInfo(const std::shared_ptr<Context>& ctx);
        // write data to target shared memory
        void WriteData(const std::string& shm_name, const std::string& data);
        void Exit();

    private:
        void GuaranteeTargetMemory(const std::string& shm_name);

    private:
        std::shared_ptr<Context> context_ = nullptr;
        std::map<std::string, std::shared_ptr<Poco::SharedMemory>> target_memories_;
    };

}

#endif //TC_APPLICATION_APP_SHARED_INFO_H
