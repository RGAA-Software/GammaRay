#pragma once

#include "audio_capture.h"

#include <Windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <ctime>
#include <mmeapi.h>
#include <iostream>
#include <mutex>
#include <condition_variable>

namespace tc
{

	class WASAPIAudioCapture : public IAudioCapture {
	public:
		static AudioCapturePtr Make(const std::string& device_id);

		int Start() override;
		int Pause() override;
		int Stop() override;
	
	private:
		bool exit_ = false;

	};

}