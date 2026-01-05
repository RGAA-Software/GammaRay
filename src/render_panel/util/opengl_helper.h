#pragma once

#include <QApplication>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QDebug>
#include <QOperatingSystemVersion>

namespace tc
{

    enum class EOpenGLBackend {
        kDesktop,
        kGLES,
        kSoftware,
        kUnknown
    };

    EOpenGLBackend DetectOpenGLBackend();
}