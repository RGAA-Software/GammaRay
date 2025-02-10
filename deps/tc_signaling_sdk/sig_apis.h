//
// Created by RGAA on 2024/5/28.
//

#ifndef TC_WEBRTC_SIG_APIS_H
#define TC_WEBRTC_SIG_APIS_H

#include <string>

const std::string kApiPing = "/ping";
const std::string kApiSignaling = "/signaling";
const std::string kApiRequestClientId = "/request/client/id";
const std::string kApiCheckClientOnline = "/check/client/online";
const std::string kApiOnlineClients = "/online/clients";
const std::string kApiOnlineRooms = "/online/rooms";
const std::string kApiRequestRoomStatus = "/request/room/status";
const std::string kApiServerStatus = "/server/status";

#endif //TC_WEBRTC_SIG_APIS_H
