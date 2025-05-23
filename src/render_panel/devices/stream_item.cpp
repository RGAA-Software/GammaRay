//
// Created by RGAA on 2023/8/14.
//

#include "stream_item.h"

namespace tc
{

    bool StreamItem::IsValid() const {
        return !stream_id_.empty();
    }

    bool StreamItem::IsRelay() const {
        return network_type_ == "relay" || !remote_device_id_.empty();
    }

}
