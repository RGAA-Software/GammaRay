#include "media_recorder.h"
#include "tc_common_new/log.h"
#include "tc_common_new/frame_common.h"
#include "tc_common_new/time_util.h"
#include "plugins/ct_plugin_events.h"
#include "media_record_plugin.h"
#include <QApplication>
#include <qstandardpaths.h>
#include <qdir.h>

namespace tc {

static std::string GetFFmpegError(const int& index) {
	char buf[256] = { 0 };
	av_strerror(index, buf, sizeof(buf));
	return std::string(buf);
}

std::shared_ptr<MediaRecorder> MediaRecorder::Make(MediaRecordPluginClient* plugin) {
	return std::make_shared<MediaRecorder>(plugin);
}

MediaRecorder::MediaRecorder(MediaRecordPluginClient* plugin) : plugin_(plugin) {
	
}

MediaRecorder::~MediaRecorder() {
	
}

void MediaRecorder::SetFilePath(std::string name) {

}

bool MediaRecorder::InitFFmpeg() {
	
	std::string record_path;
	if (plugin_) {
		record_path = plugin_->GetScreenRecordingPath();
	}
	if (record_path.empty()) {
		record_path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation).toStdString();
	}

	QDir qdir{QString::fromStdString(record_path)};
    if (!qdir.exists()) {
        qdir.mkpath(".");
    }
	if (!qdir.exists()) {
		record_path = qApp->applicationDirPath().toStdString();
	}

	file_name_ = std::format("{}/GammaRay_screen_record_{}_{}.mp4", record_path, index_, TimeUtil::FormatTimestamp2(TimeUtil::GetCurrentTimestamp()));

	//打开音视频输出封装上下文
	int result = avformat_alloc_output_context2(&format_ctx_, nullptr, nullptr, file_name_.c_str());
	if (result < 0) {
		std::cout << "avformat_alloc_output_context2 - faild:" << GetFFmpegError(result).c_str() << std::endl;
		LOGE("MediaRecord avformat_alloc_output_context2 - faild %s", GetFFmpegError(result).c_str());
		return false;
	}
	//初始化视频流相关
	video_stream_ = avformat_new_stream(format_ctx_, NULL);
	if (!video_stream_) {
		std::cout << "avformat_new_stream - faild!" << std::endl;
		LOGE("MediaRecord avformat_new_stream(video) - faild");
		return false;
	}
	video_stream_->time_base = { 1, 90000 };
	
	// 获取编解码器参数结构体
	AVCodecParameters* video_codecpar = video_stream_->codecpar;
	if (tc::VideoType::kNetH264 == video_codec_) {
		video_codecpar->codec_id = AV_CODEC_ID_H264;
	}
	else {
		video_codecpar->codec_id = AV_CODEC_ID_H265;
	}
	video_codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	video_codecpar->width = width_;
	video_codecpar->height = height_;
	video_stream_index_ = video_stream_->index;

	//初始化音频流相关
	audio_stream_ = avformat_new_stream(format_ctx_, nullptr);
	if (!audio_stream_) {
		std::cout << "avformat_new_stream - faild!" << std::endl;
		LOGE("MediaRecord avformat_new_stream(audio) - faild");
		return false;
	}

	AVCodecParameters* audio_codecpar = audio_stream_->codecpar;
	audio_codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
	audio_codecpar->codec_id = AV_CODEC_ID_OPUS;
	audio_codecpar->sample_rate = 48000;
    audio_codecpar->channels = 2;
	audio_codecpar->channel_layout = AV_CH_LAYOUT_STEREO;
	audio_codecpar->format = AV_SAMPLE_FMT_S16;

	/*
	* 添加 OPUS 音频的 extradata(重要), 否则无法播放, 会提示：
	* invalid size 0 in stsd
	* [mov,mp4,m4a,3gp,3g2,mj2 @ 000001f97ecd3180] error reading header
	* .\GammaRay_meida_record_0_1748862598989.mp4: Invalid data found when processing input
	*/
	uint8_t opus_header[19] = {
		'O', 'p', 'u', 's', 'H', 'e', 'a', 'd', // Magic signature
		0x01,                                   // Version
		0x02,                                   // Channel count
		0x38, 0x01,                             // Pre-skip (312 in little-endian)
		0x80, 0xBB, 0x00, 0x00,                 // Sample rate (48000)
		0x00, 0x00,                             // Output gain
		0x00                                    // Channel mapping family
	};

	audio_codecpar->extradata = (uint8_t*)av_malloc(sizeof(opus_header) + AV_INPUT_BUFFER_PADDING_SIZE);
	if (!audio_codecpar->extradata) {
		LOGE("Failed to allocate extradata for OPUS");
		return false;
	}

	memcpy(audio_codecpar->extradata, opus_header, sizeof(opus_header));
	audio_codecpar->extradata_size = sizeof(opus_header);
	audio_stream_index_ = audio_stream_->index;

	result = avio_open(&format_ctx_->pb, file_name_.c_str(), AVIO_FLAG_WRITE);
	if (result < 0)
	{
		std::cout << "avio_open - faild:" << GetFFmpegError(result) << std::endl;
		LOGE("MediaRecord avio_open - faild %s", GetFFmpegError(result).c_str());
		return false;
	}
	result = avformat_write_header(format_ctx_, nullptr);
	if (result == AVSTREAM_INIT_IN_INIT_OUTPUT)
	{
		std::cout << "avformat_write_header - faild:" << GetFFmpegError(result) << std::endl;
		LOGE("MediaRecord avformat_write_header - faild %s", GetFFmpegError(result).c_str());
		return false;
	}
	video_frame_count_ = 0;
	audio_frame_count_ = 0;
	init_ok_ = true;
	return true;
}

void MediaRecorder::EndRecord() {

	if (plugin_) {
		if (file_name_.size() > 0) {
			std::shared_ptr<ClientPluginNotifyMsgEvent> event = std::make_shared<ClientPluginNotifyMsgEvent>();
			event->title_ = "A screen recording has ended";
			event->message_ = file_name_;
			plugin_->CallbackEvent(event);
		}
	}

	init_ok_ = false;
	if (format_ctx_) {
		av_write_trailer(format_ctx_);
		if (video_stream_index_ > -1) {
			av_freep(&format_ctx_->streams[video_stream_index_]);
		}

		if (audio_stream_index_ > -1) {
			if (audio_stream_ && audio_stream_->codecpar && audio_stream_->codecpar->extradata) {
				av_freep(&audio_stream_->codecpar->extradata);
			}
			av_freep(&format_ctx_->streams[audio_stream_index_]);
		}
		avio_close(format_ctx_->pb);
		av_free(format_ctx_);
		format_ctx_ = nullptr;
	}

	video_stream_index_ = -1;
	audio_stream_index_ = -1;
	width_ = -1;
	height_ = -1;
	video_frame_count_ = 0;
	audio_frame_count_ = 0;
	file_name_ = "";
}

void MediaRecorder::InitByVideoFrame(const VideoFrame& frame) {
	bool is_config_frame = false;
	const auto& d = frame.data();
	uint8_t* data = (uint8_t*)d.data();
	size_t size = d.size();
	if (frame.type() == tc::VideoType::kNetH264)
	{
		if (H264_TYPE(data[4]) == ENalType::H264_NAL_SPS)
			is_config_frame = true;
	}
	else if (frame.type() == tc::VideoType::kNetHevc)
	{
		if (H265_TYPE(data[4]) == ENalType::H265_NAL_VPS)
			is_config_frame = true;
	}
	else { // vp9, no support
		LOGW("video codec type: {}, has not support", (int)frame.type());
		return;
	}

	if (!is_config_frame) {
		return;
	}

	height_ = frame.frame_height();
	width_ = frame.frame_width();
	video_codec_ = frame.type();

	//去初始化
	if (!InitFFmpeg()) {
		LOGE("media record init error!!!");
		return;
	}

	start_time_ = std::chrono::system_clock::now();
	SaveVideoFrame(frame);
	return;
}

void MediaRecorder::RecvVideoFrame(const VideoFrame& frame) {
	// 尚未初始化
	if (!init_ok_) {
		LOGI("InitByVideoFrame 0");
		InitByVideoFrame(frame);
		return;
	}
	// 已经初始化了
	const int cur_width = frame.frame_width();
	const int cu_height = frame.frame_height();
	const auto cur_video_codec = frame.type();
	// 说明没什么变化
	if (cur_width == width_ && cu_height == height_ && cur_video_codec == video_codec_) {
		SaveVideoFrame(frame);
		return;
	}
	
	//如果视频属性发生变化，需要重新初始化
	EndRecord();
	InitByVideoFrame(frame);
}

void MediaRecorder::RecvAudioFrame(const AudioFrame& frame) {
	if (0 == video_frame_count_) {
		return;
	}
	AVPacket packet;
	av_init_packet(&packet);
	packet.stream_index = audio_stream_index_;
	packet.pts = 960 * audio_frame_count_;
	packet.dts = 960 * audio_frame_count_;
	auto& d = frame.data();
	packet.size = d.size();
	packet.data = (uint8_t*)d.data();
	if (format_ctx_) {
		av_interleaved_write_frame(format_ctx_, &packet);
	}
	av_packet_unref(&packet);
	++audio_frame_count_;
}

void MediaRecorder::SaveVideoFrame(const VideoFrame& frame) {
	auto& d = frame.data();
	if (0 == d.size()) {
		return;
	}

	auto pts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time_).count();
	AVPacket avpacket;
	av_init_packet(&avpacket);
	//录取第一个包,需要将pts设置为0
	static uint32_t real_start_pts = 0;
	if (0 == video_frame_count_) {
		real_start_pts = pts;
	}
	avpacket.pts = pts - real_start_pts;
	avpacket.pts = avpacket.pts * 90; //对应 视频流的时间基
	avpacket.dts = avpacket.pts;
	avpacket.size = d.size();
#if 0
	{
		//将I帧保存为文件 方便分析调试
		static int i = 0;
		std::string file_name = std::to_string(i) + ".h264";
		bool configFrame = false;
		const auto& d = frame.data();
		uint8_t * data = (uint8_t*)d.data();
		size_t size = d.size();
		if (frame.type() == tc::VideoType::kNetH264)
		{
			if (H264_TYPE(data[4]) == ENalType::H264_NAL_SPS)
				configFrame = true;
		}
		else
		{
			if (H265_TYPE(data[4]) == ENalType::H265_NAL_VPS)
				configFrame = true;
		}
		if (configFrame) {
			++i;
			FILE *pF = fopen(file_name.c_str(), "wb");
			fwrite(d.data(), 1, size, pF);
			fflush(pF);
		}
	}
#endif
	avpacket.data = (uint8_t*)d.data();
	avpacket.stream_index = video_stream_index_;
	if (format_ctx_) {
		av_interleaved_write_frame(format_ctx_, &avpacket);
	}
	av_packet_unref(&avpacket);
	++video_frame_count_;
}


void MediaRecorder::SetIndex(int idx) {
	index_ = idx;
}

}