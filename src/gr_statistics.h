//
// Created by RGAA on 2024-04-20.
//

#ifndef GAMMARAY_GR_STATISTICS_H
#define GAMMARAY_GR_STATISTICS_H

#include <memory>
#include <vector>
#include <cstdint>

namespace tc
{

    class GrContext;
    class MessageListener;

    class GrStatistics {
    public:

        static GrStatistics* Instance() {
            static GrStatistics instance;
            return &instance;
        }

        void SetContext(const std::shared_ptr<GrContext>& ctx) { context_ = ctx;}
        void RegisterEventListeners();

    public:

        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;

        std::vector<uint32_t> video_frame_gaps_;
        std::vector<uint32_t> encode_durations_;

    };

}

#endif //GAMMARAY_GR_STATISTICS_H
