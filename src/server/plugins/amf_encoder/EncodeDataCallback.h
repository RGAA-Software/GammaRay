#pragma once

#include "tc_common_new/data.h"
#include <functional>

typedef std::function<void(std::shared_ptr<tc::Data> data, uint64_t frame_idx, bool key)> EncodeDataCallback;
typedef std::function<void(bool success)> InitEncoderCallback;
