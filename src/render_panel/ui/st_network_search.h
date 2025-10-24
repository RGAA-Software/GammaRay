//
// Created by RGAA on 23/10/2025.
//

#ifndef GAMMARAYPREMIUM_ST_NETWORK_SEARCH_H
#define GAMMARAYPREMIUM_ST_NETWORK_SEARCH_H

#include <vector>
#include <QWidget>
#include <QListWidget>
#include "tc_qt_widget/tc_custom_titlebar_dialog.h"

namespace tc
{

    class TcLabel;
    class GrContext;
    class GrApplication;
    class MessageListener;
    class StNetworkSpvrAccessInfo;

    class StNetworkSearch : public TcCustomTitleBarDialog {
    public:
        explicit StNetworkSearch(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~StNetworkSearch() override;
        void closeEvent(QCloseEvent *) override;
        void OnItemClicked(int index, const std::shared_ptr<StNetworkSpvrAccessInfo>& item_info);
        std::shared_ptr<StNetworkSpvrAccessInfo> GetSelectedItem();
        void resizeEvent(QResizeEvent *) override;

    private:
        void CreateLayout();
        void UpdateItems();
        QListWidgetItem* AddItem(int index, const std::shared_ptr<StNetworkSpvrAccessInfo>& item_info);

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        TcLabel* empty_lbl_ = nullptr;
        QListWidget* list_widget_ = nullptr;
        std::shared_ptr<StNetworkSpvrAccessInfo> selected_item_ = nullptr;
    };

}

#endif //GAMMARAYPREMIUM_ST_NETWORK_SEARCH_H
