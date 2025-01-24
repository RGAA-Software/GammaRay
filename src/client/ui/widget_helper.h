//
// Created by RGAA on 2024/3/8.
//

#ifndef TC_CLIENT_PC_WIDGET_HELPER_H
#define TC_CLIENT_PC_WIDGET_HELPER_H

#include <QLayout>
#include <QGraphicsDropShadowEffect>

namespace tc
{

    class WidgetHelper {
    public:

        static void ClearMargin(QLayout* layout) {
            layout->setSpacing(0);
            layout->setContentsMargins(0,0,0,0);
        }

        static void AddShadow(QWidget* w, int color = 0xBBBBBB, int radius = 15) {
            auto ps = new QGraphicsDropShadowEffect();
            ps->setBlurRadius(radius);
            ps->setOffset(0, 0);
            ps->setColor(color);
            w->setGraphicsEffect(ps);
        }

    };

}

#endif //TC_CLIENT_PC_WIDGET_HELPER_H
