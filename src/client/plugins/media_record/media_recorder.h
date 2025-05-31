#pragma once

#include <string>
#include <memory>
#include <chrono>

extern "C" {
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/fifo.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h" //重采样
#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/frame.h"
}

#include "tc_message.pb.h"

namespace tc { 

class MediaRecordPluginClient;

class MediaRecorder { 
public:
	static std::shared_ptr<MediaRecorder> Make(MediaRecordPluginClient* plugin);

    explicit MediaRecorder(MediaRecordPluginClient* plugin);
    ~MediaRecorder();

	void SetFilePath(std::string name);

	bool InitFFmpeg();

	void InitByVideoFrame(const VideoFrame& frame);
	
	void EndRecord();

	void RecvVideoFrame(const VideoFrame& frame);
	void SaveVideoFrame(const VideoFrame& frame);


	void RecvAudioFrame(const AudioFrame& frame);


	void SetIndex(int idx);

	AVFormatContext* format_ctx_ = nullptr;
	//AVCodecContext* mAudioCodecContext = nullptr;
	AVStream* video_stream_ = nullptr;
	AVStream* audio_stream_ = nullptr;
	AVPixelFormat pix_fmt_ = AV_PIX_FMT_NONE;
	std::string file_name_;
	int width_ = 3840;
	int height_ = 2160;
	int video_stream_index_ = -1;
	int audio_stream_index = -1;

	int video_frame_count_ = 0;

	std::chrono::system_clock::time_point start_time_;

	int last_width_ = -1;
	int last_height_ = -1;

	MediaRecordPluginClient* plugin_ = nullptr;

	tc::VideoType video_codec_ =  tc::VideoType::kNetH264;

	std::atomic<bool> init_ok_ = false;

	int index_ = 0;

	
};

}