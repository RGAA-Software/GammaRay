//
// Created by RGAA on 2024/3/17.
//

#ifndef TC_APPLICATION_DX_ADDRESS_LOADER_H
#define TC_APPLICATION_DX_ADDRESS_LOADER_H

#include <memory>

namespace tc
{

    class AppSharedMessage;

    class DxAddressLoader {
    public:

        static std::shared_ptr<AppSharedMessage> LoadDxAddress();

    };

}

#endif //TC_APPLICATION_DX_ADDRESS_LOADER_H
