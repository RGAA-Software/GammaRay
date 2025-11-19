//
// Created by RGAA on 18/11/2025.
//

#include "gr_user_manager.h"
#include <format>
#include "tc_common_new/log.h"
#include "tc_common_new/md5.h"
#include "tc_spvr_client/spvr_user.h"
#include "tc_spvr_client/spvr_user_api.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_application.h"
#include "tc_label.h"
#include "tc_dialog.h"

const std::string kUserPrefix = "spvr_user:";

namespace tc
{

    GrUserManager::GrUserManager(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
    }

    bool GrUserManager::Register(const std::string& username, const std::string& password) {
        auto host = settings_->GetSpvrServerHost();
        auto port = settings_->GetSpvrServerPort();
        auto appkey = grApp->GetAppkey();
        auto hash_password = MD5::Hex(password);
        auto r = spvr::SpvrUserApi::Register(host, port, appkey, username, hash_password);
        if (r.has_value()) {
            auto user = r.value();
            this->SaveUserInfo(user->uid_, user->username_, password, user->avatar_path_);
        }
        else {
            auto err = r.error();
            LOGE("Register failed, err: {}, msg: {}", (int)err, spvr::SpvrApiErrorAsString(err));
            context_->PostUITask([=, this]() {
                QString msg = tcTr("id_op_error") + ":" + QString::number((int)err);
                TcDialog dialog(tcTr("id_error"), msg);
                dialog.exec();
            });
        }
        return true;
    }

    bool GrUserManager::Login(const std::string& username, const std::string& password) {
        auto host = settings_->GetSpvrServerHost();
        auto port = settings_->GetSpvrServerPort();
        auto appkey = grApp->GetAppkey();
        auto hash_password = MD5::Hex(password);
        auto r = spvr::SpvrUserApi::Register(host, port, appkey, username, hash_password);
        if (r.has_value()) {
            auto user = r.value();
            this->SaveUserInfo(user->uid_, user->username_, password, user->avatar_path_);
            return true;
        }
        else {
            auto err = r.error();
            LOGE("Register failed, err: {}, msg: {}", (int)err, spvr::SpvrApiErrorAsString(err));
            context_->PostUITask([=, this]() {
                QString msg = tcTr("id_op_error") + ":" + QString::number((int)err);
                TcDialog dialog(tcTr("id_error"), msg);
                dialog.exec();
            });
            return false;
        }
    }

    bool GrUserManager::Logout() {
        auto host = settings_->GetSpvrServerHost();
        auto port = settings_->GetSpvrServerPort();
        auto appkey = grApp->GetAppkey();
        auto uid = GetUserId();
        auto password = GetPassword();
        auto hash_password = MD5::Hex(password);
        auto r = spvr::SpvrUserApi::Logout(host, port, appkey, uid, hash_password);
        if (r.has_value()) {
            LOGI("Logout: {} {}", GetUsername(), GetUserId());
            Clear();
        }
        else {
            auto err = r.error();
            LOGE("Register failed, err: {}, msg: {}", (int)err, spvr::SpvrApiErrorAsString(err));
            context_->PostUITask([=, this]() {
                QString msg = tcTr("id_op_error") + ":" + QString::number((int)err);
                TcDialog dialog(tcTr("id_error"), msg);
                dialog.exec();
            });
        }
        return true;
    }

    void GrUserManager::SaveUserInfo(const std::string& uid, const std::string& username, const std::string& password, const std::string& avatar_path) {
        // uid
        context_->SpPutString(KeyUid(), uid);

        // username
        this->UpdateUsername(username);

        // password
        this->UpdatePassword(password);

        // avatar path
        this->UpdateAvatarPath(avatar_path);
    }

    std::string GrUserManager::GetUserId() {
        return context_->SpGetString(KeyUid());
    }

    void GrUserManager::UpdateUsername(const std::string& username) {
        context_->SpPutString(KeyUsername(), username);
    }

    std::string GrUserManager::GetUsername() {
        return context_->SpGetString(KeyUsername());
    }

    void GrUserManager::UpdatePassword(const std::string& password) {
        context_->SpPutString(KeyPassword(), password);
    }

    std::string GrUserManager::GetPassword() {
        return context_->SpGetString(KeyPassword());
    }

    void GrUserManager::UpdateAvatarPath(const std::string& avatar_path) {
        context_->SpPutString(KeyAvatarPath(), avatar_path);
    }

    std::string GrUserManager::GetAvatarPath() {
        return context_->SpGetString(KeyAvatarPath());
    }

    void GrUserManager::Clear() {
        SaveUserInfo("", "", "", "");
    }

    std::string GrUserManager::KeyUid() {
        return std::format("{}{}", kUserPrefix, spvr::kUserId);
    }

    std::string GrUserManager::KeyUsername() {
        return std::format("{}{}", kUserPrefix, spvr::kUserName);
    }

    std::string GrUserManager::KeyPassword() {
        return std::format("{}{}", kUserPrefix, spvr::kUserPassword);
    }

    std::string GrUserManager::KeyAvatarPath() {
        return std::format("{}{}", kUserPrefix, spvr::kUserAvatarPath);
    }

}