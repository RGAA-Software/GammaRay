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

    public:
        explicit QtCircle(QWidget *parent = nullptr);
        void paintEvent(QPaintEvent *event) override;

    private:
        float rotate_ = 0;
        long color_count_ = 0;
    };

}

#endif // VIS_H
