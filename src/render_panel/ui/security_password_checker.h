//
// Created by RGAA on 16/06/2025.
//

#ifndef GAMMARAY_SECURITY_PASSWORD_CHECKER_H
#define GAMMARAY_SECURITY_PASSWORD_CHECKER_H

#include <string>

namespace tc
{

    class SecurityPasswordChecker {
    public:
        static bool HasSecurityPassword();
        // return true: showing the dialog
        // return false: already has security password, ignore it
        static bool ShowNoSecurityPasswordDialog();
        //
        static std::string GetSecurityPasswordDialog();
        //
        static bool IsInputSecurityPasswordOk(const std::string& pwd);
        //
        static void ShowSecurityPasswordInvalidDialog();
    };

}

#endif //GAMMARAY_SECURITY_PASSWORD_CHECKER_H
