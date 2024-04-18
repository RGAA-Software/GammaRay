//
// Created by RGAA on 2024-03-30.
//

#include "ws_server.h"
#include "gr_settings.h"

namespace tc
{

    std::shared_ptr<WSServer> WSServer::Make(const std::shared_ptr<GrContext> &ctx) {
        return std::make_shared<WSServer>(ctx);
    }

    WSServer::WSServer(const std::shared_ptr<GrContext> &ctx) : QObject(nullptr) {
        context_ = ctx;
    }

    void WSServer::Start() {
        ws_server_ = new QWebSocketServer(QStringLiteral("Echo Server"), QWebSocketServer::NonSecureMode, this);
        if (ws_server_->listen(QHostAddress::Any, GrSettings::Instance()->ws_server_port_)) {
            qDebug() << "WebSocket server listening on port" << GrSettings::Instance()->ws_server_port_;
            connect(ws_server_, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
            connect(ws_server_, &QWebSocketServer::closed, this, &WSServer::closed);
        }
    }

    void WSServer::Exit() {

    }

    WSServer::~WSServer() {
        ws_server_->close();
        qDeleteAll(clients_.begin(), clients_.end());
    }

    void WSServer::onNewConnection() {
        QWebSocket *pSocket = ws_server_->nextPendingConnection();

        connect(pSocket, &QWebSocket::textMessageReceived, this, &WSServer::processTextMessage);
        connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WSServer::processBinaryMessage);
        connect(pSocket, &QWebSocket::disconnected, this, &WSServer::socketDisconnected);

        clients_ << pSocket;
    }

    void WSServer::processTextMessage(QString message) {
        QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
        if (pClient) {
            pClient->sendTextMessage(message);
        }
    }

    void WSServer::processBinaryMessage(QByteArray message) {
        QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
        if (pClient) {
            pClient->sendBinaryMessage(message);
        }
    }

    void WSServer::socketDisconnected() {
        QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
        if (pClient) {
            clients_.removeAll(pClient);
            pClient->deleteLater();
        }
    }

}