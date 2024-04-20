//
// Created by RGAA on 2024/2/18.
//

#include "data_channel_observer_impl.h"

namespace tc
{

    std::shared_ptr<DataChannelObserverImpl> DataChannelObserverImpl::Make() {
        return std::make_shared<DataChannelObserverImpl>();
    }

    DataChannelObserverImpl::DataChannelObserverImpl() {

    }

    void DataChannelObserverImpl::OnStateChange() {

    }

    void DataChannelObserverImpl::OnMessage(const webrtc::DataBuffer &buffer) {

    }

    void DataChannelObserverImpl::OnBufferedAmountChange(uint64_t sent_data_size) {
        DataChannelObserver::OnBufferedAmountChange(sent_data_size);
    }

//    bool DataChannelObserverImpl::IsOkToCallOnTheNetworkThread() {
//        return DataChannelObserver::IsOkToCallOnTheNetworkThread();
//    }

}