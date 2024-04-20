//
// Created by RGAA on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_LAYOUT_HELPER_H
#define TC_SERVER_STEAM_LAYOUT_HELPER_H

#include <QBoxLayout>

namespace tc
{

    class LayoutHelper {
    public:

        static void ClearMargins(QBoxLayout* layout) {
            layout->setContentsMargins(0,0,0,0);
            layout->setSpacing(0);
        }

    };

}

#endif //TC_SERVER_STEAM_LAYOUT_HELPER_H
