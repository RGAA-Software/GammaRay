#include "opengl_helper.h"
#include <qdebug.h>
#include <qregularexpression.h>
#include <qoffscreensurface.h>
#include <qopenglcontext.h>
#include <qopenglfunctions.h>
#include "tc_common_new/log.h"

namespace tc
{

    EOpenGLBackend DetectOpenGLBackend()
    {
        try {
            // 用低版本探测
            QSurfaceFormat fmt;
            fmt.setVersion(2, 0);
            fmt.setProfile(QSurfaceFormat::NoProfile);

            QOpenGLContext ctx;
            ctx.setFormat(fmt);
            if (!ctx.create() || !ctx.isValid()) {
                LOGW("[OpenGL] Failed to create context.");
                return EOpenGLBackend::kUnknown;
            }

            QOffscreenSurface surface;
            surface.setFormat(fmt);
            surface.create();
            if (!ctx.makeCurrent(&surface)) {
                LOGW("[OpenGL] Failed to makeCurrent.");
                return EOpenGLBackend::kUnknown;
            }

            const char* renderer = (const char*)ctx.functions()->glGetString(GL_RENDERER);
            QString rendererStr = renderer ? QString::fromLatin1(renderer) : "Unknown";

            LOGI("renderer info: {}", rendererStr.toStdString());

            ctx.doneCurrent();
            surface.destroy();

            // [OpenGL] Renderer: ANGLE (Microsoft Basic Render Driver Direct3D11 vs_5_0 ps_5_0)    #无核显gpu驱动
            // [OpenGL] Renderer: Intel(R) UHD Graphics                                             #有核显gpu驱动

            if (rendererStr.contains("Microsoft Basic Render Driver", Qt::CaseInsensitive) ||
                rendererStr.contains("llvmpipe", Qt::CaseInsensitive) || rendererStr.contains("SVGA3D", Qt::CaseInsensitive) ||
                rendererStr.contains("LLVM", Qt::CaseInsensitive)  ) {
                LOGI("[OpenGL] Software renderer detected.");
                return EOpenGLBackend::kSoftware;
            }
            else if (rendererStr.contains("ANGLE", Qt::CaseInsensitive)) {
                LOGI("[OpenGL] ANGLE renderer detected.");
                return EOpenGLBackend::kGLES;
            }
            else if (rendererStr.contains("Intel", Qt::CaseInsensitive)) {
                LOGI("[OpenGL] Intel renderer detected.");
                return EOpenGLBackend::kGLES; // 核显直接用 ANGLE，比 Software 更稳定
            }
            else {
                LOGI("[OpenGL] Desktop renderer detected.");
                return EOpenGLBackend::kDesktop; // NVIDIA / AMD / 高端 Intel 独显
            }
        }
        catch (std::exception& e) {
            LOGE("DetectOpenGLBackend error: {}", e.what());
            return EOpenGLBackend::kUnknown;
        }
    }
}