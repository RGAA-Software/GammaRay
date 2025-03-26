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
private:
	OpenGLVideoWidget* video_widget_ = nullptr;

	std::shared_ptr<ClientContext> ctx_ = nullptr;
	std::shared_ptr<ThunderSdk> sdk_ = nullptr;
	ThunderSdkParams params_;
};

}