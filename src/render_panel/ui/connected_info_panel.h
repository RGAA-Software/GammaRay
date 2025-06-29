#pragma once 

#include <QWidget>
class QCheckBox;

namespace tc {

    class NoMarginVLayout;
    class NoMarginHLayout;
    class TcLabel;
    class TcPushButton;

    // 被客户端连接上来后，显示连接者的一些信息
    class ConnectedInfoPanel : public QWidget {
    public:
        ConnectedInfoPanel(QWidget* parent = nullptr);
    private:
        void InitView();
    private:
        NoMarginVLayout* root_vbox_layout_ = nullptr;

        NoMarginHLayout* logo_hbox_layout_ = nullptr;
        TcLabel* logo_lab_ = nullptr;
        TcLabel* logo_name_lab_ = nullptr;

        NoMarginHLayout* avatar_name_hbox_layout_ = nullptr;
        // 头像
        TcLabel* avatar_lab_ = nullptr;
        // 名字
        TcLabel* name_lab_ = nullptr;
        TcLabel* conn_prompt_lab_ = nullptr;
        // 断开连接
        TcPushButton* disconnect_btn_ = nullptr;

        NoMarginHLayout* promtp_hbox_layout_ = nullptr;
        TcLabel* prompt_lab_ = nullptr;

        NoMarginHLayout* access_control_hbox_layout_ = nullptr;
        // 声音
        QCheckBox* voice_cbox_ = nullptr;
        TcLabel* voice_lab_ = nullptr;
        // 键鼠
        QCheckBox* key_mouse_cbox_ = nullptr;
        TcLabel* key_mouse_lab_ = nullptr;
        // 文件
        QCheckBox* file_cbox_ = nullptr;
        TcLabel* file_lab_ = nullptr;
    };


}