//
// Created by RGAA on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_BASE_TAB_H
#define TC_SERVER_STEAM_BASE_TAB_H

#include <QWidget>
#include <memory>

namespace tc
{
    constexpr int kTabContentMarginTop = 3;

    class GrContext;
    class GrSettings;
    class GrApplication;
    class MessageListener;
    class GrStatistics;

    class TabBase : public QWidget {
    public:
        explicit TabBase(const std::shared_ptr<GrApplication>& app, QWidget* parent);
        ~TabBase() override;
        virtual void OnTabShow();
        virtual void OnTabHide();
        virtual void OnTranslate();

        void SetAttach(QObject* at) {attach_ = at;}
        QObject* GetAttach() {return attach_;}

    protected:
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        QObject* attach_ = nullptr;
        GrSettings* settings_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        GrStatistics* statistics_ = nullptr;
    };
}

#endif //TC_SERVER_STEAM_BASE_TAB_H
