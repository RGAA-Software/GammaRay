//
// Created by RGAA on 2024-03-30.
//

#include "render_manager.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"
#include "tc_common_new/process_util.h"

#include <QCoreApplication>

static const std::string kGammaRayName = "GammaRay.exe";
static const std::string kGammaRayServerName = "GammaRayServer.exe";

namespace tc
{

    RenderManager::RenderManager(const std::shared_ptr<ServiceContext>& ctx) {
        context_ = ctx;
    }

    RenderManager::~RenderManager() {
        Exit();
    }

    Response<bool, uint32_t> RenderManager::StartServer() {
        auto resp = Response<bool, uint32_t>::Make(false, 0, "");

        QString current_path = "D:/source/GammaRay/cmake-build-relwithdebinfo";//QCoreApplication::applicationDirPath();
        QString work_dir = current_path;
        current_path = current_path.append("/").append(kGammaRayServerName);

        ProcessUtil::StartProcessAsUser(current_path.toStdWString(), work_dir.toStdWString(), false);
        return resp;
    }

    Response<bool, bool> RenderManager::StopServer() {
        auto resp = Response<bool, bool>::Make(false, false, "");
        if (running_srv_ && running_srv_->process_) {
            running_srv_->process_->kill();
            running_srv_->process_ = nullptr;
        }
        resp.ok_ = true;
        resp.value_ = true;
        return resp;
    }

    Response<bool, uint32_t> RenderManager::ReStart() {
        this->StopServer();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return this->StartServer();
    }

    void RenderManager::Exit() {
        if (running_srv_ && running_srv_->process_) {
            running_srv_->process_->kill();
            running_srv_->process_->waitForFinished();
            running_srv_->process_ = nullptr;
        }
    }

}