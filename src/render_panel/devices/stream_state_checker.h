//
// Created by RGAA on 24/05/2025.
//

#ifndef GAMMARAY_STREAM_STATE_CHECKER_H
#define GAMMARAY_STREAM_STATE_CHECKER_H

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace tc
{

    class GrContext;
    class StreamItem;
    class MessageListener;

    using OnStreamStateCheckedCallback = std::function<void(std::vector<std::shared_ptr<StreamItem>>)>;

    class StreamStateChecker {
    public:
        explicit StreamStateChecker(const std::shared_ptr<GrContext>& ctx);
        void Start();
        void Exit();
        void SetOnCheckedCallback(OnStreamStateCheckedCallback&&);
        void UpdateCurrentStreamItems(std::vector<std::shared_ptr<StreamItem>> items);
    private:
        void CheckState(const std::vector<std::shared_ptr<StreamItem>>& items);

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        OnStreamStateCheckedCallback on_checked_cbk_;
    };

}

#endif //GAMMARAY_STREAM_STATE_CHECKER_H
