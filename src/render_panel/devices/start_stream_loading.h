//
// Created by RGAA on 21/05/2025.
//

#ifndef GAMMARAY_START_STREAM_LOADING_H
#define GAMMARAY_START_STREAM_LOADING_H

#include <memory>
#include <QDialog>

namespace tc
{

    class GrContext;
    class StreamItem;
    class Win10CircleLoadingWidget;
    class Win10HorizontalLoadingWidget;

    class StartStreamLoading : public QDialog {
    public:
        StartStreamLoading(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<StreamItem>& item);
        void resizeEvent(QResizeEvent *event) override;
        void paintEvent(QPaintEvent *event) override;

    private:
        std::shared_ptr<StreamItem> stream_item_ = nullptr;
        Win10HorizontalLoadingWidget* h_loading_widget_ = nullptr;

    };

}

#endif //GAMMARAY_START_STREAM_LOADING_H
