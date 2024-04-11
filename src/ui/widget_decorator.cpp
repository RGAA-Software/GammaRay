#pragma execution_character_set("utf-8")
#include "widget_decorator.h"

#include <QMenu>

#include <QScrollBar>
#include <boost/format.hpp>

namespace tc
{

    WidgetDecorator::WidgetDecorator() {

    }

    void WidgetDecorator::DecorateSearchBar(QLineEdit *search, QPushButton *btn_search, QPushButton *btn_clear,
                                            const QString &msg) {
//        auto search_style = R"(QLineEdit{
//                            background-color: #eeeeee;
//                            border: 6px solid #FF0000;
//                            border-radius: 0px;
//                            padding: 0 0px 0 0;
//                            selection-background-color: darkgray;
//                        }
//                        QLineEdit:focus {
//                            background-color:#ffffff;
//                        }
//                        )";
//        //search->setMinimumHeight((30));
//        if (msg.isEmpty()) {
//            search->setPlaceholderText(QObject::tr("请输入您的搜索内容"));
//        } else {
//            search->setPlaceholderText(msg);
//        }
//        search->setStyleSheet(search_style);
//        QFont font(SysConfig::Instance()->GetSysFont().c_str());
//        font.setPixelSize((10));
//        search->setFont(font);
//        search->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
//        search->grabKeyboard();
//        auto small_btn_style = R"(
//                               QPushButton {
//                                   background-color: #cccccc;border-style: solid;border-radius:15px;max-width:30px;max-height:30px;min-width:30px;min-height:30px;
//                               }
//
//                               QPushButton:hover {
//                                   background-color: #bbbbbb;border-style: solid;border-radius:15px;max-width:30px;max-height:30px;min-width:30px;min-height:30px;
//                               }
//
//                               QPushButton:pressed, QPushButton:checked {
//                                   background-color: #aaaaaa;border-style: solid;border-radius:15px;max-width:30px;max-height:30px;min-width:30px;min-height:30px;
//                               }
//
//                           )";
//        auto small_size = skScaleQSize(QSize(20, 20));
//        if (btn_search) {
//            auto prev_btn_icon = new QImage();
//            prev_btn_icon->load(":/images/resources/icon_search.png");
//            btn_search->setMinimumHeight((40));
//            btn_search->setText("");
//            btn_search->setIcon(QPixmap::fromImage(*prev_btn_icon));
//            btn_search->setIconSize(small_size);
//            btn_search->setStyleSheet(small_btn_style);
//        }
//        if (btn_clear) {
//            auto add_btn_icon = new QImage();
//            add_btn_icon->load(":/images/resources/ic_close.png");
//            btn_clear->setMinimumHeight((40));
//            btn_clear->setText("");
//            btn_clear->setIcon(QPixmap::fromImage(*add_btn_icon));
//            btn_clear->setIconSize(small_size);
//            btn_clear->setStyleSheet(small_btn_style);
//        }
    }

    void WidgetDecorator::DecorateListWidget(QListWidget *list) {
        auto play_list_style = R"(
                           QListWidget{color:#333333; background: #eeeeee;border-style:none}
                           QListWidget::Item{border:0px solid gray;padding-left:0;}
                           QListWidget::Item:hover{color: #333333; background:#bbbbbb; border:0px solid gray;}
                           QListWidget::Item:selected{background:#bbbbbb;color:#333333;border:0px solid gray;}
                           QListWidget::Item:selected:active{background:#bbbbbb;color:#333333;border-width:0;}
                           )";
        list->setStyleSheet(play_list_style);
        list->setContextMenuPolicy(Qt::CustomContextMenu);

        auto scroll_style = R"(

                        QScrollBar:vertical {
                             background: #dddddd;
                             width: 9px;
                             margin: 0px 0 0px 0;
                        }
                         QScrollBar::handle:vertical {
                             background: rgb(195, 195, 195);
                             min-height: 20px;
                             margin: 0 1px 0 2px;
                             border-radius: 3px;
                             border: none;
                         /*background: qlineargradient(spread:reflect,
                                 x1:0, y1:0, x2:1, y2:0,
                                 stop:0 rgba(164, 164, 164, 255),
                                 stop:0.5 rgba(120, 120, 120, 255),
                                 stop:1 rgba(164, 164, 164, 255));*/
                         /*border-image: url(images/scrollbar-vertical-thumb.png) 8px 0 8px 0 fixed;*/
                        }

                         QScrollBar::add-line:vertical {
                             background: url(images/scrollbar-vertical-bg.png);
                             height: 0px;
                             subcontrol-position: bottom;
                             subcontrol-origin: margin;
                        }

                         QScrollBar::sub-line:vertical {
                             background: url(images/scrollbar-vertical-bg.png);
                             height: 0px;
                             subcontrol-position: top;
                             subcontrol-origin: margin;
                        }
                        /*QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {
                            border: 1px solid grey;
                            width: 3px;
                            height: 3px;
                            background: white;
                       }*/
                         QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                             background: none;
                        }

                        )";
        list->verticalScrollBar()->setStyleSheet(scroll_style);
    }

    void WidgetDecorator::DecorateBottomBar() {

    }


    void WidgetDecorator::DecoratePopMenu(QMenu *menu) {
        menu->setStyleSheet(
                R"(
                    QMenu {
                        background-color : rgb(253,253,254);
                     padding:5px;
                     border-radius:0px;
                    }
                    QMenu::item {
                        font-size:10pt;
                        color: rgb(0,0,0);
                        background-color:rgb(253,253,254);
                        padding: 8px 25px 6px 10px;
                        margin: 4px 1px;
                    }
                    QMenu::item:selected {
                        background-color : rgb(236,236,237);
                    }
                    QMenu::icon:checked {
                        background: rgb(253,253,254);
                        position: absolute;
                        top: 1px;
                        right: 1px;
                        bottom: 1px;
                        left: 1px;
                    }
                    QMenu::icon:checked:selected {
                        background-color : rgb(236,236,237);
                        background-image: url(:/space_selected.png);
                    }
                    QMenu::separator {
                        height: 2px;
                        background: rgb(235,235,236);
                        margin-left: 10px;
                        margin-right: 10px;
                    }
                    )"

        );
    }

    void WidgetDecorator::DecorateIndexBtn(QPushButton *btn) {
        auto small_btn_style = R"(
                               QPushButton {
                                   background-color: #cccccc;border-style: solid;border-radius:5px;max-width:50px;max-height:30px;min-width:50px;min-height:30px;
                               }

                               QPushButton:hover {
                                   background-color: #bbbbbb;border-style: solid;border-radius:5px;max-width:50px;max-height:30px;min-width:50px;min-height:30px;
                               }

                               QPushButton:pressed, QPushButton:checked {
                                   background-color: #aaaaaa;border-style: solid;border-radius:5px;max-width:50px;max-height:30px;min-width:50px;min-height:30px;
                               }

                           )";
        btn->setStyleSheet(small_btn_style);
    }

    void WidgetDecorator::DecorateDialogBtn(QPushButton *sure, QPushButton *cancel) {
//        auto sure_btn_style = R"(
//                               QPushButton {
//                                   color: #ffffff;
//                                   background-color: #2d17bc;border-style: solid;border-radius:5px;
//                               }
//
//                               QPushButton:hover {
//                                   color: #ffffff;
//                                   background-color: #4516be;border-style: solid;border-radius:5px;
//                               }
//
//                               QPushButton:pressed, QPushButton:checked {
//                                   color: #ffffff;
//                                   background-color: #6313c0;border-style: solid;border-radius:5px;
//                               }
//
//                           )";
//        sure->setStyleSheet(sure_btn_style);
//        auto font = SysConfig::Instance()->sys_font_12;
//        //font.setBold(true);
//        sure->setFont(font);
//
//        auto cancel_btn_style = R"(
//                               QPushButton {
//                                   color: #ffffff;
//                                   background-color: #bbbbbb;border-style: solid;border-radius:5px;
//                               }
//
//                               QPushButton:hover {
//                                   color: #ffffff;
//                                   background-color: #dd0000;border-style: solid;border-radius:5px;
//                               }
//
//                               QPushButton:pressed, QPushButton:checked {
//                                   color: #ffffff;
//                                   background-color: #cc0000;border-style: solid;border-radius:5px;
//                               }
//
//                           )";
//        if (cancel) {
//            cancel->setStyleSheet(cancel_btn_style);
//            cancel->setFont(font);
//        }
    }

    void WidgetDecorator::DecorateMenuBar(QMenuBar *mb) {
        auto style = R"(
        
        QMenuBar {
            background-color: #ffffff;
        }

        QMenuBar::item {
            padding-left: 16px;
            padding-right: 16px;
            padding-top:3px;
            padding-bottom: 3px;
            margin-right:3px;
            background: #ffffff;
            border-radius: 0px;
        }

        QMenuBar::item:selected {
            background: #cccccc;
        }

        QMenuBar::item:pressed {
            background: #bbbbbb;
        }

        QMenu {
            background-color : rgb(253,253,254);
            padding:5px;
            border-radius:15px;
        }
        QMenu::item {
            font-size:10pt;
            color: rgb(0,0,0);
            background-color:rgb(253,253,254);
            padding: 8px 25px 6px 10px;
            margin: 4px 1px;
        }
        QMenu::item:selected {
            background-color : rgb(236,236,237);
        }
        QMenu::icon:checked {
            background: rgb(253,253,254);
            position: absolute;
            top: 1px;
            right: 1px;
            bottom: 1px;
            left: 1px;
        }
        QMenu::icon:checked:selected {
            background-color : rgb(236,236,237);
            background-image: url(:/space_selected.png);
        }
        QMenu::separator {
            height: 2px;
            background: rgb(235,235,236);
            margin-left: 10px;
            margin-right: 10px;
        }

    )";

        mb->setStyleSheet(style);
    }

    void WidgetDecorator::DecoratePropertyEdit(QLineEdit *edit) {
//        auto style = R"(QLineEdit{
//                            background-color: #eeeeee;
//                            border: 1px solid #bbbbbb;
//                            border-radius: 5px;
//                            padding: 0 8px;
//                            selection-background-color: darkgray;
//                            font-size: 14px;
//                        }
//                        QLineEdit:focus {
//                            background-color:#ffffff;
//                        }
//                        )";
//        edit->setMinimumHeight(25);
//        edit->setStyleSheet(style);
//        edit->setFont(SysConfig::Instance()->sys_font_8);
//        edit->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
//        //edit->grabKeyboard();
    }

    void WidgetDecorator::DecorateLineEdit(QLineEdit *edit, int margin_left, int margin_right) {
//        boost::format fmt(R"(QLineEdit{
//                        background-color: #eeeeee;
//                        border: 1px solid #bbbbbb;
//                        border-radius: 5px;
//                        padding: 0 8px;
//                        selection-background-color: darkgray;
//                        margin-left: %1%px;
//                        margin-right: %2%px;
//                    }
//                    QLineEdit:focus {
//                        background-color:#ffffff;
//                    }
//                    )");
//        fmt % margin_left;
//        fmt % margin_right;
//        //edit->setMinimumHeight(25);
//        edit->setStyleSheet(fmt.str().c_str());
//        edit->setFont(SysConfig::Instance()->sys_font_14);
//        edit->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    }

    void WidgetDecorator::DecorateLabel(QLabel *label, int margin_left, int margin_right, int font_size) {
        boost::format fmt(R"(QLabel{
                        background-color: #ffffff;
                        border: 0px solid #bbbbbb;
                        border-radius: 0px;
                        padding: 0 0px;
                        selection-background-color: #ffffff;
                        font-size: %3%px;
                        margin-left: %1%px;
                        margin-right: %2%px;
                    }
                    QLabel:focus {
                        background-color:#ffffff;
                    }
                    )");
        fmt % margin_left;
        fmt % margin_right;
        fmt % font_size;
        label->setMinimumHeight(25);
        label->setStyleSheet(fmt.str().c_str());
        //label->setFont(SysConfig::Instance()->sys_font_8);
        label->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    }

    void WidgetDecorator::DecorateParticleIcon(QPushButton *btn, int size) {
        QString style = R"(
                        QPushButton {
                            color: #ffffff;
                            background-color: #bbbbbb;border-style: solid;border-radius:%1px;width:%2px;height:%3px;
                        }

                        QPushButton:hover {
                            color: #ffffff;
                            background-color: #2d17bc; 
                        }

                        QPushButton:pressed, QPushButton:checked {
                            color: #ffffff;
                            background-color: #6313c0;
                        }

                        )";
        style = style.arg(size / 2).arg(size).arg(size);
        btn->setStyleSheet(style);
    }

    void WidgetDecorator::DecorateComboBox(QComboBox *box, int height) {
//        box->setStyleSheet(Style::GetComboBoxStyle(height).c_str());
//        auto listview = new QListView();
//        listview->setFont(SysConfig::Instance()->sys_font_11);
//        box->setView(listview);
//
//        box->setFont(SysConfig::Instance()->sys_font_11);
    }

    void WidgetDecorator::DecorateCheckBox(QCheckBox *box, int size) {
        QString style = R"(
        QCheckBox::indicator,QGroupBox::indicator,QTreeView::indicator,QListView::indicator,QTableView::indicator{
            width:%1px;
            height:%1px;
        }

        QCheckBox::indicator:unchecked,QGroupBox::indicator:unchecked,QTreeView::indicator:unchecked,QListView::indicator:unchecked,QTableView::indicator:unchecked{
            image:url(:/qss/lightblue/checkbox_unchecked.png);
        }

        QCheckBox::indicator:unchecked:disabled,QGroupBox::indicator:unchecked:disabled,QTreeView::indicator:unchecked:disabled,QListView::indicator:unchecked:disabled,QTableView::indicator:unchecked:disabled{
            image:url(:/qss/lightblue/checkbox_unchecked_disable.png);
        }

        QCheckBox::indicator:checked,QGroupBox::indicator:checked,QTreeView::indicator:checked,QListView::indicator:checked,QTableView::indicator:checked{
            image:url(:/qss/lightblue/checkbox_checked.png);
        }

        QCheckBox::indicator:checked:disabled,QGroupBox::indicator:checked:disabled,QTreeView::indicator:checked:disabled,QListView::indicator:checked:disabled,QTableView::indicator:checked:disabled{
            image:url(:/qss/lightblue/checkbox_checked_disable.png);
        }

        QCheckBox::indicator:indeterminate,QGroupBox::indicator:indeterminate,QTreeView::indicator:indeterminate,QListView::indicator:indeterminate,QTableView::indicator:indeterminate{
            image:url(:/qss/lightblue/checkbox_parcial.png);
        }

        QCheckBox::indicator:indeterminate:disabled,QGroupBox::indicator:indeterminate:disabled,QTreeView::indicator:indeterminate:disabled,QListView::indicator:indeterminate:disabled,QTableView::indicator:indeterminate:disabled{
            image:url(:/qss/lightblue/checkbox_parcial_disable.png);
        }
    )";
        style = style.arg((size)).arg((size));
        box->setStyleSheet(style);
    }

}