//
// Created by RGAA on 2024-04-10.
//

#ifndef TC_SERVER_STEAM_NO_MARGIN_LAYOUT_H
#define TC_SERVER_STEAM_NO_MARGIN_LAYOUT_H

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace tc
{

    class NoMarginVLayout : public QVBoxLayout {
    public:
        NoMarginVLayout() : QVBoxLayout() {
            setSpacing(0);
            setContentsMargins(0,0,0,0);
        }
    };

    class NoMarginHLayout : public QHBoxLayout {
    public:
        NoMarginHLayout() : QHBoxLayout() {
            setSpacing(0);
            setContentsMargins(0,0,0,0);
        }
    };

}
#endif //TC_SERVER_STEAM_NO_MARGIN_LAYOUT_H
