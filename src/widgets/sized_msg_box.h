//
// Created by hy on 2024/1/17.
//

#ifndef PLC_CONTROLLER_SIZED_MSG_BOX_H
#define PLC_CONTROLLER_SIZED_MSG_BOX_H

#include <QMessageBox>
#include <QString>
#include <memory>

namespace tc
{
    class SizedMessageBox : public QMessageBox {
    public:

        static std::shared_ptr<SizedMessageBox> MakeOkCancelBox(const QString& title, const QString& msg);
        static std::shared_ptr<SizedMessageBox> MakeOkBox(const QString& title, const QString& msg);
        static std::shared_ptr<SizedMessageBox> MakeErrorOkBox(const QString& title, const QString& msg);
        static std::shared_ptr<SizedMessageBox> MakeInfoOkBox(const QString& title, const QString& msg);

        explicit SizedMessageBox(QWidget* parent = nullptr) : QMessageBox(parent) {
        }

        void Resize(int width, int height) {
            this->width_ = width;
            this->height_ = height;
        }

        void resizeEvent(QResizeEvent *event) override {
            setFixedSize(width_, height_);
        }

        void closeEvent(QCloseEvent *event) override {
            if (closed_callback_) {
                closed_callback_();
            }
            this->done(1);
        }

        void SetCloseCallback(std::function<void()>&& cbk) {
            closed_callback_ = std::move(cbk);
        }

    private:
        int width_{};
        int height_{};

        std::function<void()> closed_callback_;
    };
}

#endif //PLC_CONTROLLER_SIZED_MSG_BOX_H
