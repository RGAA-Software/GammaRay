//
// Created by RGAA on 12/12/2025.
//

#include "gr_spvr_manager.h"

#include "tc_dialog.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_application.h"
#include "translator/tc_translator.h"

namespace tc
{

    GrSpvrManager::GrSpvrManager(const std::shared_ptr<GrContext>& context) {
        context_ = context;
        settings_ = GrSettings::Instance();
    }

    std::optional<spvr::AliveConnections> GrSpvrManager::QueryAliveConnections(bool show_err_dialog) const {
        const auto host = settings_->GetSpvrServerHost();
        const auto port = settings_->GetSpvrServerPort();
        const auto appkey = grApp->GetAppkey();
        const auto r = spvr::SpvrApi::QueryAliveConnections(host, port, appkey);
        if (!r.has_value()) {
            if (show_err_dialog) {
                auto err = r.error();
                context_->PostUITask([=]() {
                    const QString msg = tcTr("id_op_error") + ":" + QString::number(static_cast<int>(err)) + " " + spvr::SpvrApiErrorAsString(err).c_str();
                    TcDialog dialog(tcTr("id_error"), msg);
                    dialog.exec();
                });
            }
            return std::nullopt;
        }
        return r.value();
    }

    std::optional<spvr::AvailableNewConnection> GrSpvrManager::QueryNewConnection(bool show_err_dialog) const {
        const auto host = settings_->GetSpvrServerHost();
        const auto port = settings_->GetSpvrServerPort();
        const auto appkey = grApp->GetAppkey();
        if (host.empty() || port <= 0 || appkey.empty()) {
            return std::nullopt;
        }
        const auto r = spvr::SpvrApi::QueryAvailableNewConnection(host, port, appkey);
        if (!r.has_value()) {
            if (show_err_dialog) {
                auto err = r.error();
                context_->PostUITask([=]() {
                    const QString msg = tcTr("id_op_error") + ":" + QString::number(static_cast<int>(err)) + " " + spvr::SpvrApiErrorAsString(err).c_str();
                    TcDialog dialog(tcTr("id_error"), msg);
                    dialog.exec();
                });
            }
            return std::nullopt;
        }
        return r.value();
    }

}
