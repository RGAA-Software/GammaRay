#pragma once
#include <memory>
#include <map>

namespace tc { 
	class GrContext;
	class MessageListener;
	class ConnectedInfoSlidingWindow;
	
	class GrConnectedManager {
	public:
		GrConnectedManager(const std::shared_ptr<GrContext>& ctx);
		void RegisterMessageListener();
		void TestShowPanel();
	private:
		void AdjustPanelPosition();
		void HideAllPanels();
		void ShowAllPanels();
	private:
		std::shared_ptr<GrContext> gr_ctx_ = nullptr;
		std::shared_ptr<MessageListener> msg_listener_ = nullptr;

		std::map<int, ConnectedInfoSlidingWindow*> connected_info_panel_group_;

	};
}