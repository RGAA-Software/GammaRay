//
// Created RGAA on 15/11/2024.
//

#include "obj_detector_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "render/plugins/plugin_ids.h"

namespace tc
{

    std::string ObjDetectorPlugin::GetPluginId() {
        return kNetObjDetectorPluginId;
    }

    std::string ObjDetectorPlugin::GetPluginName() {
        return "Obj Detector";
    }

    std::string ObjDetectorPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t ObjDetectorPlugin::GetVersionCode() {
        return 110;
    }

    std::string ObjDetectorPlugin::GetPluginDescription() {
        return "Object detector";
    }

    void ObjDetectorPlugin::On1Second() {
        GrPluginInterface::On1Second();

    }
    
    bool ObjDetectorPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);
        plugin_type_ = GrPluginType::kStream;

        if (!IsPluginEnabled()) {
            return true;
        }
        root_widget_->hide();
        //root_widget_->show();
        return true;
    }

    void ObjDetectorPlugin::OnRawVideoFrameRgba(const std::string& name, const std::shared_ptr<Image>& image) {
        if (!IsPluginEnabled()) {
            return;
        }
        QMetaObject::invokeMethod(this, [=, this]() {
            QImage img((uint8_t*)image->GetData()->DataAddr(), image->width, image->height, QImage::Format_RGBA8888);
            QPixmap pixmap = QPixmap::fromImage(img);
            pixmap = pixmap.scaled(root_widget_->size().width(), root_widget_->size().height());
            if (!previewers_.contains(name)) {
                previewers_[name] = new QLabel();
                previewers_[name]->setWindowTitle(name.c_str());
                previewers_[name]->resize(960, 540);
                previewers_[name]->show();
            }
            previewers_[name]->setPixmap(pixmap);
            previewers_[name]->repaint();
        });
    }

    void ObjDetectorPlugin::OnRawVideoFrameYuv(const std::string& name, const std::shared_ptr<Image>& image) {

    }

}
