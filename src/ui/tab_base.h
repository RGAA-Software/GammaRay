//
// Created by hy on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_BASE_TAB_H
#define TC_SERVER_STEAM_BASE_TAB_H

#include <QWidget>
#include <memory>

namespace tc
{
    class Context;

    class TabBase : public QWidget {
    public:
        explicit TabBase(const std::shared_ptr<Context>& ctx, QWidget* parent);
        ~TabBase() override;
        virtual void OnTabShow();
        virtual void OnTabHide();

        void SetAttach(QObject* at) {attach_ = at;}
        QObject* GetAttach() {return attach_;}

    protected:
        std::shared_ptr<Context> context_ = nullptr;

    private:
        QObject* attach_ = nullptr;
    };
}

#endif //TC_SERVER_STEAM_BASE_TAB_H
