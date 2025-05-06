#pragma once

#include <QWidget>
#include "thunder_sdk.h"

namespace tc {

class OpenGLVideoWidget;
class ClientContext;
class ThunderSdk;
class FloatController;
class FloatControllerPanel;
class MessageListener;

class GameView : public QWidget {
public:
	GameView(const std::shared_ptr<ClientContext>& ctx, std::shared_ptr<ThunderSdk>& sdk, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent);
	~GameView();
	void resizeEvent(QResizeEvent* event) override;
	void RefreshCapturedMonitorInfo(const SdkCaptureMonitorInfo& mon_info);
	void RefreshImage(const std::shared_ptr<RawImage>& image);
	void RefreshI420Image(const std::shared_ptr<RawImage>& image);
	void RefreshI444Image(const std::shared_ptr<RawImage>& image);

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

	void SetMonitorName(const std::string mon_name);

	void SwitchToFullWindow();

	void CalculateAspectRatio();

	void SetMainView(bool main_view);

	bool IsMainView() {
		return main_view_;
	}
private:
	OpenGLVideoWidget* video_widget_ = nullptr;
	std::shared_ptr<ClientContext> ctx_ = nullptr;
	std::shared_ptr<ThunderSdk> sdk_ = nullptr;
    std::shared_ptr<ThunderSdkParams> params_;
	std::shared_ptr<MessageListener> msg_listener_ = nullptr;
	int monitor_index_ = 0;
	std::string monitor_name_ = "";
	bool active_ = false;
	bool main_view_ = false;

	FloatController* float_controller_ = nullptr;
	FloatControllerPanel* controller_panel_ = nullptr;

private:
	void InitFloatController();
	void RegisterControllerPanelListeners();
};

}