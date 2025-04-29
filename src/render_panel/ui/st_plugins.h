//
// Created by RGAA on 29/04/2025.
//

#ifndef GAMMARAY_ST_PLUGINS_H
#define GAMMARAY_ST_PLUGINS_H

#include <QLabel>
#include "tab_base.h"

namespace tc
{
    class GrApplication;

    class StPlugins : public TabBase {
    public:
        explicit StPlugins(const std::shared_ptr<GrApplication>& app, QWidget* parent = nullptr);
        ~StPlugins() override = default;

        void OnTabShow() override;
        void OnTabHide() override;

    private:
        QLabel* license_ = nullptr;
    };

}



#endif //GAMMARAY_ST_PLUGINS_H
