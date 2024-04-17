//
// Created by RGAA on 2023-12-25.
//

#ifndef TC_APPLICATION_MESSAGEMAKER_H
#define TC_APPLICATION_MESSAGEMAKER_H

#include "message_maker.h"
#include <string>
#include <memory>

#include "tc_message.pb.h"

namespace tc
{

    class Data;

    class NetMessageMaker {
    public:

        static std::string MakeHelloMsg();
        static std::string MakeAckMsg();
        static std::string MakeHeartBeatMsg();
        static std::string MakeVideoFrameMsg(const tc::VideoType& vt,
                                             const std::shared_ptr<Data>& data,
                                             uint64_t frame_index,
                                             int frame_width,
                                             int frame_height, bool key);
        static std::string MakeAudioFrameMsg(const std::shared_ptr<Data>& data,
                                             int samples, int channels, int bits, int frame_size);

    };

}
#endif //TC_APPLICATION_MESSAGEMAKER_H
