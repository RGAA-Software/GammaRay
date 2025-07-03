//
// Created by RGAA on 16/08/2024.
//

#ifndef GAMMARAYPC_CLIPBOARD_H
#define GAMMARAYPC_CLIPBOARD_H

#include <memory>
#include <QObject>
#include "tc_message.pb.h"
#include <objidl.h>

namespace tc
{

    class WinMessageLoop;
    class MessageListener;
    class CpVirtualFile;
    class ClientPluginContext;
    class ClientClipboardPlugin;

    class ClipboardManager : public QObject {
    public:
        explicit ClipboardManager(ClientClipboardPlugin* plugin);
        void Start();
        void Stop();
        void OnRemoteClipboardMessage(std::shared_ptr<tc::Message> msg);
        void OnRemoteClipboardRespMessage(std::shared_ptr<tc::Message> msg);
        void OnRemoteFileRespMessage(std::shared_ptr<tc::Message> msg);
        void OnLocalClipboardUpdated();

    private:
        ClientClipboardPlugin* plugin_ = nullptr;
        std::shared_ptr<ClientPluginContext> context_ = nullptr;
        QString remote_info_;
        std::shared_ptr<WinMessageLoop> msg_loop_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        CpVirtualFile* virtual_file_ = nullptr;
        IDataObject* data_object_ = nullptr;
    };

}

#endif //GAMMARAYPC_CLIPBOARD_H
