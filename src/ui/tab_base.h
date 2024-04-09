//
// Created by hy on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_BASE_TAB_H
#define TC_SERVER_STEAM_BASE_TAB_H

#include <QWidget>

namespace tc
{
    class TabBase : public QWidget {
    public:
        explicit TabBase(QWidget* parent);
        ~TabBase() override;
        virtual void OnTabShow();
        virtual void OnTabHide();
    };
}

#endif //TC_SERVER_STEAM_BASE_TAB_H
