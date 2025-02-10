////
//// Created by RGAA on 2024/5/28.
////
//
//#include "sig_request.h"
//#include "asio2/http/https_client.hpp"
//#include "base_lib/rlog.h"
//#include "base_lib/hardware.h"
//#include "sig_apis.h"
//#include "NlohmannJson/json.hpp"
//#include "sig_message.h"
//
//using namespace nlohmann;
//
//namespace tc
//{
//
//std::shared_ptr<SigRequest> SigRequest::Make(const std::string& sig_host, int sig_port, const std::string& appkey) {
//    auto sq = std::make_shared<SigRequest>();
//    sq->sig_host_ = sig_host;
//    sq->sig_port_ = sig_port;
//    sq->appkey_ = appkey;
//    return sq;
//}
//
//SigRespStr<bool> SigRequest::RequestClientId() {
//#ifdef WIN32
//	auto hardware = Hardware::Instance();
//	hardware->Detect();
//	hardware->Dump();
//	std::string hid = hardware->GetHardwareId();
//	auto url = fmt::format("https://{}:{}{}?platform={}&hardware={}&appkey={}",
//		sig_host_, sig_port_, kApiRequestClientId,
//		Hardware::GetPlatform(), hid, appkey_);
//	RLogI("RequestClientId url: {}", url);
//
//	SigRespStr<bool> result{ false, "" };
//	auto resp = SigRequest::Get(url);
//	if (!resp.result_) {
//		RLogE("RequestClientId failed: {}", resp.value_);
//		return result;
//	}
//
//	try {
//		auto obj = json::parse(resp.value_);
//		auto code = obj["code"].get<int>();
//		if (code != 200) {
//			RLogE("Error message: {}", resp.value_);
//			return result;
//		}
//		result.result_ = true;
//		result.value_ = obj["value"]["id"].get<std::string>();
//		return result;
//	}
//	catch (const std::exception& e) {
//		RLogE("Parse failed: {}, e: {}", resp.value_, e.what());
//		return result;
//    }
//#else
//    SigRespStr<bool> result{false, ""};
//    return result;
//#endif
//}
//
//SigResp<bool, std::shared_ptr<Room>> SigRequest::RequestRoomInfo(const std::string& room_id) {
//	auto url = fmt::format("https://{}:{}{}?appkey={}&room_id={}",
//		sig_host_, sig_port_, kApiRequestRoomStatus,
//		appkey_, room_id);
//	//RLogI("RequestRoomInfo url: {}", url);
//
//	SigResp<bool, std::shared_ptr<Room>> result{ false, nullptr };
//	auto resp = SigRequest::Get(url);
//	if (!resp.result_) {
//		return result;
//	}
//
//	try {
//		auto obj = json::parse(resp.value_);
//		auto code = obj["code"].get<int>();
//		if (code != 200) {
//			RLogE("Error message: {}", resp.value_);
//			return result;
//		}
//		result.result_ = true;
//		auto room_obj = obj["value"]["room"];
//		if (room_obj.is_null()) {
//			RLogE("Not have room field");
//			return result;
//		}
//
//		auto room = std::make_shared<Room>();
//		room->id_ = room_obj[kKeyId].get<std::string>();
//		room->name_ = room_obj[kKeyName].get<std::string>();
//		auto clients_obj = room_obj[kKeyClients];
//		if (clients_obj.is_array()) {
//			for (const auto& client_obj : clients_obj) {
//				auto client = std::make_shared<Client>();
//				client->id_ = client_obj[kKeyId].get<std::string>();
//				client->name_ = client_obj[kKeyName].get<std::string>();
//				client->role_ = client_obj[kKeyRole].get<std::string>();
//				client->platform_ = client_obj[kKeyPlatform].get<std::string>();
//				client->room_id_ = client_obj[kKeyRoomId].get<std::string>();
//				client->update_timestamp_ = client_obj[kKeyUpdateTimestamp].get<int64_t>();
//				room->clients_.push_back(client);
//			}
//		}
//		result.value_ = room;
//		return result;
//	}
//	catch (const std::exception& e) {
//		RLogE("Parse failed: {}, e: {}", resp.value_, e.what());
//		return result;
//	}
//}
//
//SigRespStr<bool> SigRequest::Get(std::string_view url) {
//	asio::ssl::context ctx{ asio::ssl::context::tlsv13 };
//	auto resp = asio2::https_client::execute(ctx, url, std::chrono::milliseconds(3000));
//	if (asio2::get_last_error()) {
//		RLogI("error: {}", asio2::last_error_msg());
//		return SigRespStr<bool>::Make(false, "");
//	}
//	return SigRespStr<bool>::Make(true, resp.body());
//}
//
//}
