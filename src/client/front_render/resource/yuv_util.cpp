//
// Created by RGAA on 7/09/2025.
//

#include "yuv_util.h"

namespace tc
{

    std::shared_ptr<NV12Frame> YUVUtil::ReadNV12FromFile(const std::string &filename, uint32_t width, uint32_t height) {
        char buf[1024] = {0};
        FILE *file = nullptr;
        sprintf_s(buf, filename.c_str());

        fopen_s(&file, buf, "rb");

        int size = sizeof(NV12Frame);
        auto nv12Frame = std::make_shared<NV12Frame>();

        nv12Frame->width = width;
        nv12Frame->height = height;
        nv12Frame->pitch = nv12Frame->width;

        int y_size = nv12Frame->pitch * nv12Frame->height;

        nv12Frame->Y = (uint8_t*) malloc(y_size);
        auto readBytes = fread(nv12Frame->Y, size, 1, file);

        int uv_size = nv12Frame->pitch * nv12Frame->height / 2;
        nv12Frame->UV = (uint8_t*) malloc(uv_size);
        readBytes = fread(nv12Frame->UV, size, 1, file);

        fclose(file);
        return nv12Frame;
    }

#ifdef WIN32
    void YUVUtil::WriteNV12ToTexture(const std::shared_ptr<NV12Frame>& nv12Frame, ID3D11DeviceContext* device_context, ID3D11Texture2D* texture) {
        // Copy from CPU access texture to bitmap buffer
        D3D11_MAPPED_SUBRESOURCE resource;
        UINT subresource = D3D11CalcSubresource(0, 0, 0);
        device_context->Map(texture, subresource, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        uint8_t *dptr = reinterpret_cast<uint8_t *>(resource.pData);

        for (int i = 0; i < nv12Frame->height; i++) {
            memcpy(dptr + resource.RowPitch * i, nv12Frame->Y + nv12Frame->pitch * i, nv12Frame->pitch);
        }

        for (int i = 0; i < nv12Frame->height / 2; i++) {
            memcpy(dptr + resource.RowPitch * (nv12Frame->height + i), nv12Frame->UV + nv12Frame->pitch * i, nv12Frame->pitch);
        }

        device_context->Unmap(texture, subresource);
    }
#endif

}