//
// Created by RGAA on 2024/2/18.
//

#ifndef TEST_WEBRTC_DATA_CHANNEL_OBSERVER_IMPL_H
#define TEST_WEBRTC_DATA_CHANNEL_OBSERVER_IMPL_H

#include "webrtc_helper.h"

namespace tc
{

    class DataChannelObserverImpl :  public webrtc::DataChannelObserver {
    public:

        static std::shared_ptr<DataChannelObserverImpl> Make();

        DataChannelObserverImpl();

        // overrides
        void OnStateChange() override;

        void OnMessage(const webrtc::DataBuffer &buffer) override;

        void OnBufferedAmountChange(uint64_t sent_data_size) override;

//        bool IsOkToCallOnTheNetworkThread() override;
    };

}

#endif //TEST_WEBRTC_DATA_CHANNEL_OBSERVER_IMPL_H
