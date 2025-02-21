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
    class Settings;
    class ClientContext;

    class MainProgress : public QLabel {
    public:
        MainProgress(const std::shared_ptr<ThunderSdk>& sdk, const std::shared_ptr<ClientContext>& ctx, QWidget* parent);
        void ResetProgress();
        void StepForward();
        void CompleteProgress();
        int GetCurrentProgress();
        void paintEvent(QPaintEvent *event) override;

    private:
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        std::shared_ptr<ClientContext> context_ = nullptr;
        Settings* settings_ = nullptr;
        QPixmap bg_pixmap_;
        QLabel* lbl_main_message_ = nullptr;
        QLabel* lbl_sub_message_ = nullptr;
        QProgressBar* progress_bar_ = nullptr;
        std::atomic_int progress_steps_ = { 0 };
    };

}

#endif //GAMMARAY_CT_MAIN_PROGRESS_H
