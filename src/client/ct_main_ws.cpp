//
// Created by RGAA on 2023-12-26.
//

#include <QApplication>
#include <QSurfaceFormat>
#include <QFontDatabase>
#include <QMessageBox>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QOpenGLWidget>
#include "thunder_sdk.h"
#include "client/ct_client_context.h"
#include "client/ct_workspace.h"
#include "client/ct_application.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/log.h"
#include "tc_common_new/base64.h"
#include "client/ct_settings.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <QDir>
#include <memory>
#include "tc_qt_widget/sized_msg_box.h"
#include "tc_qt_widget/tc_font_manager.h"
#include "translator/tc_translator.h"
#include "ct_stream_item_net_type.h"
#include "tc_common_new/dump_helper.h"
#include "tc_common_new/time_util.h"

using namespace tc;

std::string g_host_;
int g_port_ = 0;
std::string g_nt_type_;
std::string g_conn_type_;

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

    parser.process(app);

    g_host_ = parser.value(opt_host).toStdString();
    g_port_ = parser.value(opt_port).toInt();

    auto settings = tc::Settings::Instance();
    settings->remote_address_ = g_host_;
    auto audio_on = parser.value(opt_audio).toInt();
    settings->audio_on_ = (audio_on == 1);

    auto clipboard_on = parser.value(opt_clipboard).toInt();
    settings->clipboard_on_ = (clipboard_on == 1);
    settings->ignore_mouse_event_ = parser.value(opt_ignore_mouse).toInt() == 1;
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

    // init language
    tcTrMgr()->InitLanguage(LanguageKind::kEnglish);

    auto host = g_host_;
    auto port = g_port_;
    if (host.empty() || port <= 0 || port >= 65535) {
        auto msg_box = SizedMessageBox::MakeOkBox("Error Params", "You must give valid HOST & PORT");
        msg_box->exec();
        return -1;
    }

    if (false) {
#ifdef WIN32
        MessageBox(0,0,0,0);
#endif
    }

    auto settings = tc::Settings::Instance();
    auto name = [=]() -> std::string {
        if (settings->network_type_ == ClientNetworkType::kRelay || settings->network_type_ == ClientNetworkType::kWebRtc) {
            return settings->remote_device_id_;
        }
        else {
            return settings->remote_address_;
        }
    } ();
    auto ctx = std::make_shared<ClientContext>(name);
    ctx->Init(true);

    LOGI("host: {}", g_host_);
    LOGI("port: {}", g_port_);
    LOGI("audio on: {}", settings->audio_on_);
    LOGI("clipboard on: {}", settings->clipboard_on_);
    LOGI("ignore mouse event: {}", settings->ignore_mouse_event_);
    LOGI("device id: {}", settings->device_id_);
    LOGI("device rdm pwd: {}", settings->device_random_pwd_);
    LOGI("remote device id: {}", settings->remote_device_id_);
    LOGI("remote device rdm pwd: {}", settings->remote_device_random_pwd_);
    LOGI("stream id: {}", settings->stream_id_);
    LOGI("network type: {} => {}", g_nt_type_, (int)settings->network_type_);
    //LOGI("connection type: {} => {}", g_conn_type_, (int)settings->conn_type_);
    LOGI("show max window: {}", (int)settings->show_max_window_);
    LOGI("enable p2p: {}", (int)settings->enable_p2p_);
    LOGI("display name: {}", settings->display_name_);
    LOGI("display remote name: {}", settings->display_remote_name_);

    // WebSocket only
    auto media_path = std::format("/media?only_audio=0&device_id={}&stream_id={}",
                                settings->device_id_, settings->stream_id_);
    auto ft_path = std::format("/file/transfer?device_id={}&stream_id={}",
                                  settings->device_id_, settings->stream_id_);
    auto device_id = "client_" + settings->device_id_ + "_" + MD5::Hex(settings->remote_device_id_);
    auto remote_device_id = "server_" + settings->remote_device_id_;
    auto ft_device_id = "ft_" + device_id;
    auto ft_remote_device_id = "ft_" + remote_device_id;

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
    });

    auto beg = TimeUtil::GetCurrentTimestamp();
    static auto ws = std::make_shared<Workspace>(ctx, params);
    ws->Init();
    ws->resize(1366, 768);
    if (settings->show_max_window_) {
        ws->showMaximized();
    }
    ws->show();
    auto end = TimeUtil::GetCurrentTimestamp();
    LOGI("Init used: {}ms", (end-beg));

    HHOOK keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, [](int code, WPARAM wParam, LPARAM lParam) -> LRESULT {
        auto kbd_struct = (KBDLLHOOKSTRUCT *)lParam;
        if (code >= 0 && ws->IsActiveNow()) {
            bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
            if (kbd_struct->vkCode == VK_LWIN || kbd_struct->vkCode == VK_RWIN) {
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