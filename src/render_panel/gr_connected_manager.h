#pragma once
#include <memory>
#include <map>
#include <atomic>
#include <qobject.h>
#include <qabstractnativeeventfilter.h>

namespace tc { 
	class GrContext;
	class MessageListener;
	class ConnectedInfoSlidingWindow;
	
	class GrConnectedManager : public QObject, public QAbstractNativeEventFilter {
	public:
		GrConnectedManager(const std::shared_ptr<GrContext>& ctx);
		void RegisterMessageListener();
		void TestShowPanel();
		bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
	private:
		void CreatePanel();
		void AdjustPanelPosition();
		void InitPanel();
		void HideAllPanels();
		void ShowAllPanels();
	private:
		std::shared_ptr<GrContext> gr_ctx_ = nullptr;
		std::shared_ptr<MessageListener> msg_listener_ = nullptr;

		std::map<int, ConnectedInfoSlidingWindow*> connected_info_panel_group_;

		std::atomic<int> client_connected_count_{0};
	};
}