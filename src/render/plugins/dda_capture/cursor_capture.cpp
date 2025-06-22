//
// Created by RGAA on 2023/8/20.
//
#include "cursor_capture.h"
#include <Windows.h>
#include <iostream>
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/data.h"
#include "tc_common_new/time_util.h"
#include "tc_capture_new/capture_message.h"
#include "tc_common_new/log.h"
#include "tc_message.pb.h"
#include "plugin_interface/gr_plugin_events.h"
#include "dda_capture_plugin.h"

namespace tc
{

    static uint8_t *get_bitmap_data(HBITMAP hbmp, BITMAP *bmp, uint32_t *sizeOut) {
        if (GetObject(hbmp, sizeof(*bmp), bmp) != 0) {
            uint8_t *output;
            unsigned int size = (bmp->bmHeight * bmp->bmWidth * bmp->bmBitsPixel) / 8;
            if (sizeOut) {
                *sizeOut = size;
            }
            output = (uint8_t *) malloc(size);
            GetBitmapBits(hbmp, size, output);
            return output;
        }
        return nullptr;
    }

    static inline uint8_t bit_to_alpha(uint8_t *data, long pixel, bool invert) {
        uint8_t pix_byte = data[pixel / 8];
        bool alpha = (pix_byte >> (7 - pixel % 8) & 1) != 0;
        if (invert) {
            return alpha ? 0xFF : 0;
        } else {
            return alpha ? 0 : 0xFF;
        }
    }

    static inline bool bitmap_has_alpha(uint8_t *data, long num_pixels) {
        for (long i = 0; i < num_pixels; i++) {
            if (data[i * 4 + 3] != 0) {
                return true;
            }
        }
        return false;
    }

    static inline void apply_mask(uint8_t *color, uint8_t *mask, long num_pixels) {
        for (long i = 0; i < num_pixels; i++)
            color[i * 4 + 3] = bit_to_alpha(mask, i, false);
    }

    static inline void apply_mask(uint8_t *color, uint8_t *mask, BITMAP *bmp_mask) {
        long mask_pix_offs;
        for (long y = 0; y < bmp_mask->bmHeight; y++) {
            for (long x = 0; x < bmp_mask->bmWidth; x++) {
                mask_pix_offs = y * (bmp_mask->bmWidthBytes * 8) + x;
                color[(y * bmp_mask->bmWidth + x) * 4 + 3] = bit_to_alpha(mask, mask_pix_offs, false);
            }
        }
    }

    static inline uint8_t *copy_from_color(ICONINFO *ii, uint32_t *width, uint32_t *height, uint32_t *sizeOut) {
        BITMAP bmp_color;
        BITMAP bmp_mask;
        uint8_t *color;
        uint8_t *mask;

        color = get_bitmap_data(ii->hbmColor, &bmp_color, sizeOut);
        if (!color) {
            return nullptr;
        }

        if (bmp_color.bmBitsPixel < 32) {
            free(color);
            return nullptr;
        }

        mask = get_bitmap_data(ii->hbmMask, &bmp_mask, nullptr);
        if (mask) {
            long pixels = bmp_color.bmHeight * bmp_color.bmWidth;

            if (!bitmap_has_alpha(color, pixels)) {
                //apply_mask(color, mask, pixels);
                apply_mask(color, mask, &bmp_mask);
            }
            free(mask);
        }

        *width = bmp_color.bmWidth;
        *height = bmp_color.bmHeight;
        return color;
    }

    static inline uint8_t *copy_from_mask(ICONINFO *ii, uint32_t *width, uint32_t *height, uint32_t *sizeOut) {
        uint8_t *output;
        uint8_t *mask;
        long pixels;
        long bottom;
        BITMAP bmp;

        mask = get_bitmap_data(ii->hbmMask, &bmp, sizeOut);
        if (!mask) {
            return nullptr;
        }

        bmp.bmHeight /= 2;

        pixels = bmp.bmHeight * bmp.bmWidth;
        int outputSize = pixels * 4;
        if (sizeOut)
            *sizeOut = outputSize;
        output = (uint8_t *) calloc(1, outputSize);

        bottom = bmp.bmWidthBytes * bmp.bmHeight;

        for (long i = 0; i < pixels; i++) {
            uint8_t alpha = bit_to_alpha(mask, i, false);
            uint8_t color = bit_to_alpha(mask + bottom, i, true);
            if (!alpha) {
                output[i * 4 + 3] = color;
            } else {
                *(uint32_t *) &output[i * 4] = !!color ? 0xFFFFFFFF : 0xFF000000;
            }
        }

        free(mask);

        *width = bmp.bmWidth;
        *height = bmp.bmHeight;
        return output;
    }

    static inline uint8_t *cursor_capture_icon_bitmap(ICONINFO *ii, uint32_t *width, uint32_t *height, uint32_t *sizeOut) {
        uint8_t *output;

        output = copy_from_color(ii, width, height, sizeOut);
        if (!output) {
            output = copy_from_mask(ii, width, height, sizeOut);
        }
        return output;
    }

    static void reorder_rgba(CaptureCursorBitmap *cursor) {
        int offset = 0;
        for (int row = 0; row < cursor->height_; ++row) {
            for (int col = 0; col < cursor->width_; ++col) {
                char r = cursor->data_->At(offset);
                *((char *) cursor->data_->DataAddr() + offset) = *(cursor->data_->DataAddr() + offset + 2);
                *((char *) cursor->data_->DataAddr() + offset + 2) = r;
                offset += 4;
            }
        }
    }

    CursorCapture::CursorCapture(DDACapturePlugin* plugin) {
        plugin_ = plugin;
        last_cursor_bitmap_data_ = Data::Make(nullptr, 1);
    }

    bool CursorCapture::CaptureCursorIcon(CaptureCursorBitmap *data, HICON icon) {
        uint8_t *bitmap;
        uint32_t height;
        uint32_t width;
        ICONINFO ii;

        if (!icon) {
            return false;
        }
        if (!GetIconInfo(icon, &ii)) {
            return false;
        }
        uint32_t bitmapSize = 0;
        bitmap = cursor_capture_icon_bitmap(&ii, &width, &height, &bitmapSize);
        if (bitmap) {
            data->data_ = Data::From(std::string(bitmap, bitmap + bitmapSize));
            data->width_ = width;
            data->height_ = height;
            data->hotspot_x_ = ii.xHotspot;
            data->hotspot_y_ = ii.yHotspot;
            free(bitmap);
        } else {
            return false;
        }
        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
        return true;
    }

    void CursorCapture::Capture() {
        CaptureCursorBitmap cursor_bitmap;
        CURSORINFO ci = {0};
        HICON icon;
        ci.cbSize = sizeof(ci);

        if (!GetCursorInfo(&ci)) {
            cursor_bitmap.visible_ = true;
            return;
        }
        cursor_bitmap.visible_ = (ci.flags & CURSOR_SHOWING) == CURSOR_SHOWING;
        cursor_bitmap.x_ = ci.ptScreenPos.x;
        cursor_bitmap.y_ = ci.ptScreenPos.y;

        static HCURSOR cursor_arrow = LoadCursorW(nullptr, IDC_ARROW);
        static HCURSOR cursor_ibeam = LoadCursorW(nullptr, IDC_IBEAM);
        static HCURSOR cursor_wait = LoadCursorW(nullptr, IDC_WAIT);
        static HCURSOR cursor_cross = LoadCursorW(nullptr, IDC_CROSS);
        static HCURSOR cursor_uparrow = LoadCursorW(nullptr, IDC_UPARROW);
        static HCURSOR cursor_size = LoadCursorW(nullptr, IDC_SIZE);
        static HCURSOR cursor_icon = LoadCursorW(nullptr, IDC_ICON);
        static HCURSOR cursor_sizenwse = LoadCursorW(nullptr, IDC_SIZENWSE);
        static HCURSOR cursor_sizenesw = LoadCursorW(nullptr, IDC_SIZENESW);
        static HCURSOR cursor_sizewe = LoadCursorW(nullptr, IDC_SIZEWE);
        static HCURSOR cursor_sizens = LoadCursorW(nullptr, IDC_SIZENS);
        static HCURSOR cursor_sizeall = LoadCursorW(nullptr, IDC_SIZEALL);
        static HCURSOR cursor_hand = LoadCursorW(nullptr, IDC_HAND);
        static HCURSOR cursor_help = LoadCursorW(nullptr, IDC_HELP);
        static HCURSOR cursor_pin = LoadCursorW(nullptr, IDC_PIN);
        static HCURSOR cursor_person = LoadCursorW(nullptr, IDC_PERSON);

        if (ci.hCursor == cursor_arrow) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcArrow;
        } else if (ci.hCursor == cursor_ibeam) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcIBeam;
        } else if (ci.hCursor == cursor_wait) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcWait;
        } else if (ci.hCursor == cursor_cross) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcCross;
        } else if (ci.hCursor == cursor_uparrow) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcUpArrow;
        } else if (ci.hCursor == cursor_size) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcSize;
        } else if (ci.hCursor == cursor_icon) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcIcon;
        } else if (ci.hCursor == cursor_sizenwse) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcSizeNWSE;
        } else if (ci.hCursor == cursor_sizenesw) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcSizeNESW;
        } else if (ci.hCursor == cursor_sizewe) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcSizeWE;
        } else if (ci.hCursor == cursor_sizens) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcSizeNS;
        } else if (ci.hCursor == cursor_sizeall) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcSizeAll;
        } else if (ci.hCursor == cursor_hand) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcHand;
        } else if (ci.hCursor == cursor_help) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcHelp;
        } else if (ci.hCursor == cursor_pin) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcPin;
        } else if (ci.hCursor == cursor_person) {
            cursor_bitmap.type_ = CursorInfoSync::kIdcPerson;
        }

        // RGB Data
        icon = CopyIcon(ci.hCursor);
        if (CaptureCursorIcon(&cursor_bitmap, icon)) {
            reorder_rgba(&cursor_bitmap);
        } else {
            return;
        }
        DestroyIcon(icon);
        std::string current_data;
        std::string last_data = last_cursor_bitmap_data_->AsString();
        if (cursor_bitmap.data_) {
            current_data = cursor_bitmap.data_->AsString();
        }

        auto event = std::make_shared<GrPluginCursorEvent>();
        event->cursor_info_ = cursor_bitmap;

        if (current_data != last_data && !current_data.empty()) {
            last_cursor_bitmap_data_ = cursor_bitmap.data_;
        }
        else {
            //如果采集到的鼠标图标和上次采集的鼠标图标相同,在一定时间内，就不发送鼠标图标
            auto current_time = TimeUtil::GetCurrentTimestamp();
            if (last_timestamp_ + 2000 > current_time) {
                event->cursor_info_.data_ = nullptr;
                //LOGI("event->cursor_info_.data_ = nullptr;");
            }
        }

        if (event->cursor_info_.data_) {
            last_timestamp_ = TimeUtil::GetCurrentTimestamp();
        }

        this->plugin_->CallbackEvent(event);
    }
}
