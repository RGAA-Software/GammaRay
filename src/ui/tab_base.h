//
// Created by RGAA on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_BASE_TAB_H
#define TC_SERVER_STEAM_BASE_TAB_H

#include <QWidget>
#include <memory>

namespace tc
{
    class GrContext;
    class GrSettings;

    class TabBase : public QWidget {
    public:
        explicit TabBase(const std::shared_ptr<GrContext>& ctx, QWidget* parent);
        ~TabBase() override;
        virtual void OnTabShow();
        virtual void OnTabHide();

        void SetAttach(QObject* at) {attach_ = at;}
        QObject* GetAttach() {return attach_;}

    protected:
        std::shared_ptr<GrContext> context_ = nullptr;
        QObject* attach_ = nullptr;
        GrSettings* settings_ = nullptr;
    };
}

#endif //TC_SERVER_STEAM_BASE_TAB_H
