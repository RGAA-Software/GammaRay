//
// Created by RGAA on 10/07/2025.
//

#ifndef GAMMARAY_STREAM_MESSAGES_H
#define GAMMARAY_STREAM_MESSAGES_H

#include <string>
#include <memory>

// send from panel -> remote render
// 1. direct: http request -> remote render
// 2. relay: http reqeust -> relay server -> remote render
namespace tc
{

    // type
    enum class GrStreamMessageType {
       kRestartRender,
       kLockScreen,
       kRestartDevice,
       kShutdownDevice,
    };

    // stream
    class StreamItem;

    // base
    class GrBaseStreamMessage {
    public:
        virtual std::string AsJson() = 0;
    public:
        GrStreamMessageType type_;
        std::shared_ptr<StreamItem> stream_item_ = nullptr;
    };

    //
    class GrSmRestartRender : public GrBaseStreamMessage {
    public:
        GrSmRestartRender() {
            type_ = GrStreamMessageType::kRestartRender;
        }

        std::string AsJson() override;
    };

    //
    class GrSmLockScreen : public GrBaseStreamMessage {
    public:
        GrSmLockScreen() {
            type_ = GrStreamMessageType::kLockScreen;
        }

        std::string AsJson() override;
    };

    //
    class GrSmRestartDevice : public GrBaseStreamMessage {
    public:
        GrSmRestartDevice() {
            type_ = GrStreamMessageType::kRestartDevice;
        }

        std::string AsJson() override;
    };

    //
    class GrSmShutdownDevice : public GrBaseStreamMessage {
    public:
        GrSmShutdownDevice() {
            type_ = GrStreamMessageType::kShutdownDevice;
        }

        std::string AsJson() override;
    };
}

#endif //GAMMARAY_STREAM_MESSAGES_H
