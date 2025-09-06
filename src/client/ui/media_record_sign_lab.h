#pragma once
#include <qevent.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qcolor.h>

class QTimer;


namespace tc
{
    class MediaRecordSignLab : public QWidget {
    public:
        explicit MediaRecordSignLab(QWidget* parent = nullptr);
        ~MediaRecordSignLab();
        void paintEvent(QPaintEvent* event) override;
    private:
        QTimer* timer_ = nullptr;
        QColor brush_color_;

        int color_value_ = 0;
        int toggle_ = 0;
    };
}