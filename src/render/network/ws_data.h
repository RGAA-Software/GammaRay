//
// Created by RGAA on 27/05/2025.
//

#ifndef GAMMARAY_WS_DATA_H
#define GAMMARAY_WS_DATA_H

namespace tc
{
    class WsData {
    public:
        std::map<std::string, std::any> vars_;
    };
    using WsDataPtr = std::shared_ptr<WsData>;
}

#endif //GAMMARAY_WS_DATA_H
