//
// Created by RGAA on 11/06/2025.
//

#ifndef GAMMARAY_RENDER_API_H
#define GAMMARAY_RENDER_API_H

#include <string>
#include "tc_3rdparty/expt/expected.h"

namespace tc
{

    // Render configuration
    class RenderConfiguration {
    public:
        std::string device_id_;
        std::string relay_host_;
        std::string relay_port_{0};
    };

    // api to Renderer
    class RenderApi {
    public:
        // can connect to Renderer?
        static Result<RenderConfiguration, int> GetRenderConfiguration(const std::string& host, int port);

        // verify security password in Renderer
        static Result<bool, int> VerifySecurityPassword(const std::string& host, int port, const std::string& safety_pwd_md5);
    };

}

#endif //GAMMARAY_RENDER_API_H
