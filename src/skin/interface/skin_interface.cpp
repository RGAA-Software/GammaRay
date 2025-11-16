//
// Created by RGAA on 22/05/2025.
//

#include "skin_interface.h"
#include "skin_context.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "snowflake/snowflake.h"
#include <QtCore/QTimer>
#include <QtCore/QEvent>

namespace tc
{

    std::shared_ptr<SkinContext> SkinInterface::GetPluginContext() {
        return skin_context_;
    }

    bool SkinInterface::OnCreate(const SkinParam& param) {
        SnowflakeId::initialize(0, 105);
        this->skin_param_ = param;
        if (param.cluster_.contains("name")) {
            auto n = param.cluster_.at("name");
            plugin_file_name_ = std::any_cast<std::string>(n);
        }

        if (param.cluster_.contains("base_path")) {
            base_path_ = std::any_cast<std::string>(param.cluster_.at("base_path"));
        }
        skin_context_ = std::make_shared<SkinContext>(GetSkinName().toStdString());

        Logger::InitLog(base_path_ + "/gr_logs/skin_" + plugin_file_name_+".log", true);
        LOGI("{} OnCreate", GetSkinName().toStdString());

        // print params
        LOGI("Input params size : {}", param.cluster_.size());
        for (const auto& [key, value]: param.cluster_) {
            if (value.type() == typeid(std::string)) {
                LOGI(" * {} => {}", key, std::any_cast<std::string>(value));
            }
            else if (value.type() == typeid(int64_t)) {
                LOGI(" * {} => {}", key, std::any_cast<int64_t>(value));
            }
            else if (value.type() == typeid(double)) {
                LOGI(" * {} => {}", key, std::any_cast<double>(value));
            }
            else if (value.type() == typeid(bool)) {
                LOGI(" * {} => {}", key, std::any_cast<bool>(value));
            }

            // parse
            if (key == "author") {
                //plugin_author_ = std::any_cast<std::string>(value);
            }
        }

        //root_widget_ = new QWidget();
        //root_widget_->installEventFilter(this);

        return true;
    }

    QWidget* SkinInterface::GetRootWidget() {
        return /*root_widget_*/nullptr;
    }

    bool SkinInterface::eventFilter(QObject *watched, QEvent *event) {
        //if (watched == root_widget_) {
        //    if (event->type() == QEvent::Type::Close) {
        //        LOGI("Event: {}", (int) event->type());
        //        event->ignore();
        //        ((QWidget*)watched)->hide();
        //        return true;
        //    }
        //}
        return QObject::eventFilter(watched, event);
    }

}
