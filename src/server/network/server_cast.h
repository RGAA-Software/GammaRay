//
// Created by hy on 2024/3/15.
//

#ifndef TC_APPLICATION_SERVER_CAST_H
#define TC_APPLICATION_SERVER_CAST_H

#include <asio2/udp/udp_cast.hpp>

namespace tc
{

    class Context;

    class ServerCast {
    public:

        static std::shared_ptr<ServerCast> Make(std::shared_ptr<Context>& app);

        explicit ServerCast(const std::shared_ptr<Context>& app);
        void Start();
        void CastMessage(const std::string& msg);
        void Stop();

    private:

        std::shared_ptr<Context> app_ = nullptr;
        std::shared_ptr<asio2::udp_cast> udp_cast_ = nullptr;
    };

}

#endif //TC_APPLICATION_SERVER_CAST_H
