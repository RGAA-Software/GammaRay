//
// Created by RGAA on 20/03/2025.
//

#ifndef GAMMARAY_MAINWINDOW_WRAPPER_H
#define GAMMARAY_MAINWINDOW_WRAPPER_H

#include <QMainWindow>

namespace tc
{

    class MessageNotifier;

    class MainWindowWrapper {
    public:
        explicit MainWindowWrapper(const std::shared_ptr<MessageNotifier>& notifier, QMainWindow* window);
        void Setup(const QString& title);

    private:
        QMainWindow* window_ = nullptr;
        std::shared_ptr<MessageNotifier> notifier_ = nullptr;
    };

}

#endif //GAMMARAY_MAINWINDOW_WRAPPER_H
