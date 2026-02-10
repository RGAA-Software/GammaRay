#include "ct_game_overlay.h"
#include <random>
#include <windows.h>

namespace tc {

    OverlayWidget::OverlayWidget(QWidget* parent) : QWidget(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::Window);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setFocusPolicy(Qt::NoFocus);

        HWND hwnd = reinterpret_cast<HWND>(winId());
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
    }

    void OverlayWidget::SetWatermarkText(const QString& text) {
        watermark_text_ = text;
        update();
    }

    void OverlayWidget::SetWatermarkCount(int count) {
        watermark_count_ = count;
        if (watermark_count_ > 20) {
            watermark_count_ = 20;
        }
        update();
    }

    void OverlayWidget::SetOpacity(double op) {
        opacity_ = std::clamp(op, 0.0, 1.0);
        update();
    }

#if 1
    void OverlayWidget::paintEvent(QPaintEvent*) {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);

        // -------------------------
        // 1. 根据窗口大小自动调整字体
        // -------------------------
        const int baseFontPx = 24;     // 参考字体大小
        const int refHeight = 600;     // 参考高度，可改

        double scale = double(height()) / refHeight;
        scale = std::clamp(scale, 0.5, 1.5);  // 防止太小或太大

        int fontPx = int(baseFontPx * scale);

        QFont font("Microsoft YaHei");
        font.setPixelSize(fontPx);
        p.setFont(font);

        // 文本颜色 + 透明度
        QColor color(0, 0, 0);
        color.setAlphaF(opacity_);
        p.setPen(color);

        // -------------------------
        // 2. 整体旋转
        // -------------------------
        p.translate(width() / 2, height() / 2);
        p.rotate(-45);
        p.translate(-width() / 2, -height() / 2);

        if (watermark_count_ <= 0)
            return;

        // -------------------------
        // 3. 均匀散布 + 随机 jitter
        // -------------------------
        int gridCols = std::ceil(std::sqrt(watermark_count_));
        int gridRows = std::ceil(double(watermark_count_) / gridCols);

        int cellW = width() / gridCols;
        int cellH = height()  / gridRows;

        std::mt19937 rng(reinterpret_cast<quintptr>(this));
        auto randOffset = [&](int range) {
            std::uniform_int_distribution<int> dist(-range / 2, range / 2);
            return dist(rng);
            };

        int idx = 0;
        for (int r = 0; r < gridRows && idx < watermark_count_; r++) {
            for (int c = 0; c < gridCols && idx < watermark_count_; c++) {

                int baseX = c * cellW + cellW / 1.5;
                int baseY = r * cellH + cellH / 2.5;

                int x = baseX + randOffset(cellW / 3);
                int y = baseY + randOffset(cellH / 3);

                QRect textRect(x - 200, y - fontPx * 4, 800, fontPx * 4);
                p.drawText(textRect, Qt::AlignCenter, watermark_text_);

                idx++;
            }
        }
    }
#endif
}