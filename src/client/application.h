//
// Created by RGAA on 2024/3/8.
//

#ifndef TC_CLIENT_PC_APPLICATION_H
#define TC_CLIENT_PC_APPLICATION_H

#include <QtWidgets/QMainWindow>
#include <QStackedWidget>
#include "theme/QtAdvancedStylesheet.h"

#include "db/stream_item.h"

namespace tc
{

    struct MainWindowPrivate {
        explicit MainWindowPrivate(QMainWindow* _public) : _this(_public) {}

        QMainWindow* _this;
        acss::QtAdvancedStylesheet* AdvancedStyleSheet{};
    };

    class AppMenu;
    class ClientContext;
    class StreamContent;
    class MessageListener;
    class Settings;

    class Application : public QMainWindow {
    public:
        explicit Application(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        ~Application() override;

    private:
        void CreateLayout();
        void Init();
        void StartStreaming(const StreamItem& item);
        void changeEvent(QEvent *) override;
        void closeEvent(QCloseEvent *event) override;

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        AppMenu* app_menu_ = nullptr;
        QStackedWidget* content_widget_ = nullptr;
        StreamContent* stream_content_ = nullptr;
        MainWindowPrivate* theme_ = nullptr;
        Settings* settings_ = nullptr;
    };

}

#endif //TC_CLIENT_PC_APPLICATION_H
