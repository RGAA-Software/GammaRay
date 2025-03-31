//
// Created by RGAA on 2024-04-11.
//

#ifndef WIDGET_EFFECT_H
#define WIDGET_EFFECT_H

#include <QTimer>
#include <QWidget>

#include <mutex>
#include "render_panel/gr_statistics.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/audio_filter.h"

namespace tc
{

    class EffectWidget : public QWidget {
    public:
        explicit EffectWidget(QWidget *parent = nullptr) : QWidget(parent) {
            auto timer = new QTimer(this);
            timer->setInterval(17);
            QObject::connect(timer, &QTimer::timeout, this, [this] {
                auto st = GrStatistics::Instance();
                this->OnDataComing(st->left_spectrum_, st->right_spectrum_);
                this->update();
            });
            timer->start();
        }

        virtual QWidget *AsWidget() {
            return this;
        }

        static void FallDownBars(std::vector<double> &bars, std::vector<double> &new_bars) {
            for (int i = 0; i < (int) new_bars.size(); i++) {
                auto new_bar_height = new_bars[i];
                auto old_bar_height = bars[i];

                double target_height = 0;
                double diff = new_bar_height - old_bar_height;
                target_height = old_bar_height + diff * 1.0f / 2;

                if (target_height < 0) {
                    target_height = 0;
                }

                bars[i] = target_height;
            }
        }

        void OnDataComing(const std::vector<double> &left, const std::vector<double> &right) {
            counts_++;
            if (counts_ % 3 != 0) {
                return;
            }

            std::lock_guard<std::mutex> guard(data_mtx_);
            if (left_new_bars_.size() != left.size()) {
                left_new_bars_.resize(left.size());
            }
            if (right_new_bars_.size() != right.size()) {
                right_new_bars_.resize(right.size());
            }

            memcpy(left_new_bars_.data(), left.data(), left.size() * sizeof(double));
            memcpy(right_new_bars_.data(), right.data(), right.size() * sizeof(double));

            MonsterCatFilter::FilterBars(left_new_bars_);
            MonsterCatFilter::FilterBars(right_new_bars_);
        }

    protected:

        std::mutex data_mtx_;
        std::vector<double> left_bars;
        std::vector<double> left_new_bars_;
        std::vector<double> right_bars;
        std::vector<double> right_new_bars_;
        std::vector<double> origin_left_bars_;
        std::vector<double> origin_right_bars_;
        long counts_ = 0;
    };

    typedef std::shared_ptr<EffectWidget> EffectWidgetPtr;

}

#endif // WIDGET_EFFECT_H
