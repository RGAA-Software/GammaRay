//
// Created by RGAA on 2024-04-11.
//

#ifndef QT_VERTICLE_H
#define QT_VERTICLE_H

#include "effect_widget.h"
#include <QPixmap>

namespace tc
{

    class QtVertical : public EffectWidget {
    public:
        QtVertical(QWidget *parent);
        void paintEvent(QPaintEvent *event) override;

    private:
        QPixmap pixmap;
    };


}

#endif // QT_VERTICLE_H
