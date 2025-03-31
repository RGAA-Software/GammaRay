#include "nvenc_video_encoder.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "tc_common_new/log.h"
#include "tc_common_new/win32/d3d_debug_helper.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/defer.h"
#include "tc_common_new/string_ext.h"
#include "plugin_interface/gr_plugin_events.h"
#include "nvenc_encoder_plugin.h"

namespace tc
{

    NVENCVideoEncoder::NVENCVideoEncoder(NvencEncoderPlugin* plugin, uint64_t adapter_uid) {
        plugin_ = plugin;
        d3d11_device_ = plugin->d3d11_device_;
        d3d11_device_context_ = plugin->d3d11_device_context_;
    }

    NVENCVideoEncoder::~NVENCVideoEncoder() = default;

    bool NVENCVideoEncoder::Initialize(const tc::EncoderConfig& config) {
        encoder_config_ = config;
        auto format = DxgiFormatToNvEncFormat(static_cast<DXGI_FORMAT>(config.texture_format));
        LOGI("input_frame_width_ = {}, input_frame_height_ = {}, format = {:x} , m_pD3DRender->GetDevice() = {}",
             config.width, config.height, (int) format, (void *) d3d11_device_.Get());
        try {
            nv_encoder_ = std::make_shared<NvEncoderD3D11>(d3d11_device_.Get(), config.width, config.height, format, 0);
        } catch (const NVENCException& e) {
            nv_encoder_ = nullptr;
            LOGI("NVENC NvEncoderD3D11 failed: {} => {}", e.getErrorCode(), e.what());
            return false;
        }
        NV_ENC_INITIALIZE_PARAMS initializeParams = {NV_ENC_INITIALIZE_PARAMS_VER};
        NV_ENC_CONFIG encode_config = {NV_ENC_CONFIG_VER};

        encode_config.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
        encode_config.rcParams.averageBitRate = 10 * 1024 * 1024;
        encode_config.rcParams.maxBitRate = 50 * 1024 * 1024;
        encode_config.rcParams.vbvBufferSize = 1 * 1024 * 1024;

        initializeParams.encodeConfig = &encode_config;
        FillEncodeConfig(initializeParams, config.fps, config.encode_width, config.encode_height, config.bitrate);
        try {
            nv_encoder_->CreateEncoder(&initializeParams);
        } catch (const NVENCException& e) {
            LOGI("Config failed: {} => {}", e.getErrorCode(), e.what());
            return false;
        }
        LOGI("NVENC init success.");
        return true;
    }

    void NVENCVideoEncoder::InsertIdr() {
        insert_idr_ = true;
    }

    void NVENCVideoEncoder::Exit() {
        Shutdown();
    }

    void NVENCVideoEncoder::Shutdown() {
        std::vector<std::vector<uint8_t>> out_packet;
        if (nv_encoder_) {
            nv_encoder_->EndEncode(out_packet);
        }
        if (nv_encoder_) {
            nv_encoder_->DestroyEncoder();
            nv_encoder_.reset();
        }
    }

    void NVENCVideoEncoder::Encode(ID3D11Texture2D *tex, uint64_t frame_index, std::any extra) {
        Transmit(tex, frame_index, extra);
    }

    void NVENCVideoEncoder::Transmit(ID3D11Texture2D* texture, uint64_t frame_index, std::any extra) {
        std::vector<std::vector<uint8_t>> out_packet;
        const NvEncInputFrame *input_frame = nv_encoder_->GetNextInputFrame();
        auto pInputTexture = reinterpret_cast<ID3D11Texture2D*>(input_frame->inputPtr);
        d3d11_device_context_->CopyResource(pInputTexture, texture);

        bool is_key_frame = false;
        NV_ENC_PIC_PARAMS picParams = {};
        if (insert_idr_) {
            picParams.encodePicFlags = NV_ENC_PIC_FLAG_FORCEIDR;
            insert_idr_ = false;
            is_key_frame = true;
        }
        nv_encoder_->EncodeFrame(out_packet, &picParams);

        CD3D11_TEXTURE2D_DESC desc;
        texture->GetDesc(&desc);

        for (std::vector<uint8_t> &packet: out_packet) {
            auto encoded_data = Data::Make((char *) packet.data(), packet.size());
            auto event = std::make_shared<GrPluginEncodedVideoFrameEvent>();
            event->type_ = [=, this]() {
                if (encoder_config_.codec_type == EVideoCodecType::kHEVC) {
                    return GrPluginEncodedVideoType::kH265;
                } else if (encoder_config_.codec_type == EVideoCodecType::kH264) {
                    return GrPluginEncodedVideoType::kH264;
                } else {
                    return GrPluginEncodedVideoType::kH264;
                }
            }();
            event->data_ = encoded_data;
            event->frame_width_ = desc.Width;
            event->frame_height_ = desc.Height;
            event->key_frame_ = is_key_frame;
            event->frame_index_ = frame_index;
            event->extra_ = extra;
            this->plugin_->CallbackEvent(event);
        }
    }

    void
    NVENCVideoEncoder::FillEncodeConfig(NV_ENC_INITIALIZE_PARAMS &initialize_params, int refreshRate, int renderWidth, int renderHeight, uint64_t bitrate_bps) {
        auto& encode_config = *initialize_params.encodeConfig;
        GUID encoder_guid = encoder_config_.codec_type == tc::EVideoCodecType::kH264 ? NV_ENC_CODEC_H264_GUID : NV_ENC_CODEC_HEVC_GUID;

        GUID quality_preset;

        // 随着从 P1 到 P7 的转变，性能下降而质量提高
        // See recommended NVENC settings for low-latency encoding.
        // https://docs.nvidia.com/video-technologies/video-codec-sdk/nvenc-video-encoder-api-prog-guide/#recommended-nvenc-settings
        switch (encoder_config_.quality_preset) {
            case 7:
                quality_preset = NV_ENC_PRESET_P7_GUID;
                break;
            case 6:
                quality_preset = NV_ENC_PRESET_P6_GUID;
                break;
            case 5:
                quality_preset = NV_ENC_PRESET_P5_GUID;
                break;
            case 4:
                quality_preset = NV_ENC_PRESET_P4_GUID;
                break;
            case 3:
                quality_preset = NV_ENC_PRESET_P3_GUID;
                break;
            case 2:
                quality_preset = NV_ENC_PRESET_P2_GUID;
                break;
            case 1:
            default:
                quality_preset = NV_ENC_PRESET_P1_GUID;
                break;
        }

        //Tuning information of NVENC encoding(TuningInfo is not applicable to H264 and HEVC MEOnly mode).
        //MEOnly（Motion Estimation Only）模式是指在视频编码中仅使用运动估计（Motion Estimation）的一种模式。在传统的视频编码过程中，运动估计是编码器中的一个重要步骤，用于分析图像帧之间的运动信息，并生成运动矢量（Motion Vector）。这些运动矢量用于描述帧间的运动差异，从而实现视频帧的压缩。
        //在MEOnly模式中，编码器仅执行运动估计步骤，而不进行实际的编码和压缩操作。它通常用于一些特殊的应用场景，例如视频编辑和后期处理中的运动补偿、运动分析或运动跟踪等。通过仅执行运动估计，可以提取图像帧之间的运动信息，而无需实际进行编码和压缩操作。
        //需要注意的是，MEOnly模式在H.264和HEVC编码中并不适用。H.264和HEVC是一种基于帧间预测的视频编码标准，需要进行更多的编码步骤（如变换、量化、熵编码等）来实现高效的压缩。因此，MEOnly模式通常用于其他类型的编码器或特定的视频处理任务中。


        //低延迟流媒体
        //NV_ENC_TUNING_INFO_LOW_LATENCY       = 2,                                     /**< Tune presets for low latency streaming.
        //超低延迟流媒体
        //NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY = 3,                                     /**< Tune presets for ultra low latency streaming.

        NV_ENC_TUNING_INFO tuning_preset = NV_ENC_TUNING_INFO_LOW_LATENCY;
        nv_encoder_->CreateDefaultEncoderParams(&initialize_params, encoder_guid, quality_preset, tuning_preset);

        initialize_params.encodeWidth = initialize_params.darWidth = renderWidth;
        initialize_params.encodeHeight = initialize_params.darHeight = renderHeight;
        initialize_params.frameRateNum = refreshRate;
        initialize_params.frameRateDen = 1;

        // enableWeightedPrediction 是否启用加权预测

        //enableWeightedPrediction 是一个编码参数，用于启用或禁用加权预测（Weighted Prediction）。
        //加权预测是一种视频编码技术，用于更准确地预测帧间的像素值。在帧间预测中，编码器使用先前的参考帧来预测当前帧的像素值。
        //加权预测通过为不同像素位置分配不同的权重，以更好地适应图像中的变化和运动情况。
        //当 enableWeightedPrediction 参数被设置为启用时，编码器将使用加权预测来提高预测的准确性。
        //这可以在某些情况下改善编码的效果，特别是对于具有大幅度运动或复杂纹理的视频内容。
        //然而，需要注意的是，加权预测可能会增加编码的计算复杂性，并略微增加比特率。
        //因此，在某些情况下，禁用加权预测可能更适合，例如在对编码速度有更高要求的实时应用中。

        initialize_params.enableWeightedPrediction = 0;

        // 16 is recommended when using reference frame invalidation. But it has caused bad visual quality.
        // Now, use 0 (use default).

        //在使用参考帧无效化时，推荐使用16作为DPB大小。然而，这可能导致视觉质量下降
        //指定用于编码的 DPB（Decoded Picture Buffer）大小。将其设置为 0 将让驱动程序使用默认的 DPB 大小。
        //对于希望将无效的参考帧作为错误容忍工具的低延迟应用程序，建议使用较大的 DPB 大小，这样编码器可以保留旧的参考帧，以便在最近的帧被无效化时使用。

        uint32_t maxNumRefFrames = 0;
        uint32_t gopLength = NVENC_INFINITE_GOPLENGTH;

        if (encoder_config_.gop_size != -1) {
            gopLength = encoder_config_.gop_size;
        }

        if (encoder_config_.codec_type == tc::EVideoCodecType::kH264) {
            auto &config = encode_config.encodeCodecConfig.h264Config;
            //将其设置为1以启用在每个IDR帧中写入序列参数（Sequence Parameter）和图像参数（Picture Parameter）。
            //等再研究下这两种参数
            config.repeatSPSPPS = 1;

            //enableIntraRefresh
            //将其设置为1以启用逐渐解码器刷新（gradual decoder refresh）或帧内刷新（intra refresh）。如果 GOP 结构使用了 B 帧，则此设置将被忽略。
            //逐渐解码器刷新是一种技术，可以在视频序列中定期插入额外的帧内编码帧，以减少编码引起的累积错误和失真

            if (encoder_config_.supports_intra_refresh) {
                config.enableIntraRefresh = 1;
                config.intraRefreshPeriod = refreshRate * 10;
                config.intraRefreshCnt = refreshRate;
            }

            //CABAC 提供了更高的编码效率，但需要更多的计算资源和硬件支持。而 CAVLC 具有较低的计算复杂度和硬件要求，适用于资源受限或对编码效率要求不高的场景。
            //在选择熵编码模式时，可以根据具体应用的需求和硬件平台的支持来进行选择。

            config.entropyCodingMode = NV_ENC_H264_ENTROPY_CODING_MODE_CAVLC;
            config.maxNumRefFrames = maxNumRefFrames;
            config.idrPeriod = gopLength;

            // api version = 12

            //设置为1以在比特流中插入填充数据。
            //该标志仅在使用CBR（恒定比特率）的其中一种速率控制模式（
            //NV_ENC_PARAMS_RC_CBR、NV_ENC_PARAMS_RC_CBR_HQ、NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ）
            //并且NV_ENC_INITIALIZE_PARAMS::frameRateNum和NV_ENC_INITIALIZE_PARAMS::frameRateDen都设置为非零值时才生效。
            //在NV_ENC_INITIALIZE_PARAMS::enableOutputInVidmem也设置的情况下设置此字段目前不受支持，会导致::NvEncInitializeEncoder()返回错误。
            // 这里先不启用了，不知道会不会 影响客户端解码。如果启用了，在比特流中 插入了填充数据，那么客户端解码时候，应该如何使用呢？

            // config.enableFillerDataInsertion;
        } else {
            auto &config = encode_config.encodeCodecConfig.hevcConfig;
            config.repeatSPSPPS = 1;
            if (encoder_config_.supports_intra_refresh) {
                config.enableIntraRefresh = 1;
                // Do intra refresh every 10sec.
                config.intraRefreshPeriod = refreshRate * 10;
                config.intraRefreshCnt = refreshRate;
            }
            config.maxNumRefFramesInDPB = maxNumRefFrames;
            config.idrPeriod = gopLength;
        }
        encode_config.gopLength = gopLength;
        encode_config.frameIntervalP = 1; // forbidden B frame
        // 恒定码率 还是 可变码率
        //return; // no error
        switch (encoder_config_.rate_control_mode) {
            case tc::ERateControlMode::kRateControlModeCbr:
                encode_config.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
                break;
            case tc::ERateControlMode::kRateControlModeVbr:
                encode_config.rcParams.rateControlMode = NV_ENC_PARAMS_RC_VBR;
                // 在 NVIDIA Video Codec (NVENC) SDK 中，targetQuality 是用于可变比特率（VBR）模式的参数设置之一。
                // Target CQ (Constant Quality) level for VBR mode (range 0-51 with 0-automatic)
                // targetQuality 值越大编码质量越好
                // encode_config.rcParams.targetQuality = 28;
                if (encoder_config_.target_quality != -1) {
                    encode_config.rcParams.targetQuality = encoder_config_.target_quality;
                }

                break;
            case tc::ERateControlMode::kRateControlModeConstQp:
                encode_config.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CONSTQP;
                break;
            default:
                break;
        }

        //	_NV_ENC_MULTI_PASS 多次编码设置   8.x版本的api没有这个参数
        //	NV_ENC_MULTI_PASS_DISABLED：表示禁用多次编码过程。在单次编码过程中，每个视频帧只编码一次，没有额外的预处理或后处理步骤。
        //	使用此标志时，编码器不会保存任何状态信息，并且不需要执行多次编码。
        //
        //	NV_ENC_TWO_PASS_QUARTER_RESOLUTION：表示使用两次编码过程，其中第一次编码过程使用四分之一的分辨率进行编码。
        //	在第一次编码过程中，视频帧以较低的分辨率进行编码，以获得初步的编码统计信息。然后，编码器使用这些统计信息来优化第二次编码过程，
        //	该过程使用完整的分辨率进行编码。这种方法可以在保持较高质量的同时减少编码的计算成本。
        //
        //	NV_ENC_TWO_PASS_FULL_RESOLUTION：表示使用两次编码过程，其中两次编码过程均使用完整的分辨率进行编码。
        //	在第一次编码过程中，视频帧以完整的分辨率进行编码，并生成编码统计信息。然后，编码器使用这些统计信息来优化第二次编码过程，
        //	该过程仍使用完整的分辨率进行编码。这种方法可以进一步提高编码质量，但需要更多的计算资源。
        switch (encoder_config_.multi_pass) {
            case tc::ENvdiaEncMultiPass::kMultiPassDisabled:
                encode_config.rcParams.multiPass = NV_ENC_MULTI_PASS_DISABLED;
                break;
            case tc::ENvdiaEncMultiPass::kTwoPassQuarterResolution:
                encode_config.rcParams.multiPass = NV_ENC_TWO_PASS_QUARTER_RESOLUTION;
                break;
            case tc::ENvdiaEncMultiPass::kTwoPassFullResolution:
                encode_config.rcParams.multiPass = NV_ENC_TWO_PASS_FULL_RESOLUTION;
                break;
            default:
                break;
        }

        //	在单帧 VBV（Video Buffering Verifier）和 CBR（Constant Bit Rate）速率控制模式下，指定 I 帧比 P 帧的比特率。对于低延迟调优信息，默认设置为 2；
        //	对于超低延迟调优信息，默认设置为 1。
        encode_config.rcParams.lowDelayKeyFrameScale = 1;
        auto maxFrameSize = static_cast<uint32_t>(bitrate_bps / refreshRate);
        encode_config.rcParams.vbvBufferSize = maxFrameSize * 1.1;
        encode_config.rcParams.vbvInitialDelay = maxFrameSize * 1.1;
        encode_config.rcParams.maxBitRate = static_cast<uint32_t>(bitrate_bps);
        encode_config.rcParams.averageBitRate = static_cast<uint32_t>(bitrate_bps);

        //	在 NVIDIA Video Codec (NVENC) SDK 中，_NV_ENC_RC_PARAMS 结构体中的 enableAQ 字段用于启用自适应量化（Adaptive Quantization）。
        //	自适应量化是一种编码技术，用于根据图像内容的复杂性动态调整量化参数。量化参数控制编码过程中的压缩比和图像质量之间的权衡。通过自适应量化，
        //	编码器可以根据图像的空间特征和运动信息，针对不同区域和帧间预测类型采用不同的量化参数，从而提高编码效率和图像质量。
        //	enableAQ 字段的含义是是否启用自适应量化。如果将其设置为非零值（通常为 1），则表示启用自适应量化；如果将其设置为零，则表示禁用自适应量化。
        //	通过启用自适应量化，编码器可以根据图像内容进行动态调整，以获得更好的压缩效率和图像质量。但请注意，自适应量化可能会增加编码的计算复杂性，并可能对编码性能产生一定的影响。因此，在使用 enableAQ 字段时需要权衡编码质量、性能和计算资源的需求。
        encode_config.rcParams.enableAQ = 0;
        if (encoder_config_.enable_adaptive_quantization) {
            encode_config.rcParams.enableAQ = 1;
        }


        //	在 NVIDIA Video Codec (NVENC) SDK 中，_NV_ENC_RC_PARAMS 结构体中的 enableTemporalAQ 字段用于启用时域自适应量化（Temporal Adaptive Quantization）。
        //	时域自适应量化是一种编码技术，基于视频序列中帧间相关性的变化，动态调整量化参数。与空间自适应量化（Adaptive Quantization）不同，
        //	时域自适应量化考虑了帧间的相关性，以提高编码效率和图像质量。
        //	enableTemporalAQ 字段的含义是是否启用时域自适应量化。如果将其设置为非零值（通常为 1），则表示启用时域自适应量化；如果将其设置为零，则表示禁用时域自适应量化。
        //	启用时域自适应量化可以在编码过程中根据帧间相关性进行动态调整，以优化压缩效率和图像质量。
        //	它可以根据帧间运动信息和相关性来调整量化参数，以便更好地保留运动细节和细微差异，从而提高编码效果。
        //	需要注意的是，启用时域自适应量化可能会增加编码器的计算复杂性，并可能对编码性能产生一定的影响。
        //	因此，在使用 enableTemporalAQ 字段时需要根据具体情况权衡编码质量、性能和计算资源的需求。

        // 这里先不启用
        // encode_config.rcParams.enableTemporalAQ = 1;

        //  qp 通常情况下，QP（量化参数）值越大，编码质量越差
        //	QP参数调节，指的是量化参数调节。它主要是来调节图像的细节，最终达到调节画面质量的作用。
        //	QP值和比特率成反比，QP值越小画面质量越高；反之QP值越大，画面质量越低。
        //	而且随着视频源复杂度，这种反比的关系会更加明显。QP调节是改变画面质量最常用的手段之一
        //
        //	在 NVIDIA Video Codec (NVENC) SDK 中，constQP、maxQP 和 minQP 是用于控制量化参数（QP）的配置参数。
        //	constQP：指定恒定量化参数（Constant QP），即所有帧使用相同的固定量化参数值。这是一种简单的编码模式，
        //	可以提供一致的编码质量，但无法根据图像内容进行自适应调整。
        //	maxQP 和 minQP：用于指定动态量化参数的范围，即允许的最大和最小量化参数值。编码器可以在该范围内根据图像内容进行自适应调整，以平衡压缩率和图像质量。
        //这三个参数之间的关系是：
        //	maxQP 必须大于或等于 minQP，以确保范围的有效性。
        //	constQP 可以设置为介于 maxQP 和 minQP 之间的任意值，即在动态量化参数范围内选择一个恒定的值。
        //	如果同时设置了 constQP 和动态量化参数范围（maxQP 和 minQP），编码器将优先使用 constQP 的值，并忽略动态范围的调整。
        //	总结起来，constQP 提供了一个固定的量化参数值，而 maxQP 和 minQP 定义了动态范围，允许编码器根据图像内容进行自适应调整。
        //	这些参数的使用取决于应用的需求和目标，需要权衡编码质量、压缩率和资源消耗。

        if (encoder_config_.const_qp != -1) {
            uint32_t constQP = encoder_config_.const_qp;
            encode_config.rcParams.constQP = {constQP, constQP, constQP};
        } else if (encoder_config_.max_qp != -1 && encoder_config_.min_qp != -1) {
            uint32_t minQP = encoder_config_.min_qp;
            uint32_t maxQP = encoder_config_.max_qp;
            encode_config.rcParams.minQP = {minQP, minQP, minQP};
            encode_config.rcParams.maxQP = {maxQP, maxQP, maxQP};
            encode_config.rcParams.enableMinQP = 1;
            encode_config.rcParams.enableMaxQP = 1;
        }
    }


    NV_ENC_BUFFER_FORMAT NVENCVideoEncoder::DxgiFormatToNvEncFormat(DXGI_FORMAT dxgiFormat) {
        switch (dxgiFormat) {
            case DXGI_FORMAT_NV12:
                return NV_ENC_BUFFER_FORMAT_NV12;
            case DXGI_FORMAT_B8G8R8A8_UNORM:
                return NV_ENC_BUFFER_FORMAT_ARGB;
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                return NV_ENC_BUFFER_FORMAT_ABGR;
            case DXGI_FORMAT_R10G10B10A2_UNORM:
                return NV_ENC_BUFFER_FORMAT_ABGR10;
            default:
                return NV_ENC_BUFFER_FORMAT_UNDEFINED;
        }
    }


} // namespace tc