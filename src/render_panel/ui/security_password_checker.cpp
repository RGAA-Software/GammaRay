//
// Created by RGAA on 16/06/2025.
//

#include "security_password_checker.h"
#include "tc_dialog.h"
#include "tc_label.h"
#include "tc_common_new/md5.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_workspace.h"
#include "render_panel/gr_application.h"
#include "render_panel/devices/input_remote_pwd_dialog.h"

namespace tc
{

    bool SecurityPasswordChecker::HasSecurityPassword() {
        auto settings = GrSettings::Instance();
        return !settings->device_safety_pwd_md5_.empty();
    }

    bool SecurityPasswordChecker::ShowNoSecurityPasswordDialog() {
        if (HasSecurityPassword()) {
            return false;
        }

        TcDialog dialog(tcTr("id_tips"), tcTr("id_must_input_security_password"));
        dialog.exec();
        return true;
    }

    std::string SecurityPasswordChecker::GetSecurityPasswordDialog() {
        InputRemotePwdDialog dialog(grApp->GetContext());
        dialog.exec();
        return dialog.GetInputPassword().toStdString();
    }

    bool SecurityPasswordChecker::IsInputSecurityPasswordOk(const std::string& pwd) {
        auto settings = GrSettings::Instance();
        if (settings->device_safety_pwd_md5_.empty()) {
            return true;
        }
        auto pwd_md5 = MD5::Hex(pwd);
        return pwd_md5 == settings->device_safety_pwd_md5_;
    }

    void SecurityPasswordChecker::ShowSecurityPasswordInvalidDialog() {
        TcDialog dialog(tcTr("id_password_invalid"), tcTr("id_password_invalid_msg"), grWorkspace.get());
        dialog.exec();
    }
}