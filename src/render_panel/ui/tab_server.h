//
// Created by RGAA on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_TAB_SERVER_H
#define TC_SERVER_STEAM_TAB_SERVER_H

#include "tab_base.h"
#include "render_panel/database/stream_item.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QStackedWidget>
#include <QLabel>
#include <QComboBox>
#include <QProcess>

namespace tc
{
    class RnApp;
    class RnEmpty;
    class MessageListener;
    class QtCircle;
    class GrSettings;
    class TcQRWidget;
    class StreamContent;
    class TcImageButton;
    class RunningStreamManager;
    class StreamDBOperator;
    class TcPasswordInput;

    class TabServer : public TabBase {
    public:
        explicit TabServer(const std::shared_ptr<GrApplication>& app, QWidget *parent);
        ~TabServer() override;
        void OnTabShow() override;
        void OnTabHide() override;
        void RegisterMessageListener();
        void resizeEvent(QResizeEvent *event) override;

    private:
        void UpdateQRCode();

    private:
        GrSettings* settings_ = nullptr;
        QPixmap qr_pixmap_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        QLabel* lbl_machine_code_ = nullptr;
        QLabel* lbl_machine_random_pwd_ = nullptr;
        QLineEdit* lbl_detailed_info_ = nullptr;
        TcQRWidget* lbl_qr_code_ = nullptr;
        StreamContent* stream_content_ = nullptr;
        TcPasswordInput* password_input_ = nullptr;
        QComboBox* remote_devices_ = nullptr;
        std::shared_ptr<RunningStreamManager> running_stream_mgr_ = nullptr;
        std::shared_ptr<StreamDBOperator> stream_db_mgr_ = nullptr;
        std::vector<std::shared_ptr<StreamItem>> recent_streams_;
    };
}

#endif //TC_SERVER_STEAM_TAB_SERVER_H
