//
// Created by RGAA on 20/02/2025.
//

#ifndef GAMMARAY_CT_MAIN_PROGRESS_H
#define GAMMARAY_CT_MAIN_PROGRESS_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <memory>

namespace tc
{

    class ThunderSdk;

    class MainProgress : public QLabel {
    public:
        MainProgress(const std::shared_ptr<ThunderSdk>& sdk, QWidget* parent);

        void paintEvent(QPaintEvent *event) override;

    private:
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        QPixmap bg_pixmap_;
        QLabel* lbl_main_message_ = nullptr;
        QLabel* lbl_sub_message_ = nullptr;
        QProgressBar* progress_bar_ = nullptr;
    };

}

#endif //GAMMARAY_CT_MAIN_PROGRESS_H
