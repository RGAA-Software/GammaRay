//
// Created by RGAA on 16/08/2024.
//

#ifndef GAMMARAYPC_CLIPBOARD_H
#define GAMMARAYPC_CLIPBOARD_H

#include <memory>
#include <QObject>

namespace tc
{

    class RdContext;
    class Message;

    class ClipboardManager : public QObject {
    public:
        explicit ClipboardManager(const std::shared_ptr<RdContext>& ctx);
        void Monitor();
        void UpdateRemoteInfo(std::shared_ptr<Message>&& msg);

    private:
        std::shared_ptr<RdContext> context_ = nullptr;
        QString remote_info_;
    };

}

#endif //GAMMARAYPC_CLIPBOARD_H
