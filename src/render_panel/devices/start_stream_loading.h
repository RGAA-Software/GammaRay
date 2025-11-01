//
// Created by RGAA on 21/05/2025.
//

#ifndef GAMMARAY_START_STREAM_LOADING_H
#define GAMMARAY_START_STREAM_LOADING_H

#include <memory>
#include <QDialog>

namespace spvr
{
    class SpvrStream;
}

namespace tc
{

    class GrContext;
    class Win10CircleLoadingWidget;
    class Win10HorizontalLoadingWidget;

    class StartStreamLoading : public QDialog {
    public:
        StartStreamLoading(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<spvr::SpvrStream>& item, const std::string& network_type);
        void resizeEvent(QResizeEvent *event) override;
        void paintEvent(QPaintEvent *event) override;

    private:
        std::shared_ptr<spvr::SpvrStream> stream_item_ = nullptr;
        Win10HorizontalLoadingWidget* h_loading_widget_ = nullptr;

    };

}

#endif //GAMMARAY_START_STREAM_LOADING_H
