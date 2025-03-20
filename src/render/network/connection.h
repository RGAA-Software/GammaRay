//
// Created by RGAA on 2024/1/25.
//

#ifndef TC_APPLICATION_CONNECTION_H
#define TC_APPLICATION_CONNECTION_H

// DISABLED
#if 0

#include <memory>
#include <string>

namespace tc
{

    class Data;
    class RdContext;
    class RdApplication;
    class MessageProcessor;

    class Connection {
    public:

        explicit Connection(const std::shared_ptr<RdApplication>& app);
        virtual ~Connection();

        virtual void Start();
        virtual void Exit();

        virtual void PostVideoMessage(const std::string& data);
        virtual void PostAudioMessage(const std::string& data);
        virtual void PostNetMessage(const std::string& data);
        virtual void PostIpcMessage(const std::string& data);
        virtual int GetConnectionPeerCount();
        virtual void NotifyPeerConnected();
        virtual void NotifyPeerDisconnected();
        virtual bool OnlyAudioClient();

    protected:

        std::shared_ptr<RdApplication> app_ = nullptr;
        std::shared_ptr<RdContext> context_ = nullptr;
        std::string ip_{};
        int port_ = 0;
        std::shared_ptr<MessageProcessor> msg_processor_ = nullptr;

    };

}

#endif

#endif //TC_APPLICATION_CONNECTION_H
