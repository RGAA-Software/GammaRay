////
//// Created by RGAA on 2024/5/28.
////
//
//#ifndef TC_WEBRTC_SIG_REQUEST_H
//#define TC_WEBRTC_SIG_REQUEST_H
//
//#include <map>
//#include <string>
//#include <memory>
//
//#include "sig_message.h"
//
//using RequestParams = std::map<std::string, std::string>;
//
//namespace tc {
//
//template<typename T>
//class SigRespStr {
//public:
//    static SigRespStr Make(const T& result, const std::string& value) {
//        SigRespStr resp;
//        resp.result_ = result;
//        resp.value_ = value;
//        return resp;
//    }
//
//public:
//    T result_;
//    std::string value_;
//};
//
//template<typename T, typename V>
//class SigResp {
//public:
//    static SigResp Make(const T& t, const V& v) {
//        SigResp resp;
//        resp.result_ = t;
//        resp.value_ = v;
//        return resp;
//    }
//
//public:
//    T result_;
//    V value_;
//};
//
//class SigRequest {
//public:
//    static std::shared_ptr<SigRequest> Make(const std::string& sig_host, int sig_port, const std::string& appkey);
//
//    SigRespStr<bool> RequestClientId();
//    SigResp<bool, std::shared_ptr<Room>> RequestRoomInfo(const std::string& room_id);
//    SigRespStr<bool> Get(std::string_view url);
//
//private:
//    std::string sig_host_;
//    int sig_port_;
//    std::string appkey_;
//};
//
//}
//
//#endif //TC_WEBRTC_SIG_REQUEST_H
