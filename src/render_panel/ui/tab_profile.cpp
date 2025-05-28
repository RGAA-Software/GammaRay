//
// Created by RGAA on 22/03/2025.
//

#include "tab_profile.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "no_margin_layout.h"
#include "tc_pushbutton.h"
#include "tc_common_new/log.h"

#include <Windows.h>
#include <shellapi.h>

namespace tc
{

    TabProfile::TabProfile(const std::shared_ptr<GrApplication>& app, QWidget *parent)
        : TabBase(app, parent) {
        auto hwnd = HWND(winId());
        ::DragAcceptFiles(hwnd, TRUE);
        ::ChangeWindowMessageFilterEx(hwnd, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
        ::ChangeWindowMessageFilterEx(hwnd, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
        ::ChangeWindowMessageFilterEx(hwnd, 0x0049, MSGFLT_ALLOW, nullptr);

        setAcceptDrops(true);

        auto layout = new NoMarginHLayout();
        auto btn = new TcPushButton(this);
        btn->setText("--------------------");
        btn->setFixedSize(200, 50);
        btn->setAcceptDrops(true);
        setLayout(layout);
    }

    void TabProfile::OnTabShow() {

    }

    void TabProfile::OnTabHide() {

    }

    void TabProfile::dragEnterEvent(QDragEnterEvent *event) {
        event->accept();
        if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        }
        LOGI("DragEventEnter....");
    }

    void TabProfile::dragMoveEvent(QDragMoveEvent *event) {
        event->accept();
        LOGI("DragEventMove....");
    }

    void TabProfile::dropEvent(QDropEvent *event) {
        QList<QUrl> urls = event->mimeData()->urls();
        LOGI("DragEventDrop....{}", urls.size());
        if (urls.isEmpty()) {
            return;
        }
        std::vector<QString> files;
        for (const auto& url : urls) {
            files.push_back(url.toLocalFile());
            LOGI("Drop files: {}", url.toLocalFile().toStdString());
        }
        //if (file_transfer_) {
        //    file_transfer_->SendFiles(files);
        //}
    }

}