#include <iostream>

#include "app.h"
#include "settings/settings.h"
#include "context.h"
#include "tc_common_new/log.h"
#include "tc_common_new/dump_helper.h"
#include "gflags/gflags.h"

using namespace tc;

DEFINE_int32(steam_app_id, 0, "steam app id");
DEFINE_bool(logfile, false, "log to file");
DEFINE_bool(isolate, false, "only use settings.toml, args below aren't be used");

// encoder
DEFINE_string(encoder_select_type, "auto", "auto/specify");
DEFINE_string(encoder_name, "nvenc", "nvenc/amf/ffmpeg");
DEFINE_string(encoder_format, "h264", "h264/h265");
DEFINE_int32(encoder_bitrate, 20, "encoder bitrate");
DEFINE_string(encoder_resolution_type, "origin", "origin/specify");
DEFINE_int32(encoder_width, 1280, "");
DEFINE_int32(encoder_height, 720, "");

// capture
DEFINE_bool(capture_audio, true, "");
DEFINE_string(capture_audio_type, "global", "hook/global");
DEFINE_bool(capture_video, true, "");
DEFINE_string(capture_video_type, "global", "hook/global");

// network
DEFINE_string(network_type, "websocket", "");
DEFINE_int32(network_listen_port, 20371, "");

// application
DEFINE_string(app_game_path, "", "");
DEFINE_string(app_game_args, "", "");

void UpdateSettings(Settings* settings) {
    LOGI("--------------In args begin--------------");
    LOGI("steam_app_id: {}", FLAGS_steam_app_id);
    LOGI("logfile: {}", FLAGS_logfile);
    LOGI("encoder_select_type: {}", FLAGS_encoder_select_type);
    LOGI("encoder_name: {}", FLAGS_encoder_name);
    LOGI("encoder_format: {}", FLAGS_encoder_format);
    LOGI("encoder_bitrate: {}", FLAGS_encoder_bitrate);
    LOGI("encoder_resolution_type: {}", FLAGS_encoder_resolution_type);
    LOGI("encoder_width: {}", FLAGS_encoder_width);
    LOGI("encoder_height: {}", FLAGS_encoder_height);
    LOGI("capture_audio: {}", FLAGS_capture_audio);
    LOGI("capture_audio_type: {}", FLAGS_capture_audio_type);
    LOGI("capture_video: {}", FLAGS_capture_video);
    LOGI("capture_video_type: {}", FLAGS_capture_video_type);
    LOGI("network_type: {}", FLAGS_network_type);
    LOGI("network_listen_port: {}", FLAGS_network_listen_port);
    LOGI("app_game_path: {}", FLAGS_app_game_path);
    LOGI("app_game_args: {}", FLAGS_app_game_args);
    LOGI("--------------In args end----------------");
    if (FLAGS_steam_app_id > 0) {
        settings->app_.steam_app_.app_id_ = FLAGS_steam_app_id;
        settings->app_.steam_app_.steam_url_ = std::format("steam://rungameid/{}", FLAGS_steam_app_id);
    }

    // encoder
    if (FLAGS_encoder_select_type == "auto") {
        settings->encoder_.encoder_select_type_ = ECreateEncoderPolicy::kAuto;
    } else {
        settings->encoder_.encoder_select_type_ = ECreateEncoderPolicy::kSpecify;
    }

    if (FLAGS_encoder_name == "nvenc") {
        settings->encoder_.encoder_name_ = ECreateEncoderName::kNVENC;
    } else if (FLAGS_encoder_name == "amf") {
        settings->encoder_.encoder_name_ = ECreateEncoderName::kAMF;
    } else {
        settings->encoder_.encoder_name_ = ECreateEncoderName::kFFmpeg;
    }

    if (FLAGS_encoder_format == "h264") {
        settings->encoder_.encoder_format_ = Encoder::EncoderFormat::kH264;
    } else {
        settings->encoder_.encoder_format_ = Encoder::EncoderFormat::kHEVC;
    }

    settings->encoder_.bitrate_ = FLAGS_encoder_bitrate;

    if (FLAGS_encoder_resolution_type == "origin") {
        settings->encoder_.encode_res_type_ = Encoder::EncodeResolutionType::kOrigin;
    } else {
        settings->encoder_.encode_res_type_ = Encoder::EncodeResolutionType::kSpecify;
    }
    settings->encoder_.encode_width_ = FLAGS_encoder_width;
    settings->encoder_.encode_height_ = FLAGS_encoder_height;

    // capture
    settings->capture_.enable_audio_ = FLAGS_capture_audio;
    if (FLAGS_capture_audio_type == "global") {
        settings->capture_.capture_audio_type_ = Capture::CaptureAudioType::kAudioGlobal;
    } else {
        settings->capture_.capture_audio_type_ = Capture::CaptureAudioType::kAudioHook;
    }

    settings->capture_.enable_video_ = FLAGS_capture_video;
    if (FLAGS_capture_video_type == "global") {
        settings->capture_.capture_video_type_ = Capture::CaptureVideoType::kCaptureScreen;
    } else {
        settings->capture_.capture_video_type_ = Capture::CaptureVideoType::kVideoHook;
    }

    // network !! only support websocket now
    if (FLAGS_network_type == "websocket") {
        settings->transmission_.network_type_ = Transmission::NetworkType::kWebsocket;
    } else {
        settings->transmission_.network_type_ = Transmission::NetworkType::kWebsocket;
    }
    settings->transmission_.listening_port_ = FLAGS_network_listen_port;

    // app
    if (!FLAGS_app_game_path.empty()) {
        settings->app_.game_path_ = FLAGS_app_game_path;
    }
    if (!FLAGS_app_game_args.empty()) {
        settings->app_.game_arguments_ = FLAGS_app_game_args;
    }
}

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // dump
    CaptureDump();

    // Log
    std::cout << "logfile: " << FLAGS_logfile << std::endl;
    Logger::InitLog("GammaRayServer.log", FLAGS_logfile);

    // 1. load from config.toml
    auto settings = Settings::Instance();
    settings->LoadSettings("settings.toml");
    if (!FLAGS_isolate) {
        UpdateSettings(settings);
    }
    auto settings_str = settings->Dump();
    LOGI("\n" + settings_str);

    // start application
    tc::AppParams params = {};
    auto app = tc::Application::Make(params);
    app->CaptureControlC();

    //
    auto task = AppMessageMaker::MakeTaskMessage([]() {
        LOGI("Server started...");
    });
    app->PostGlobalAppMessage(std::move(task));

    return app->Run();
}
