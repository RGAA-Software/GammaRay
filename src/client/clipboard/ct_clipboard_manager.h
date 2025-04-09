//
// Created by RGAA on 16/08/2024.
//

#ifndef GAMMARAYPC_CLIPBOARD_H
#define GAMMARAYPC_CLIPBOARD_H

#include <memory>
#include <QObject>

namespace tc
{

    class ClientContext;

    class ClipboardManager : public QObject {
    public:
        explicit ClipboardManager(const std::shared_ptr<ClientContext>& ctx);
        void Monitor();
        void UpdateRemoteInfo(const QString& info);

    private:
        std::shared_ptr<ClientContext> context_ = nullptr;
        QString remote_info_;
    };

}

#endif //GAMMARAYPC_CLIPBOARD_H
