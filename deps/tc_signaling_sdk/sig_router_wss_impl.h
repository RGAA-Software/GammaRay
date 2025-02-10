#pragma once

#include "sig_router_interface.h"
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <asio2/asio2.hpp>
#include "sig_message.h"
#include "json/json.hpp"

namespace tc {

class SigRouterWssImpl : public SigRouterInterface {
public:
	explicit SigRouterWssImpl(const std::shared_ptr<RtcContext>& ctx);
	void Init(const SignalingParam& params) override;
	void Start() override;
	void Exit() override;
	void SendSigMessage(const std::string& sig_name, const std::string& token, const std::string& msg) override;
	void SendHeartBeat() override;
	bool IsAlive() override;

private:
	std::shared_ptr<asio2::wss_client> client_ = nullptr;
	uint64_t heart_beat_idx_ = 0;
};

}