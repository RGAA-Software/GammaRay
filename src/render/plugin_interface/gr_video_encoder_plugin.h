//
// Created by RGAA on 19/11/2024.
//

#ifndef GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H
#define GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H

#include <d3d11.h>
#include <mutex>
#include <map>
#include <optional>
#include "gr_plugin_interface.h"
#include "tc_encoder_new/encoder_config.h"

namespace tc
{
    class Image;

    // dynamic working capture information
    class WorkingEncoderInfo {
    public:
        // monitor name or hook capturing
        std::string target_name_;
        int32_t fps_ = 0;
        // NVENC / AMF / SOFTWARE
        std::string encoder_name_;
        // max 180
        std::vector<int32_t> encode_durations_;
    };
    using WorkingEncoderInfoPtr = std::shared_ptr<WorkingEncoderInfo>;

    // encoder capability
    class EncoderCapability {
    public:
        bool support_h264_yuv444_ = false;
        bool support_hevc_yuv444_ = false;
    };

    // Video encoder error type
    enum class VideoEncoderErrorType {
        kOk,
        kNotFound,
        kEncodeFailed,
        kInvalidInput,
        kNotImplemented,
        kUnknown,
    };

    // Video encoder error
    class VideoEncoderError {
    public:
        static VideoEncoderError Ok() {
            VideoEncoderError err;
            err.type_ = VideoEncoderErrorType::kOk;
            return err;
        }

        static VideoEncoderError NotFound() {
            VideoEncoderError err;
            err.type_ = VideoEncoderErrorType::kNotFound;
            return err;
        }

        static VideoEncoderError EncodeFailed() {
            VideoEncoderError err;
            err.type_ = VideoEncoderErrorType::kEncodeFailed;
            return err;
        }

        static VideoEncoderError InvalidInput() {
            VideoEncoderError err;
            err.type_ = VideoEncoderErrorType::kInvalidInput;
            return err;
        }

        static VideoEncoderError NotImplemented() {
            VideoEncoderError err;
            err.type_ = VideoEncoderErrorType::kNotImplemented;
            return err;
        }

        [[nodiscard]] bool Success() const {
            return type_ == VideoEncoderErrorType::kOk;
        }

        [[nodiscard]] std::string GetReadableType() const {
            if (type_ == VideoEncoderErrorType::kOk) {
                return "Ok";
            }
            else if (type_ == VideoEncoderErrorType::kNotFound) {
                return "Not found encoder";
            }
            else if (type_ == VideoEncoderErrorType::kEncodeFailed) {
                return "Encode failed";
            }
            else if (type_ == VideoEncoderErrorType::kInvalidInput) {
                return "Invalid input";
            }
            else if (type_ == VideoEncoderErrorType::kNotImplemented) {
                return "Not implemented";
            }
            else {
                return "Unknown error";
            }
        }

    public:
        VideoEncoderErrorType type_ = VideoEncoderErrorType::kUnknown;
        int inner_error_ = 0;
        std::string msg_;
    };

    class GrVideoEncoderPlugin : public GrPluginInterface {
    public:
        GrVideoEncoderPlugin();
        ~GrVideoEncoderPlugin() override;

        bool OnCreate(const tc::GrPluginParam &param) override;
        bool OnDestroy() override;
        void InsertIdr() override;
        void On1Second() override;

        virtual bool CanEncodeTexture();
        virtual bool HasEncoderForMonitor(const std::string& monitor_name) = 0;
        virtual bool Init(const EncoderConfig& config, const std::string& monitor_name);
        virtual VideoEncoderError Encode(const Microsoft::WRL::ComPtr<ID3D11Texture2D>& tex2d, uint64_t frame_index, const std::any& extra);
        virtual VideoEncoderError Encode(const std::shared_ptr<Image>& i420_image, uint64_t frame_index, const std::any& extra);
        virtual void Exit(const std::string& monitor_name);
        virtual void ExitAll();
        // encoding information for monitors/hook
        virtual std::map<std::string, WorkingEncoderInfoPtr> GetWorkingCapturesInfo() = 0;

        std::optional<EncoderConfig> GetEncoderConfig(const std::string& monitor_name);

        // 如果客户端开始录屏，则需要设置此参数为true
        void SetClientSideMediaRecording(bool recording);

        virtual void ConfigEncoder(const std::string& mon_name, uint32_t bps, uint32_t fps) {}

        virtual std::optional<EncoderCapability> GetEncoderCapability(const std::string& monitor_name) { return std::nullopt;}
    public:
        int refresh_rate_ = 60;
        uint32_t out_width_ = 0;
        uint32_t out_height_ = 0;
        int gop_size_ = 60;
        int bitrate_ = 10000000; // 10Mbps
        bool insert_idr_ = false;
        std::map<std::string, EncoderConfig> encoder_configs_;

    private:
        std::atomic<bool> client_side_media_recording_ = false;
    };

}

#endif //GAMMARAY_GR_VIDEO_ENCODER_PLUGIN_H
