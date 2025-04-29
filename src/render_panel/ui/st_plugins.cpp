//
// Created by RGAA on 29/04/2025.
//

#include "st_plugins.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "tc_common_new/message_notifier.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"

namespace tc
{

    StPlugins::StPlugins(const std::shared_ptr<GrApplication>& app, QWidget* parent) : TabBase(app, parent) {
        msg_listener_->Listen<MsgPluginsInfo>([=, this](const MsgPluginsInfo& m_info) {
            //LOGI("----");
            auto plugins_info = m_info.plugins_info_->plugins_info();
            for (int i = 0; i < plugins_info.size(); i++) {
                auto info = plugins_info.at(i);
                //LOGI("{}:{} => {}", info.author(), info.name(), info.desc());
            }
        });
    }

    void StPlugins::OnTabShow() {

    }

    void StPlugins::OnTabHide() {

    }


}