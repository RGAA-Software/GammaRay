#pragma once

#include "tc_common_new/data.h"

#include <memory>
#include <functional>

typedef std::function<void()> OnPrepareCallback;
typedef std::function<void(int samples, int channels, int bits)> OnFormatCallback;
typedef std::function<void(const tc::DataPtr& data)> OnDataCallback;
typedef std::function<void(const tc::DataPtr& left, const tc::DataPtr& right)> OnSplitDataCallback;
typedef std::function<void()> OnPauseCallback;
typedef std::function<void()> OnStopCallback;

namespace tc
{

	class IAudioCapture {
	public:

		virtual int Start() = 0;
		virtual int Pause() = 0;
		virtual int Stop() = 0;

		void RegisterPrepareCallback(const OnPrepareCallback& cbk) {
            prepare_callback_ = cbk;
		}

		void RegisterFormatCallback(const OnFormatCallback& cbk) {
            format_callback_ = cbk;
		}

		void RegisterDataCallback(const OnDataCallback& cbk) {
            data_callback_ = cbk;
		}

		void RegisterSplitDataCallback(const OnSplitDataCallback& cbk) {
            split_data_callback_ = cbk;
		}

		void RegisterPauseCallback(const OnPauseCallback& cbk) {
            pause_callback_ = cbk;
		}

		void RegisterStopCallback(const OnStopCallback& cbk) {
            stop_callback_ = cbk;
		}

	protected:
		OnPrepareCallback prepare_callback_{nullptr };
		OnFormatCallback format_callback_{nullptr };
		OnDataCallback data_callback_{nullptr };
		OnSplitDataCallback split_data_callback_{nullptr };
		OnPauseCallback pause_callback_{nullptr };
		OnStopCallback stop_callback_{nullptr };
        std::string device_id_;
	};

	typedef std::shared_ptr<IAudioCapture> AudioCapturePtr;
}