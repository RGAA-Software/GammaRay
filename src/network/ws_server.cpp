//
// Created by RGAA on 2024-03-30.
//

#include "ws_server.h"
#include "gr_settings.h"
#include "tc_common_new/log.h"
#include "tc_message.pb.h"
#include "gr_context.h"
#include "app_messages.h"

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
            connect(ws_server_, &QWebSocketServer::newConnection, this, &WSServer::OnNewConnection);
            connect(ws_server_, &QWebSocketServer::closed, this, &WSServer::Closed);
        }
    }

    void WSServer::Exit() {

    }

    WSServer::~WSServer() {
        ws_server_->close();
        qDeleteAll(clients_.begin(), clients_.end());
    }

    void WSServer::OnNewConnection() {
        QWebSocket* socket = ws_server_->nextPendingConnection();
        connect(socket, &QWebSocket::textMessageReceived, this, &WSServer::ProcessTextMessage);
        connect(socket, &QWebSocket::binaryMessageReceived, this, &WSServer::ProcessBinaryMessage);
        connect(socket, &QWebSocket::disconnected, this, &WSServer::SocketDisconnected);
        clients_ << socket;
    }

    void WSServer::ProcessTextMessage(const QString& message) {
        auto client = qobject_cast<QWebSocket *>(sender());
    }

    void WSServer::ProcessBinaryMessage(const QByteArray& message) {
        auto client = qobject_cast<QWebSocket*>(sender());
        //
        this->ParseBinaryMessage(message);
    }

    void WSServer::SocketDisconnected() {
        auto client = qobject_cast<QWebSocket *>(sender());
        if (client) {
            clients_.removeAll(client);
            client->deleteLater();
        }
    }

    void WSServer::Closed() {

    }

    void WSServer::ParseBinaryMessage(const QByteArray& msg) {
        auto msg_str = msg.toStdString();
        auto proto_msg = std::make_shared<tc::Message>();
        if (!proto_msg->ParseFromString(msg_str)) {
            LOGE("Parse binary message failed.");
            return;
        }
        auto capture_statistics = proto_msg->capture_statistics();
        context_->SendAppMessage(MsgCaptureStatistics {
           .msg_ = proto_msg,
           .statistics_ = capture_statistics,
        });
    }

}