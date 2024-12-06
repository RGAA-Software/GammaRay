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
        return "Obj Detector Plugin";
    }

    std::string ObjDetectorPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t ObjDetectorPlugin::GetVersionCode() {
        return 110;
    }

    void ObjDetectorPlugin::On1Second() {
        GrPluginInterface::On1Second();

    }
    
    bool ObjDetectorPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);
        plugin_type_ = GrPluginType::kStream;

        root_widget_->show();
        {
            auto root_layout = new QVBoxLayout();
            label_ = new QLabel(root_widget_);
            label_->setFixedSize(root_widget_->size());
            root_layout->addWidget(label_);
            root_widget_->setLayout(root_layout);
        }

        return true;
    }

    void ObjDetectorPlugin::OnRawVideoFrameRgba(const std::shared_ptr<Image>& image) {
        QMetaObject::invokeMethod(this, [=]() {
            QImage img((uint8_t*)image->GetData()->DataAddr(), image->width, image->height, QImage::Format_RGBA8888);
            QPixmap pixmap = QPixmap::fromImage(img);
            pixmap = pixmap.scaled(root_widget_->size().width(), root_widget_->size().height());
            label_->setPixmap(pixmap);
            label_->repaint();
        });
    }

    void ObjDetectorPlugin::OnRawVideoFrameYuv(const std::shared_ptr<Image>& image) {

    }

}
