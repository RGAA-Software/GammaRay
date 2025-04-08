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
    class ClipboardPlugin;

    class ClipboardManager : public QObject {
    public:
        explicit ClipboardManager(ClipboardPlugin* plugin);
        void Monitor();
        void UpdateRemoteInfo(const std::shared_ptr<Message>& msg);

    private:
        QString remote_info_;
        ClipboardPlugin* plugin_ = nullptr;
    };

}

#endif //GAMMARAYPC_CLIPBOARD_H
