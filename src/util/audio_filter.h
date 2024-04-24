#ifndef FILTER_H
#define FILTER_H

#include <vector>
#include <cmath>
#include <cstdint>

namespace tc
{

    class MonsterCatFilter {
    public:
        static void FilterBars(std::vector<double> &bars) {

            if (bars.empty()) {
                return;
            }
            unsigned int bars_length = bars.size();
            std::vector<double> m_monstercat_smoothing_weights;

            //PrintLog("mc bar size : %d, size : %d", bars_length, bars.size());

            // re-compute weights if needed, this is a performance tweak to computer the
            // smoothing considerably faster
            if (m_monstercat_smoothing_weights.size() != bars.size()) {
                m_monstercat_smoothing_weights.resize(bars.size());
                for (auto i = 0u; i < bars.size(); ++i) {
                    m_monstercat_smoothing_weights[i] = std::pow(1.5f, i);
                }
            }

            auto k_minimum_bar_height = 2.25f;
            // apply monstercat sytle smoothing
            // Since this type of smoothing smoothes the bars around it, doesn't make
            // sense to smooth the first value so skip it.
            int count = 0;
            for (auto i = 1l; i < bars_length; ++i) {
                auto outer_index = static_cast<size_t>(i);
                if (bars[outer_index] < k_minimum_bar_height) {
                    bars[outer_index] = k_minimum_bar_height;
                    count++;
                } else {
                    for (int64_t j = 0; j < bars_length; ++j) {
                        if (i != j) {
                            auto index = static_cast<size_t>(j);
                            const auto weighted_value =
                                    bars[outer_index] /
                                    m_monstercat_smoothing_weights[static_cast<size_t>(
                                            std::abs(i - j))];
                            // Note: do not use max here, since it's actually slower.
                            // Separating the assignment from the comparison avoids an
                            // unneeded assignment when bars[index] is the largest which
                            // is often
                            if (bars[index] < weighted_value) {
                                bars[index] = weighted_value;
                            }
                            count++;
                        }
                    }
                }
            }
        }
    };

}

#endif // FILTER_H
