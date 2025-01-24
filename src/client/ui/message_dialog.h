//
// Created by RGAA on 2023/8/19.
//

#ifndef SAILFISH_CLIENT_PC_MESSAGEDIALOG_H
#define SAILFISH_CLIENT_PC_MESSAGEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>

#include <memory>

namespace tc
{

    class ClientContext;

    enum DialogButton {
        kCancel,
        kSure,
    };

    class MessageDialog : public QDialog {
    public:

        static std::shared_ptr<MessageDialog> Make(const std::shared_ptr<ClientContext> &ctx, const QString &msg) {
            return std::make_shared<MessageDialog>(ctx, msg, nullptr);
        }

        explicit MessageDialog(const std::shared_ptr<ClientContext> &ctx, const QString &msg, QWidget *parent = nullptr);

        ~MessageDialog() override;

        void paintEvent(QPaintEvent *event) override;

    private:

        void CreateLayout();

    private:

        std::shared_ptr<ClientContext> context_ = nullptr;

        QString msg_;

    };

}

#endif //SAILFISH_CLIENT_PC_MESSAGEDIALOG_H
