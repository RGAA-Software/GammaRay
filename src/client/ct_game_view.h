#pragma once

#include <QWidget>
#include "thunder_sdk.h"

namespace tc {

class OpenGLVideoWidget;
class ClientContext;
class ThunderSdk;

class GameView : public QWidget {
public:
	GameView(const std::shared_ptr<ClientContext>& ctx, std::shared_ptr<ThunderSdk>& sdk, const ThunderSdkParams& params, QWidget* parent);
	~GameView();

	void RefreshCapturedMonitorInfo(const SdkCaptureMonitorInfo& mon_info);
	void RefreshI420Image(const std::shared_ptr<RawImage>& image);

	void SendKeyEvent(quint32 vk, bool down);

	void SetActiveStatus(bool active) {
		active_ = active;
	}

	bool GetActiveStatus() {
		return active_;
	}

	void SetMonitorIndex(int index) {
		monitor_index_ = index;
	}

	int GetMonitorIndex() {
		return monitor_index_;
	}

	void SwitchToFullWindow();

	void CalculateAspectRatio();
private:
	OpenGLVideoWidget* video_widget_ = nullptr;

	std::shared_ptr<ClientContext> ctx_ = nullptr;
	std::shared_ptr<ThunderSdk> sdk_ = nullptr;
	ThunderSdkParams params_;

	int monitor_index_ = 0;
	std::string monitor_name_ = "";
	bool active_ = false;
};

}