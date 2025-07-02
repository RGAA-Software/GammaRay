//
// Created by RGAA on 2023-12-26.
//

#include <QDir>
#include <memory>
#include <QApplication>
#include <QSurfaceFormat>
#include <QFontDatabase>
#include <QMessageBox>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QOpenGLWidget>
#include <qstandardpaths.h>
#include "thunder_sdk.h"
#include "client/ct_client_context.h"
#include "ct_base_workspace.h"
#include "client/ct_workspace.h"
#include "client/ct_base_workspace.h"
#include "client/ct_application.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/log.h"
#include "tc_common_new/base64.h"
#include "client/ct_settings.h"
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_qt_widget/tc_font_manager.h"
#include "translator/tc_translator.h"
#include "ct_stream_item_net_type.h"
#include "tc_common_new/dump_helper.h"
#include "tc_common_new/time_util.h"

using namespace tc;

std::string g_remote_host_;
int g_remote_port_ = 0;
std::string g_nt_type_;

void ParseCommandLine(QApplication& app) {
    QCommandLineParser parser;
    parser.setApplicationDescription("GammaRay Client");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption opt_host("host", "Host", "xx.xx.xx.xx", "");
    parser.addOption(opt_host);

    QCommandLineOption opt_port("port", "Port", "9999", "0");
    parser.addOption(opt_port);

    QCommandLineOption opt_audio("audio", "Audio enabled", "value", "0");
    parser.addOption(opt_audio);

    QCommandLineOption opt_clipboard("clipboard", "Clipboard", "value", "0" );
    parser.addOption(opt_clipboard);

    QCommandLineOption opt_ignore_mouse("ignore_mouse", "Ignore mouse event", "value", "0");
    parser.addOption(opt_ignore_mouse);

    QCommandLineOption opt_stream_id("stream_id", "Stream id", "value", "");
    parser.addOption(opt_stream_id);

    QCommandLineOption opt_network_type("network_type", "Network Type", "value", "");
    parser.addOption(opt_network_type);

    QCommandLineOption opt_conn_type("conn_type", "Conn Type", "value", "");
    parser.addOption(opt_conn_type);

    QCommandLineOption opt_stream_name("stream_name", "Stream name", "value", "");
    parser.addOption(opt_stream_name);

    QCommandLineOption opt_device_id("device_id", "device id", "value", "");
    parser.addOption(opt_device_id);

    QCommandLineOption opt_device_rp("device_rp", "device rp", "value", "");
    parser.addOption(opt_device_rp);

    QCommandLineOption opt_device_sp("device_sp", "device sp", "value", "");
    parser.addOption(opt_device_sp);

    QCommandLineOption opt_remote_device_id("remote_device_id", "remote_device id", "value", "");
    parser.addOption(opt_remote_device_id);

    QCommandLineOption opt_remote_device_rp("remote_device_rp", "remote_device rp", "value", "");
    parser.addOption(opt_remote_device_rp);

    QCommandLineOption opt_remote_device_sp("remote_device_sp", "remote_device sp", "value", "");
    parser.addOption(opt_remote_device_sp);

    QCommandLineOption opt_enable_p2p("enable_p2p", "enable p2p", "value", "0");
    parser.addOption(opt_enable_p2p);

    QCommandLineOption opt_show_max_window("show_max_window", "show max window", "value", "0");
    parser.addOption(opt_show_max_window);

    QCommandLineOption opt_display_name("display_name", "display name", "value", "");
    parser.addOption(opt_display_name);

    QCommandLineOption opt_display_remote_name("display_remote_name", "display remote name", "value", "");
    parser.addOption(opt_display_remote_name);

    QCommandLineOption opt_panel_server_port("panel_server_port", "panel server port", "value", "");
    parser.addOption(opt_panel_server_port);

    QCommandLineOption opt_screen_recording_path("screen_recording_path", "screen recording path", "value", "");
    parser.addOption(opt_screen_recording_path);

    QCommandLineOption opt_my_host("my_host", "my ip address", "value", "");
    parser.addOption(opt_my_host);

    QCommandLineOption opt_language("language", "language", "value", "");
    parser.addOption(opt_language);

    QCommandLineOption opt_only_viewing("only_viewing", "only viewing", "value", "");
    parser.addOption(opt_only_viewing);

    QCommandLineOption opt_split_windows("split_windows", "split windows", "value", "");
    parser.addOption(opt_split_windows);

    QCommandLineOption opt_max_num_of_screen("max_num_of_screen", "maximum allowed number of screens", "value", "2");
    parser.addOption(opt_max_num_of_screen);

    QCommandLineOption opt_display_logo("display_logo", "display logo", "value", "");
    parser.addOption(opt_display_logo);

    parser.process(app);

    g_remote_host_ = parser.value(opt_host).toStdString();
    g_remote_port_ = parser.value(opt_port).toInt();

    auto settings = tc::Settings::Instance();
    settings->host_ = g_remote_host_;
    settings->port_ = g_remote_port_;

    auto audio_on = parser.value(opt_audio).toInt();
    settings->audio_on_ = (audio_on == 1);

    auto clipboard_on = parser.value(opt_clipboard).toInt();
    settings->clipboard_on_ = (clipboard_on == 1);
    settings->stream_id_ = parser.value(opt_stream_id).toStdString();
    g_nt_type_ = parser.value(opt_network_type).toStdString();
    settings->network_type_ = [=]() -> ClientNetworkType {
        if (g_nt_type_ == kStreamItemNtTypeWebSocket) {
            return ClientNetworkType::kWebsocket;
        }
        else if (g_nt_type_ == kStreamItemNtTypeUdpKcp) {
            return ClientNetworkType::kUdpKcp;
        }
        else if (g_nt_type_ == kStreamItemNtTypeRelay) {
            return ClientNetworkType::kRelay;
        }
        else {
            return ClientNetworkType::kWebsocket;
        }
    }();

    settings->stream_name_ = parser.value(opt_stream_name).toStdString();
    if (!settings->stream_name_.empty()) {
        settings->stream_name_ = Base64::Base64Decode(settings->stream_name_);
    }
    settings->device_id_ = parser.value(opt_device_id).toStdString();
    settings->device_random_pwd_ = parser.value(opt_device_rp).toStdString();
    if (!settings->device_random_pwd_.empty()) {
        settings->device_random_pwd_ = Base64::Base64Decode(settings->device_random_pwd_);
    }
    settings->device_safety_pwd_ = parser.value(opt_device_sp).toStdString();
    if (!settings->device_safety_pwd_.empty()) {
        settings->device_safety_pwd_ = Base64::Base64Decode(settings->device_safety_pwd_);
    }

    settings->remote_device_id_ = parser.value(opt_remote_device_id).toStdString();
    settings->remote_device_random_pwd_ = parser.value(opt_remote_device_rp).toStdString();
    if (!settings->remote_device_random_pwd_.empty()) {
        settings->remote_device_random_pwd_ = Base64::Base64Decode(settings->remote_device_random_pwd_);
    }
    settings->remote_device_safety_pwd_ = parser.value(opt_remote_device_sp).toStdString();
    if (!settings->remote_device_safety_pwd_.empty()) {
        settings->remote_device_safety_pwd_ = Base64::Base64Decode(settings->remote_device_safety_pwd_);
    }

    settings->enable_p2p_ = parser.value(opt_enable_p2p).toInt() == 1;
    settings->show_max_window_ = parser.value(opt_show_max_window).toInt() == 1;

    settings->display_name_ = parser.value(opt_display_name).toStdString();
    settings->display_remote_name_ = parser.value(opt_display_remote_name).toStdString();

    {
        auto value = parser.value(opt_panel_server_port);
        if (!value.isEmpty()) {
            settings->panel_server_port_ = value.toInt();
        }
        else {
            settings->panel_server_port_ = 20369;
        }
    }

    {
        auto value = parser.value(opt_screen_recording_path);
        if (!value.isEmpty()) {
            settings->screen_recording_path_ = value.toStdString();
        }
        else {
            settings->screen_recording_path_ = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation).toStdString();
        }
    }

    // my host
    {
        auto value = parser.value(opt_my_host);
        if (!value.isEmpty()) {
            settings->my_host_ = value.toStdString();
        }
    }

    // language
    {
        auto value = parser.value(opt_language);
        if (!value.isEmpty()) {
            settings->language_ = value.toInt();
        }
    }

    // only viewing
    {
        auto value = parser.value(opt_only_viewing);
        if (!value.isEmpty()) {
            settings->only_viewing_ = value.toInt() == 1;
        }
    }

    // split windows
    {
        auto value = parser.value(opt_split_windows);
        if (!value.isEmpty()) {
            settings->split_windows_ = value.toInt() == 1;
        }
    }

    // max number of screen
    {
        auto value = parser.value(opt_max_num_of_screen);
        if (!value.isEmpty()) {
            settings->max_number_of_screen_window_ = value.toInt();
        }
    }

    // display logo
    {
        auto value = parser.value(opt_display_logo);
        if (!value.isEmpty()) {
            settings->display_logo_ = value.toInt() == 1;
        }
    }
}

bool PrepareDirs(const QString& base_path) {
    std::vector<QString> dirs = {
        "gr_logs", "gr_data"
    };

    bool result = true;
    for (const QString& dir : dirs) {
        auto target_dir_path = base_path + "/" + dir;
        QDir target_dir(target_dir_path);
        if (target_dir.exists()) {
            continue;
        }
        if (!target_dir.mkpath(target_dir_path)) {
            result = false;
            LOGI("Make path failed: {}", target_dir_path.toStdString());
        }
    }
    return result;
}

int main(int argc, char** argv) {
#ifdef WIN32
    CaptureDump();
#endif

#ifdef __APPLE__
    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(3, 3);
    fmt.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(fmt);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif
    //QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QSurfaceFormat myFormat;
    myFormat.setDepthBufferSize(24);
    myFormat.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(myFormat);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication app(argc, argv);
    ParseCommandLine(app);

    tcFontMgr()->InitFont(":/resources/font/ms_yahei.ttf");

    PrepareDirs(app.applicationDirPath());

    auto settings = tc::Settings::Instance();
    auto host = g_remote_host_;
    auto port = g_remote_port_;
    if (host.empty() || port <= 0 || port >= 65535) {
        auto msg_box = SizedMessageBox::MakeOkBox("Error Params", "You must give valid (HOST & PORT) or Remote device ID.");
        msg_box->exec();
        return -1;
    }

    // init language
    tcTrMgr()->InitLanguage((LanguageKind)settings->language_);

    if (false) {
#ifdef WIN32
        MessageBox(0,0,0,0);
#endif
    }

    auto name = [=]() -> std::string {
        if (settings->network_type_ == ClientNetworkType::kRelay || settings->network_type_ == ClientNetworkType::kWebRtc) {
            return settings->remote_device_id_;
        }
        else {
            return settings->host_;
        }
    } ();
    auto ctx = std::make_shared<ClientContext>(name);
    ctx->Init(true);

    LOGI("host: {}", g_remote_host_);
    LOGI("port: {}", g_remote_port_);
    LOGI("audio on: {}", settings->audio_on_);
    LOGI("clipboard on: {}", settings->clipboard_on_);
    LOGI("device id: {}", settings->device_id_);
    LOGI("device rdm pwd: {}", settings->device_random_pwd_);
    LOGI("remote device id: {}", settings->remote_device_id_);
    LOGI("remote device rdm pwd: {}", settings->remote_device_random_pwd_);
    LOGI("stream id: {}", settings->stream_id_);
    LOGI("network type: {} => {}", g_nt_type_, (int)settings->network_type_);
    LOGI("show max window: {}", (int)settings->show_max_window_);
    LOGI("enable p2p: {}", (int)settings->enable_p2p_);
    LOGI("display name: {}", settings->display_name_);
    LOGI("display remote name: {}", settings->display_remote_name_);
    LOGI("panel server port: {}", settings->panel_server_port_);
    LOGI("screen recording path: {}", settings->screen_recording_path_);
    LOGI("my host: {}", settings->my_host_);
    LOGI("only viewing: {}", settings->only_viewing_);
    LOGI("split windows: {}", settings->split_windows_);

    // WebSocket only
    auto bare_remote_device_id = settings->remote_device_id_.empty() ? g_remote_host_ : settings->remote_device_id_;
    auto visitor_device_id = settings->device_id_.empty() ? settings->my_host_ : settings->device_id_;
    auto media_path = std::format("/media?only_audio=0&remote_device_id={}&stream_id={}&visitor_device_id={}",
                                  bare_remote_device_id, settings->stream_id_, visitor_device_id);
    auto ft_path = std::format("/file/transfer?remote_device_id={}&stream_id={}&visitor_device_id={}",
                                  bare_remote_device_id, settings->stream_id_, visitor_device_id);
    auto target_device_id = settings->device_id_.empty() ? settings->my_host_ : settings->device_id_;
    auto device_id = "client_" + target_device_id + "_" + MD5::Hex(settings->remote_device_id_);
    settings->full_device_id_ = device_id;
    auto remote_device_id = "server_" + settings->remote_device_id_;
    settings->full_remote_device_id_ = remote_device_id;
    auto ft_device_id = "ft_" + device_id;
    auto ft_remote_device_id = "ft_" + remote_device_id;

    LOGI("full device id: {}", settings->full_device_id_);
    LOGI("full remote device id: {}", settings->full_remote_device_id_);

    auto client_type = []() {
#if defined(WIN32)
        return ClientType::kWindows;
#elif defined(ANDROID)
        return ClientType::kAndroid;
#endif
    } ();

    auto params = std::make_shared<ThunderSdkParams>(ThunderSdkParams {
        .ssl_ = false,
        .enable_audio_ = true,
        .enable_video_ = true,
        .enable_controller_ = false,
        .ip_ = host,
        .port_ = port,
        .media_path_ = media_path,
        .ft_path_ = ft_path,
        .client_type_ = client_type,
        .nt_type_ = settings->network_type_,
        .bare_device_id_ = settings->device_id_,
        .bare_remote_device_id_ = settings->remote_device_id_,
        .device_id_ = device_id,
        .remote_device_id_ = remote_device_id,
        .ft_device_id_ = ft_device_id,
        .ft_remote_device_id_ = ft_remote_device_id,
        .stream_id_ = settings->stream_id_,
        .stream_name_ = settings->stream_name_,
        .enable_p2p_ = settings->enable_p2p_,
        .display_name_ = settings->display_name_,
        .display_remote_name_ = settings->display_remote_name_,
        .language_id_ = settings->language_,
    });

    auto beg = TimeUtil::GetCurrentTimestamp();

    static auto ws = std::make_shared<Workspace>(ctx, params);
    ws->Init();
    ws->show();
    ctx->PostDelayUITask([=]() {
        if (settings->show_max_window_) {
            ws->showMaximized();
        }
    }, 100);
    auto end = TimeUtil::GetCurrentTimestamp();
    LOGI("Init used: {}ms", (end-beg));

    HHOOK keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, [](int code, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (Settings::Instance()->only_viewing_) {
            return CallNextHookEx(nullptr, code, wParam, lParam);
        }

        auto kbd_struct = (KBDLLHOOKSTRUCT *)lParam;
        if (code >= 0 && ws->IsActiveNow()) {
            bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
            if (kbd_struct->vkCode == VK_LWIN || kbd_struct->vkCode == VK_RWIN || kbd_struct->vkCode == VK_LMENU || kbd_struct->vkCode == VK_RMENU) {
                ws->SendWindowsKey(kbd_struct->vkCode, down);
                return 1; // ignore it , send to remote
            }

            // Tab was sent in video_widget_event.cpp, and the ALT + TAB are pressed together, sending the TAB here.
            if (kbd_struct->vkCode == VK_TAB && (GetKeyState(VK_LMENU) < 0 || GetKeyState(VK_RMENU) < 0)) {
                ws->SendWindowsKey(kbd_struct->vkCode, down);
                return 1;
            }
        }
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }, nullptr, 0);

    auto r = app.exec();
    UnhookWindowsHookEx(keyboardHook);
    return r;
}