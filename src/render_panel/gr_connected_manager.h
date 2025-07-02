#pragma once
#include <memory>
#include <map>

namespace tc { 
	class GrContext;
	class MessageListener;
	class ConnectedInfoTag;
	class ConnectedInfoPanel;

	class ConnectedPair {
	public:
		ConnectedInfoTag* tag_ = nullptr;
		ConnectedInfoPanel* panel_ = nullptr;
	};

	class GrConnectedManager {
	public:
		GrConnectedManager(const std::shared_ptr<GrContext>& ctx);
		void RegisterMessageListener();
		void TestShowPanel();
	private:
		void AdjustPanelPosition();
	private:
		std::shared_ptr<GrContext> gr_ctx_ = nullptr;
		std::shared_ptr<MessageListener> msg_listener_ = nullptr;

		//默认最多显示两个在线的连接
		std::map<int, ConnectedPair> connected_panel_group_;

	};
}