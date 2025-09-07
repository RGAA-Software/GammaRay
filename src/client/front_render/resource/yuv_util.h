//
// Created by RGAA on 7/09/2025.
//

#ifndef GAMMARAYPREMIUM_RESOURCE_YUV_UTIL_H
#define GAMMARAYPREMIUM_RESOURCE_YUV_UTIL_H

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <memory>

#ifdef WIN32
#include <dxgi.h>
#include <d3d11.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;
#endif

namespace tc
{

    class NV12Frame {
    public:
        ~NV12Frame() {
            if (Y) {
                free(Y);
            }
            if (UV) {
                free(UV);
            }
        }

    public:
        uint32_t width;
        uint32_t height;
        uint32_t pitch;
        uint8_t *Y = nullptr;
        uint8_t *UV = nullptr;
    };

    class YUVUtil {
    public:
        // read the NV12 file
        std::shared_ptr<NV12Frame> ReadNV12FromFile(const std::string& filename, uint32_t width, uint32_t height);

#ifdef WIN32
        //
        void WriteNV12ToTexture(const std::shared_ptr<NV12Frame>& nv12Frame, ID3D11DeviceContext* device_context, ID3D11Texture2D* texture);
#endif

    };

}

#endif //GAMMARAYPREMIUM_YUV_UTIL_H
