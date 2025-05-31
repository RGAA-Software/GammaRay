#include "media_recorder.h"
#include "tc_common_new/log.h"
#include "tc_common_new/frame_common.h"
#include "tc_common_new/time_util.h"
#include "media_record_plugin.h"
#include <QApplication>

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
	
	file_name_ = std::format("{}/GammaRay_meida_record_{}_{}.mp4", qApp->applicationDirPath().toStdString(), index_, TimeUtil::GetCurrentTimestamp());

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
		LOGE("MediaRecord avformat_new_stream - faild");
		return false;
	}
	video_stream_->time_base = { 1, 90000 };
	/*if (mVideoCodecType == cloudapp::Frame::Type::Frame_Type_kH264)
		video_stream_->codec->codec_id = AV_CODEC_ID_H264;
	else
		video_stream_->codec->codec_id = AV_CODEC_ID_H265;*/

		// 获取编解码器参数结构体
	AVCodecParameters* codecpar = video_stream_->codecpar;
	if (tc::VideoType::kNetH264 == video_codec_) {
		codecpar->codec_id = AV_CODEC_ID_H264;
	}
	else {
		codecpar->codec_id = AV_CODEC_ID_H265;
	}

	codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	//video_stream_->codec->max_b_frames = 0;
	codecpar->width = width_;
	codecpar->height = height_;
	//video_stream_->codec->flags = AV_CODEC_FLAG_LOW_DELAY;
	//video_stream_->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	video_stream_index_ = video_stream_->index;

#if 0
	//初始化音频流相关
	mOutAudioStream = avformat_new_stream(format_ctx_, nullptr);
	if (!mOutAudioStream)
	{
		std::cout << "avformat_new_stream - faild!" << std::endl;
		LOGE("MediaRecord avformat_new_stream - faild");
		return false;
	}
	mOutAudioStream->codecpar->codec_tag = 0;
	result = avcodec_parameters_from_context(mOutAudioStream->codecpar, mAudioCodecContext);
	if (result < 0)
	{
		std::cout << "avcodec_parameters_from_context - faild:" << GetFFmpegError(result) << std::endl;
		LOGE("MediaRecord avcodec_parameters_from_context - faild %s", GetFFmpegError(result).c_str());
		return false;
	}
	av_dump_format(format_ctx_, 0, mFileName.c_str(), 1);
#endif
	result = avio_open(&format_ctx_->pb, file_name_.c_str(), AVIO_FLAG_WRITE);
	if (result < 0)
	{
		std::cout << "avio_open - faild:" << GetFFmpegError(result) << std::endl;
		LOGE("MediaRecord avio_open - faild %s", GetFFmpegError(result).c_str());
		return false;
	}
	//audio_stream_index = mOutAudioStream->index;
	result = avformat_write_header(format_ctx_, nullptr);
	if (result == AVSTREAM_INIT_IN_INIT_OUTPUT)
	{
		std::cout << "avformat_write_header - faild:" << GetFFmpegError(result) << std::endl;
		LOGE("MediaRecord avformat_write_header - faild %s", GetFFmpegError(result).c_str());
		return false;
	}
	/*if (!ReInitMixAudioFifo()) {
		return false;
	}
	mAudioPacketNumOfTimes = 0;
	video_frame_count_ = 0;
	mIsCanStart = false;
	mIsInitOk = true;*/
	video_frame_count_ = 0;
	init_ok_ = true;
	return true;
}

void MediaRecorder::EndRecord() {
	//mIsInitOk = false;
	init_ok_ = false;
	if (format_ctx_) {
		LOGI(" MediaRecord::End()");
		av_write_trailer(format_ctx_);
		if (video_stream_index_ > -1) {
			//avcodec_close(format_ctx_->streams[video_stream_index_]->codec);
			//av_freep(&format_ctx_->streams[video_stream_index_]->codec);
			av_freep(&format_ctx_->streams[video_stream_index_]);
		}

		if (audio_stream_index > -1) {
			//avcodec_close(format_ctx_->streams[audio_stream_index]->codec);
			//av_freep(&format_ctx_->streams[audio_stream_index]->codec);
			av_freep(&format_ctx_->streams[audio_stream_index]);
		}
		avio_close(format_ctx_->pb);
		av_free(format_ctx_);
		format_ctx_ = nullptr;
	}
	//mIsCanStart = true;
	video_stream_index_ = -1;
	audio_stream_index = -1;
	last_width_ = -1;
	last_height_ = -1;
	video_frame_count_ = 0;

	LOGI(" MediaRecord::End() 2");
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
	else { // vp9 ? no supprot


	}


	LOGI("InitByVideoFrame  is_config_frame: {}, , key: {}", is_config_frame, frame.key());

	if (!is_config_frame) {
		return;
	}

	LOGI("InitByVideoFrame 2");


	LOGI("RecvVideoFrame is_config_frame");

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

	LOGI("RecvVideoFrame video_frame_count_ : {}", video_frame_count_);

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

	EndRecord();

	InitByVideoFrame(frame);
}

void MediaRecorder::RecvAudioFrame(const AudioFrame& frame) {

}

void MediaRecorder::SaveVideoFrame(const VideoFrame& frame) {
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
	auto& d = frame.data();
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