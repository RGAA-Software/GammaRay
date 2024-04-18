//
// Created by RGAA on 2024-03-30.
//

#ifndef TC_SERVER_STEAM_WS_SERVER_H
#define TC_SERVER_STEAM_WS_SERVER_H

#include <memory>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

namespace tc
{
    class GrContext;

    class WSServer : public QObject {
        Q_OBJECT
    public:

        static std::shared_ptr<WSServer> Make(const std::shared_ptr<GrContext>& ctx);
        explicit WSServer(const std::shared_ptr<GrContext>& ctx);
        ~WSServer() override;

        void Start();
        void Exit();
    Q_SIGNALS:
        void closed();

    private Q_SLOTS:
        void onNewConnection();
        void processTextMessage(QString message);
        void processBinaryMessage(QByteArray message);
        void socketDisconnected();

    private:
        QWebSocketServer* ws_server_ = nullptr;
        QList<QWebSocket*> clients_;
        std::shared_ptr<GrContext> context_ = nullptr;
    };
}

#endif //TC_SERVER_STEAM_WS_SERVER_H
