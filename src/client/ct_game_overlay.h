#pragma once
#include <qwidget.h>
#include <qpainter.h>
#include <qevent.h>
#include <qstring.h>

namespace tc { 
    class OverlayWidget : public QWidget {
        Q_OBJECT
    public:
        explicit OverlayWidget(QWidget* parent = nullptr);
        void SetWatermarkText(const QString& text);
        void SetWatermarkCount(int count);
        void SetOpacity(double op);
    protected:
        void paintEvent(QPaintEvent*) override;

    private:
        QString watermark_text_ = "UnLicensed Stream";
        int watermark_count_ = 1;
        double opacity_ = 0.6;    // 0.0 ~ 1.0
        bool enable_watermark_ = true;
    };
}