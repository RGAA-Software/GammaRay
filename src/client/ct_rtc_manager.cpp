//
// Created by RGAA on 13/04/2025.
//

#include "ct_rtc_manager.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "webrtc/rtc_client_interface.h"

typedef void *(*FnGetInstance)();

namespace tc
{

    CtRtcManager::CtRtcManager(const std::shared_ptr<ClientContext>& ctx) {
        context_ = ctx;
        thread_ = Thread::Make("rtc_client_thread", 1024);
        thread_->Poll();
    }

    void CtRtcManager::Init() {
        thread_->Post([=, this]() {
            auto lib_name = QApplication::applicationDirPath() + "/gr_client/tc_rtc_client.dll";
            rtc_lib_ = new QLibrary(lib_name);
            auto r = rtc_lib_->load();
            if (!r) {
                LOGE("LOAD rtc conn FAILED");
                return;
            }

            auto fn_get_instance = (FnGetInstance)rtc_lib_->resolve("GetInstance");
            if (!fn_get_instance) {
                LOGE("DON'T have GetInstance");
                return;
            }

            rtc_client_ = (RtcClientInterface*)fn_get_instance();
            if (!rtc_client_->Init()) {
                LOGE("RTC client init FAILED!");
                return;
            }

            LOGI("RTC client init success");
        });
    }

    void CtRtcManager::OnSignalingMessage(const std::shared_ptr<Message>& msg) {
        thread_->Post([=, this]() {

        });
    }

}