//
// Created by hy on 2024/1/25.
//

#ifndef TC_APPLICATION_CONNECTION_H
#define TC_APPLICATION_CONNECTION_H


#include <memory>
#include <string>

namespace tc
{

    class Data;
    class Context;
    class Application;
    class MessageProcessor;

    class Connection {
    public:

        explicit Connection(const std::shared_ptr<Application>& app);
        virtual ~Connection();

        virtual void Start();
        virtual void Exit();

        virtual void PostMediaMessage(const std::string& data);
        virtual void PostControlMessage(const std::string& data);
        virtual void PostIpcMessage(const std::string& data);

        virtual int GetConnectionPeerCount();

        virtual void NotifyPeerConnected();
        virtual void NotifyPeerDisconnected();

    protected:

        std::shared_ptr<Application> app_ = nullptr;
        std::shared_ptr<Context> context_ = nullptr;
        std::string ip_{};
        int port_ = 0;
        std::shared_ptr<MessageProcessor> msg_processor_ = nullptr;

    };

}


#endif //TC_APPLICATION_CONNECTION_H
