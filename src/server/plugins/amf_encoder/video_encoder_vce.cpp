#include "video_encoder_vce.h"

#include "tc_common_new/log.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/data.h"
#include "tc_encoder_new/encoder_config.h"
#include "tc_common_new/image.h"
#include "tc_common_new/defer.h"
#include "d3d_texture_debug.h"
#include "tc_encoder_new/frame_render/FrameRender.h"
#include "tc_common_new/string_ext.h"
#include <combaseapi.h>
#include <atlbase.h>
#include <fstream>
#include <iostream>
#include <utility>

#define DEBUG_FILE 0

#define AMF_LOG_ERR_IF(expr) {AMF_RESULT res = expr;\
    if(res != AMF_OK){LOGE("ERROR: {}", res);}}

const wchar_t* START_TIME_PROPERTY = L"StartTimeProperty";
const wchar_t* FRAME_INDEX_PROPERTY = L"FrameIndexProperty";
const wchar_t* IS_KEY_FRAME = L"IsKeyFrame";
static uint64_t last_time = tc::TimeExt::GetCurrentTimestamp();
static int fps = 0;

namespace tc
{
    AMFTextureEncoder::AMFTextureEncoder(const amf::AMFContextPtr &amfContext, EncoderConfig config,
                                         amf::AMF_SURFACE_FORMAT inputFormat, AMFTextureReceiver receiver)
                                         : receiver_(std::move(receiver)) {
        codec_ = config.codec_type;
        const wchar_t* pCodec = L"";

        amf_int32 frameRateIn = config.fps;
        amf_int64 bitRateIn = config.bitrate * 000000L; //5Mbits// Settings::Instance().m_encodeBitrateInMBits * 1000000L; // in bits

        switch (codec_) {
            case EVideoCodecType::kH264:
                pCodec = AMFVideoEncoderVCE_AVC;
                break;
            case EVideoCodecType::kHEVC:
                pCodec = AMFVideoEncoder_HEVC;
                break;
            default:
                LOGE("Error codec type: {}", (int)codec_);
        }

        // Create encoder component.
        auto ret = g_AMFFactory.GetFactory()->CreateComponent(amfContext, pCodec, &amf_encoder_);
        if (ret != AMF_OK) {
            LOGE("CreateComponent failed: {}", ret);
            return;
        }

        if (codec_ == EVideoCodecType::kH264) {
            //m_amfEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
            //m_amfEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCODING);
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 0);
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitRateIn);
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(config.width, config.height));
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(frameRateIn, 1));

            //m_amfEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH);
            //m_amfEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, 51);
        } else {
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_USAGE, AMF_VIDEO_ENCODER_HEVC_USAGE_ULTRA_LOW_LATENCY);
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET, AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY);
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_TARGET_BITRATE, bitRateIn);
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_FRAMESIZE, ::AMFConstructSize(config.width, config.height));
            amf_encoder_->SetProperty(AMF_VIDEO_ENCODER_HEVC_FRAMERATE, ::AMFConstructRate(frameRateIn, 1));

            //m_amfEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_TIER, AMF_VIDEO_ENCODER_HEVC_TIER_HIGH);
            //m_amfEncoder->SetProperty(AMF_VIDEO_ENCODER_HEVC_PROFILE_LEVEL, AMF_LEVEL_5);
        }

        ret = amf_encoder_->Init(inputFormat, config.width, config.height);
        if (ret != AMF_OK) {
            LOGE("!!! AMF encoder init failed: {}, {}x{}, format: {}", ret, config.width, config.height, inputFormat);
            return;
        }

        LOGI("Initialized AMFTextureEncoder.");
    }

    AMFTextureEncoder::~AMFTextureEncoder() {
        if (amf_encoder_) {
            amf_encoder_->Terminate();
        }
    }

    void AMFTextureEncoder::Start() {
        thread_ = new std::thread(&AMFTextureEncoder::Run, this);
    }

    void AMFTextureEncoder::Shutdown() {
        amf_encoder_->Drain();
        thread_->join();
        delete thread_;
        thread_ = nullptr;
    }

    void AMFTextureEncoder::Submit(amf::AMFData *data) {
        while (true) {
            auto res = amf_encoder_->SubmitInput(data);
            if (res == AMF_INPUT_FULL) {
                return;
            } else {
                break;
            }
        }
    }

    void AMFTextureEncoder::Run() {
        LOGI("Start AMFTextureEncoder thread. Thread Id={}", GetCurrentThreadId());
        amf::AMFDataPtr data;
        while (true) {
            auto res = amf_encoder_->QueryOutput(&data);
            if (res == AMF_EOF) {
                LOGI("m_amfEncoder->QueryOutput returns AMF_EOF.");
                return;
            }

            if (data != nullptr) {
                receiver_(data);
            } else {
                Sleep(1);
            }
        }
    }

    AMFTextureConverter::AMFTextureConverter(const amf::AMFContextPtr &amfContext, int width, int height,
                                             amf::AMF_SURFACE_FORMAT inputFormat, amf::AMF_SURFACE_FORMAT outputFormat,
                                             AMFTextureReceiver receiver) : receiver_(std::move(receiver)) {
        AMF_LOG_ERR_IF(g_AMFFactory.GetFactory()->CreateComponent(amfContext, AMFVideoConverter, &amf_converter_));
        AMF_LOG_ERR_IF(amf_converter_->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, amf::AMF_MEMORY_DX11));
        AMF_LOG_ERR_IF(amf_converter_->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, outputFormat));
        AMF_LOG_ERR_IF(amf_converter_->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_SIZE, ::AMFConstructSize(width, height)));
        AMF_LOG_ERR_IF(amf_converter_->Init(inputFormat, width, height));
        LOGI("Initialized AMFTextureConverter.");
    }

    AMFTextureConverter::~AMFTextureConverter() {
        if (amf_converter_) {
            amf_converter_->Terminate();
        }
    }

    void AMFTextureConverter::Start() {
        thread_ = new std::thread(&AMFTextureConverter::Run, this);
    }

    void AMFTextureConverter::Shutdown() {
        amf_converter_->Drain();
        thread_->join();
        delete thread_;
        thread_ = nullptr;
    }

    void AMFTextureConverter::Submit(amf::AMFData *data) {
        while (true) {
            auto res = amf_converter_->SubmitInput(data);
            if (res == AMF_INPUT_FULL) {
                return;
            } else {
                break;
            }
        }
    }

    void AMFTextureConverter::Run() {
        amf::AMFDataPtr data;
        while (true) {
            auto res = amf_converter_->QueryOutput(&data);
            if (res == AMF_EOF) {
                LOGE("m_amfConverter->QueryOutput returns AMF_EOF.");
                return;
            }

            if (data != nullptr) {
                receiver_(data);
            } else {
                Sleep(1);
            }
        }
    }

    VideoEncoderVCE::VideoEncoderVCE(uint64_t adapter_uid) {
        ComPtr<IDXGIFactory1> factory1;
        ComPtr<IDXGIAdapter1> adapter;
        DXGI_ADAPTER_DESC desc;
        HRESULT res = NULL;
        bool found_adapter = false;
        int adapter_index = 0;
        res = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(factory1.GetAddressOf()));
        if (res != S_OK) {
            LOGE("CreateDXGIFactory1 failed");
            return;
        }
        while (true) {
            res = factory1->EnumAdapters1(adapter_index, adapter.GetAddressOf());
            if (res != S_OK) {
                LOGE("EnumAdapters1 index:{} failed\n", adapter_index);
                return;
            }

            adapter->GetDesc(&desc);
            if (adapter_uid == desc.AdapterLuid.LowPart) {
                found_adapter = true;
                LOGI("Adapter Index:{} Name: {}", adapter_index, StringExt::ToUTF8(desc.Description).c_str());
                LOGI("find adapter");
                break;
            }
            ++adapter_index;
        }

        D3D_FEATURE_LEVEL featureLevel;
        res = D3D11CreateDevice(adapter.Get(),
                                D3D_DRIVER_TYPE_UNKNOWN, nullptr,
                                D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                nullptr, 0, D3D11_SDK_VERSION,
                                &d3d11_device_, &featureLevel, &d3d11_device_context_);

        if (res != S_OK || !d3d11_device_) {
            LOGE("D3D11CreateDevice failed: {}", res);
        } else {
            LOGI("D3D11CreateDevice mDevice = {}", (void *) d3d11_device_.Get());
        }
    }

    VideoEncoderVCE::~VideoEncoderVCE() {

    }

    amf::AMF_SURFACE_FORMAT DXGI_FORMAT_to_AMF_FORMAT(DXGI_FORMAT format) {
        if (DXGI_FORMAT_R8G8B8A8_UNORM == format || DXGI_FORMAT_R8G8B8A8_UNORM_SRGB == format) {
            return amf::AMF_SURFACE_RGBA;
        } else if (DXGI_FORMAT_B8G8R8A8_UNORM == format || DXGI_FORMAT_B8G8R8A8_UNORM_SRGB == format) {
            return amf::AMF_SURFACE_BGRA;
        } else {
            LOGE("DONT known the dxgi format !");
            return amf::AMF_SURFACE_UNKNOWN;
        }
    }

    bool VideoEncoderVCE::Initialize(const tc::EncoderConfig &config) {
        codec_type_ = config.codec_type;
        auto ret = g_AMFFactory.Init();
        AMF_LOG_ERR_IF(ret);
        if (ret != AMF_OK) {
            return false;
        }
        ::amf_increase_timer_precision();
        ret = g_AMFFactory.GetFactory()->CreateContext(&amf_context_);
        AMF_LOG_ERR_IF(ret);
        if (ret != AMF_OK) {
            return false;
        }
        if (amf_context_->InitDX11(d3d11_device_.Get()) != AMF_OK) {
            LOGE("Amf context init dx11 failed.");
            return false;
        }

        auto format = static_cast<DXGI_FORMAT>(config.texture_format);
        convert_input_format_ = DXGI_FORMAT_to_AMF_FORMAT(format);
        encoder_input_format_ = DXGI_FORMAT_to_AMF_FORMAT(format);
        encoder_ = std::make_shared<AMFTextureEncoder>(amf_context_, config, encoder_input_format_,
                                                       std::bind(&VideoEncoderVCE::Receive, this, std::placeholders::_1));
        converter_ = std::make_shared<AMFTextureConverter>(amf_context_, config.width, config.height,
                                                           convert_input_format_, encoder_input_format_,
                                                           std::bind(&AMFTextureEncoder::Submit, encoder_.get(), std::placeholders::_1));

        encoder_->Start();
        converter_->Start();

        LOGI("Successfully initialized VideoEncoderVCE.");
        return true;
    }

    void VideoEncoderVCE::Encode(ID3D11Texture2D *tex2d, uint64_t frame_index) {
        D3D11_TEXTURE2D_DESC desc;
        tex2d->GetDesc(&desc);
        EncodeTexture(tex2d, desc.Width, desc.Height, frame_index);
    }

    void VideoEncoderVCE::Encode(const std::shared_ptr<Image> &i420_data, uint64_t frame_index) {

    }

    void VideoEncoderVCE::Exit() {
        this->Shutdown();
    }

    void VideoEncoderVCE::Shutdown() {
        LOGI("Shutting down VideoEncoderVCE.");
        encoder_->Shutdown();
        converter_->Shutdown();
        amf_restore_timer_precision();
        LOGI("Successfully shutdown VideoEncoderVCE.");
    }

    void VideoEncoderVCE::EncodeTexture(ID3D11Texture2D* texture, int width, int height, int64_t frame_idx) {
        amf::AMFSurfacePtr surface = nullptr;
        // Surface is cached by AMF.
        AMF_LOG_ERR_IF(amf_context_->AllocSurface(amf::AMF_MEMORY_DX11, convert_input_format_, width, height, &surface));
        if (!surface) {
            LOGE("AllocSurface failed!");
            return;
        }
        auto textureDX11 = (ID3D11Texture2D *) surface->GetPlaneAt(0)->GetNative(); // no reference counting - do not Release()
        d3d11_device_context_->CopyResource(textureDX11, texture);

        // print to check the format equal or not
        //D3DTextureDebug::PrintTextureDesc(textureDX11);
        //D3DTextureDebug::PrintTextureDesc(texture);

        // save to dds
        //D3DTextureDebug::SaveAsDDS(d3d11_device_context_.Get(), textureDX11, "aaaa.dds");
        //D3DTextureDebug::SaveAsDDS(d3d11_device_context_.Get(), shared_texture.Get(), "bbbb.dds");

        amf_pts start_time = amf_high_precision_clock();
        surface->SetProperty(START_TIME_PROPERTY, start_time);
        surface->SetProperty(FRAME_INDEX_PROPERTY, frame_idx);

        if (!insert_idr_) {
            if (frame_idx % gop_ == 0) {
                insert_idr_ = true;
            }
        }
        surface->SetProperty(IS_KEY_FRAME, insert_idr_);
        ApplyFrameProperties(surface, insert_idr_);
        insert_idr_ = false;

        // perhaps to convert
        //m_converter->Submit(surface);
        encoder_->Submit(surface);
    }

    void VideoEncoderVCE::Receive(amf::AMFData *data) {
        amf_pts current_time = amf_high_precision_clock();
        amf_pts start_time = 0;
        uint64_t frameIndex;
        bool is_key_frame;
        data->GetProperty(START_TIME_PROPERTY, &start_time);
        data->GetProperty(FRAME_INDEX_PROPERTY, &frameIndex);
        data->GetProperty(IS_KEY_FRAME, &is_key_frame);

        amf::AMFBufferPtr buffer(data); // query for buffer interface
        char *p = reinterpret_cast<char *>(buffer->GetNative());
        int length = (int)buffer->GetSize();

        SkipAUD(&p, &length);

        //LOGI("VCE encode latency: {} ms. Size={} bytes frameIndex={}, length : {}", double(current_time - start_time) / MILLISEC_TIME, (int)buffer->GetSize(), frameIndex, length);
#if DEBUG_FILE
        if (dbg_file_.is_open()) {
            dbg_file_.write(p, length);
        }
#endif
        auto encoded_data = Data::Make((char*)p, length);
//        if (encoder_callback_) {
//            auto image = Image::Make(encoded_data, out_width_, out_height_, 3);
//            encoder_callback_(image, ++encoded_frame_index_, insert_idr_);
//        }

        static std::ofstream file("234234.h264", std::ios::binary);
        file.write(encoded_data->CStr(), encoded_data->Size());

        fps++;
        uint64_t ct = tc::TimeExt::GetCurrentTimestamp();
        if (ct - last_time > 1000) {
            //LOGI("Recv FPS : {}", fps);
            last_time = ct;
            fps = 0;
        }

    }

    void VideoEncoderVCE::ApplyFrameProperties(const amf::AMFSurfacePtr &surface, bool insertIDR) {
        switch (codec_type_) {
            case EVideoCodecType::kH264:
                // Disable AUD (NAL Type 9) to produce the same stream format as VideoEncoderNVENC.
                surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_AUD, false);
                if (insertIDR) {
                    //LOGI("Inserting IDR frame for H.264.");
                    surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_SPS, true);
                    surface->SetProperty(AMF_VIDEO_ENCODER_INSERT_PPS, true);
                    surface->SetProperty(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);
                }
                break;
            case EVideoCodecType::kHEVC:
                // This option is ignored. Maybe a bug on AMD driver.
                surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_INSERT_AUD, false);
                if (insertIDR) {
                    //LOGI("Inserting IDR frame for H.265.");
                    // Insert VPS,SPS,PPS
                    // These options don't work properly on older AMD driver (Radeon Software 17.7, AMF Runtime 1.4.4)
                    // Fixed in 18.9.2 & 1.4.9
                    surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER, true);
                    surface->SetProperty(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_IDR);
                }
                break;
        }
    }

    void VideoEncoderVCE::SkipAUD(char **buffer, int *length) {
        // H.265 encoder always produces AUD NAL even if AMF_VIDEO_ENCODER_HEVC_INSERT_AUD is set. But it is not needed.
        static const int AUD_NAL_SIZE = 7;

        if (codec_type_ != EVideoCodecType::kHEVC) {
            return;
        }

        if (*length < AUD_NAL_SIZE + 4) {
            return;
        }

        // Check if start with AUD NAL.
        if (memcmp(*buffer, "\x00\x00\x00\x01\x46", 5) != 0) {
            return;
        }
        // Check if AUD NAL size is AUD_NAL_SIZE bytes.
        if (memcmp(*buffer + AUD_NAL_SIZE, "\x00\x00\x00\x01", 4) != 0) {
            return;
        }
        *buffer += AUD_NAL_SIZE;
        *length -= AUD_NAL_SIZE;
    }

}