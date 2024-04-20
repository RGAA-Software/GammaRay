//
// Created by RGAA on 2024/1/25.
//

#include "ws_server.h"

#include "tc_common_new/log.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"

#include "message_processor.h"

namespace tc
{

    WSServer::WSServer(const std::shared_ptr<Application>& app)
            : Connection(app) {
    }

    WSServer::~WSServer() {

    }

    void WSServer::Start() {
        ws_server_ = std::make_shared<server>();

        auto task = [this](){
            try {
                ws_server_->set_access_channels(websocketpp::log::alevel::none);
                //ws_server_->clear_access_channels(websocketpp::log::alevel::frame_payload);

                ws_server_->init_asio();
                ws_server_->set_message_handler([=, this](websocketpp::connection_hdl hdl, message_ptr msg) {
                    ProcessMessage(hdl, msg);
                });

                ws_server_->set_open_handler([=, this](websocketpp::connection_hdl hdl) {
//                    if (!Settings::Instance()->IsMultiClientsEnabled()) {
//                        auto conn_count = GetConnectionPeerCount();
//                        if (conn_count >= 1) {
//                            ws_server_->close(hdl, websocketpp::close::status::normal, "config as single client...");
//                            LOGI("Close new connection because we have no permission for multi clients.");
//                            return;
//                        }
//                    }
                    AddSession(hdl);
                    NotifyPeerConnected();
                    LOGI("Open...");
                });

                ws_server_->set_close_handler([=, this](websocketpp::connection_hdl hdl) {
                    RemoveSession(hdl);
                    NotifyPeerDisconnected();
                    LOGI("Close...");
                });

                ws_server_->set_fail_handler([=, this](websocketpp::connection_hdl hdl) {
                    RemoveSession(hdl);
                    LOGI("Failed...");
                });

                LOGI("PORT::: {}", port_);
                ws_server_->listen(port_);
                ws_server_->start_accept();
                ws_server_->run();
                LOGI("After ws_server run...{}", port_);
            }
            catch (websocketpp::exception const& e) {
                LOGE("The port : {} may already used, error : {}", port_, e.what());
            }
            catch (...) {
                LOGE("The port : {} may already used, error ", port_);
            }
        };

        ws_thread_ = std::make_shared<Thread>(task, "", false);
    }

    void WSServer::Exit() {
        if (ws_server_) {
            ws_server_->stop();
            ws_server_.reset();
            LOGI("WS Server exit");
        }
        if (ws_thread_ && ws_thread_->IsJoinable()) {
            ws_thread_->Join();
            LOGI("WS Server thread exit");
        }
    }

    void WSServer::PostMediaMessage(const std::string &data) {
        if (!ws_server_) {
            return;
        }

        std::lock_guard<std::mutex> guard(session_mtx_);
        if (sessions_.empty()) {
            return;
        }
        for (auto& [handle, session] : sessions_) {
            try {
                ws_server_->send(handle, data, binary);
            }
            catch (std::exception &e) {
                LOGE("Send binary error : {}", e.what());
            }
        }
    }

    void WSServer::ProcessMessage(websocketpp::connection_hdl hdl, message_ptr msg) {

        //std::cout << "WSServer ProcessMessage" << std::endl;

        std::lock_guard<std::mutex> guard(session_mtx_);
        auto session = GetSession(hdl);
        if (!session) {
            LOGE("Not find session !");
            return;
        }
        // to do 判断session是否是主控
        if (msg->get_opcode() == text) {
            std::string value = msg->get_payload();
            LOGI("Text Msg: {}", value);
        }
        else if (msg->get_opcode() == binary) {

        }
        std::string value = msg->get_payload();

        // to do 这里要不要专门弄一个线程处理消息
        msg_processor_->HandleMessage(value);

//
//        this->msg_proc_thread->Post(SimpleThreadTask::Make([=]() {
//            if (message->has_mouse_info() || message->has_keyboard_info()) {
//                replayer->Replay(message);
//            }
//            else if (message->has_message_ack()) {
//                MessageACK ack = message->message_ack();
//                auto duration = GetCurrentTimestamp() - ack.send_time();
//                //std::cout << "ack , type : " << ack.type() << " frame index : " << ack.frame_index()
//                //	<< " duration : " << duration/2 << std::endl;
//            }
//        }));

    }

    void WSServer::AddSession(websocketpp::connection_hdl hdl) {
        std::lock_guard<std::mutex> guard(session_mtx_);
        auto session = WSSession::Make(hdl);
        sessions_.insert(std::pair<websocketpp::connection_hdl, WSSessionPtr>(hdl, session));

    }

    void WSServer::RemoveSession(websocketpp::connection_hdl hdl) {
        std::lock_guard<std::mutex> guard(session_mtx_);
        auto it = sessions_.find(hdl);
        if (it != sessions_.end()) {
            sessions_.erase(it);
        }
    }

    std::shared_ptr<WSSession> WSServer::GetSession(websocketpp::connection_hdl hdl) {
        if (sessions_.find(hdl) != sessions_.end()) {
            return sessions_[hdl];
        }
        return nullptr;
    }

    std::shared_ptr<WSSession> WSServer::GetSessionLocked(websocketpp::connection_hdl hdl) {
        std::lock_guard<std::mutex> guard(session_mtx_);
        if (sessions_.find(hdl) != sessions_.end()) {
            return sessions_[hdl];
        }
        return nullptr;
    }

    int WSServer::GetConnectionPeerCount() {
        std::lock_guard<std::mutex> guard(session_mtx_);
        return sessions_.size();
    }

}
