//
// Created by RGAA on 2023/8/16.
//

#ifndef SAILFISH_SERVER_INFORMATIONCONTENT_H
#define SAILFISH_SERVER_INFORMATIONCONTENT_H

#include "app_content.h"

#include "db/stream_item.h"

namespace tc
{

    class ClientContext;
    class AppStreamList;

    using OnStartingStreamCallback = std::function<void(const StreamItem&)>;

    class AddButton : public QLabel {
    public:

        explicit AddButton(QWidget* parent = nullptr);

        void SetOnClickCallback(std::function<void()>&& cbk) {
            click_cbk_ = std::move(cbk);
        }

        void paintEvent(QPaintEvent *) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mousePressEvent(QMouseEvent *ev) override;
        void mouseReleaseEvent(QMouseEvent *ev) override;

    private:
        std::function<void()> click_cbk_;

        bool enter_ = false;
        bool pressed_ = false;

    };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

    class StreamContent : public AppContent {
    public:

        explicit StreamContent(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        ~StreamContent() override;

        void OnContentShow() override;
        void OnContentHide() override;

        void resizeEvent(QResizeEvent *event) override;

        void ShowEmptyTip();
        void HideEmptyTip();

    private:

        AppStreamList* stream_list_ = nullptr;
        AddButton* add_btn_ = nullptr;
        QLabel* empty_tip_ = nullptr;

    };

}

#endif //SAILFISH_SERVER_INFORMATIONCONTENT_H
