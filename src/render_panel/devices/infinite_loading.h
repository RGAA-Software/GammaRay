//
// Created by RGAA on 21/05/2025.
//

#ifndef GAMMARAY_INIFINITE_LOADING_H
#define GAMMARAY_INIFINITE_LOADING_H

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

    class InfiniteLoading : public QDialog {
    public:
        InfiniteLoading(const std::shared_ptr<GrContext>& ctx, const QString& msg);
        void resizeEvent(QResizeEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
        void Close();

    private:
        std::shared_ptr<spvr::SpvrStream> stream_item_ = nullptr;
        Win10HorizontalLoadingWidget* h_loading_widget_ = nullptr;

    };

}

#endif //GAMMARAY_START_STREAM_LOADING_H
