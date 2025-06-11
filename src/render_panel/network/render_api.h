//
// Created by RGAA on 11/06/2025.
//

#ifndef GAMMARAY_RENDER_API_H
#define GAMMARAY_RENDER_API_H

#include <string>
#include "tc_3rdparty/expt/expected.h"

namespace tc
{

    // api to Renderer
    class RenderApi {
    public:
        static Result<bool, int> VerifySafetyPassword(const std::string& host, int port, const std::string& safety_pwd_md5);
    };

}

#endif //GAMMARAY_RENDER_API_H
