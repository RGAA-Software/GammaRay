//
// Created by RGAA on 2024/1/25.
//

#include "connection.h"

#include "app.h"
#include "context.h"
#include "tc_encoder_new/encoder_messages.h"
#include "settings/settings.h"
#include "network/message_processor.h"
#include "tc_capture_new/capture_message.h"

namespace tc
{

    Connection::Connection(const std::shared_ptr<Application>& app) {
        this->app_ = app;
        this->context_ = app->GetContext();
        this->ip_ = "0.0.0.0";
        this->port_ = Settings::Instance()->transmission_.listening_port_;
        this->msg_processor_ = std::make_shared<MessageProcessor>(this->app_);;
    }

    Connection::~Connection() {

    }

    void Connection::Start() {

    }

    void Connection::Exit() {

    }

    void Connection::PostVideoMessage(const std::string& data) {

    }

    void Connection::PostAudioMessage(const std::string& data) {

    }

    void Connection::PostNetMessage(const std::string& data) {

    }

    void Connection::PostIpcMessage(const std::string& data) {

    }

    int Connection::GetConnectionPeerCount() {
        return 0;
    }

    void Connection::NotifyPeerConnected() {
        context_->SendAppMessage(MsgInsertIDR {});
        context_->SendAppMessage(RefreshScreenMessage {});
    }

    void Connection::NotifyPeerDisconnected() {
        //context_->SendAppMessage(PeerDisconnectedMessage::Make());
    }

    bool Connection::OnlyAudioClient() {
        return false;
    }

}