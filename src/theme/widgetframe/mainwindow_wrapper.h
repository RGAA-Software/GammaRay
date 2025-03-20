//
// Created by RGAA on 20/03/2025.
//

#ifndef GAMMARAY_MAINWINDOW_WRAPPER_H
#define GAMMARAY_MAINWINDOW_WRAPPER_H

#include <QMainWindow>

namespace tc
{

    class MainWindowWrapper {
    public:
        explicit MainWindowWrapper(QMainWindow* window);
        void Setup(const QString& title);

    private:
        QMainWindow* window_ = nullptr;
    };

}

#endif //GAMMARAY_MAINWINDOW_WRAPPER_H
