//
// Created by RGAA on 2024-04-11.
//

#ifndef QT_VIS_H_
#define QT_VIS_H_

#include <QWidget>
#include <QObject>
#include "effect_widget.h"

namespace tc
{

    class QtCircle : public EffectWidget {
    //Q_OBJECT

    public:
        explicit QtCircle(QWidget *parent = nullptr);
        void paintEvent(QPaintEvent *event) override;

    private:

        float step = 3.5f;
        float rotate = 0;
        long color_count = 0;
        long long start = 0;
        float delta = 0;

        int bar_size = kMaxBars / 2;
        QPixmap pixmap;

    signals:

    };

}

#endif // VIS_H
