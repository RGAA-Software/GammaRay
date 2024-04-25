//
// Created by RGAA on 2024-04-11.
//

#ifndef WIDGET_EFFECT_H
#define WIDGET_EFFECT_H

#include <QTimer>
#include <QWidget>

#include <mutex>
#include "util/audio_filter.h"
#include "gr_statistics.h"
#include "tc_common_new/log.h"

namespace tc
{

    constexpr auto kMaxBars = 480;

    class EffectWidget : public QWidget {
        //Q_OBJECT
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

                float target_height = 0;

                float diff = new_bar_height - old_bar_height;
                target_height = old_bar_height + diff * 1.0f / 2;

                if (target_height < 0) {
                    target_height = 0;
                }

                bars[i] = target_height;
            }
        }

        void OnDataComing(const std::vector<double> &left, const std::vector<double> &right) {
            counts++;
            if (counts % 3 != 0) {
                return;
            }

            std::lock_guard<std::mutex> guard(data_mtx);
            if (left_new_bars.size() != left.size()) {
                left_new_bars.resize(left.size());
            }
            if (right_new_bars.size() != right.size()) {
                right_new_bars.resize(right.size());
            }

            memcpy(left_new_bars.data(), left.data(), left.size() * sizeof(double));
            memcpy(right_new_bars.data(), right.data(), right.size() * sizeof(double));

            MonsterCatFilter::FilterBars(left_new_bars);
            MonsterCatFilter::FilterBars(right_new_bars);

        }

    protected:
        std::mutex data_mtx;

        std::vector<double> left_bars;
        std::vector<double> left_new_bars;

        std::vector<double> right_bars;
        std::vector<double> right_new_bars;

        std::vector<double> origin_left_bars;
        std::vector<double> origin_right_bars;

        long counts = 0;
    };

    typedef std::shared_ptr<EffectWidget> EffectWidgetPtr;

}

#endif // WIDGET_EFFECT_H
