#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <memory>

namespace tc
{
    class Data;
    class DDACapturePlugin;
    class CaptureCursorBitmap;

    class CursorCapture {
    public:
        explicit CursorCapture(DDACapturePlugin* plugin);
        void Capture();

    private:
        static bool CaptureCursorIcon(CaptureCursorBitmap* data, HICON icon);

    private:
        DDACapturePlugin* plugin_ = nullptr;

        std::shared_ptr<Data> last_cursor_bitmap_data_ = nullptr;
        uint64_t last_timestamp_ = 0;
    };

}
