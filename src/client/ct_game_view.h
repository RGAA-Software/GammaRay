#pragma once

#include <QWidget>
#include <QLabel>
#include <qevent.h>
#include "thunder_sdk.h"

#define TEST_SDL 0

namespace tc
{

    class OpenGLVideoWidget;
    class SDLVideoWidget;
    class D3D11VideoWidget;
    class VideoWidget;
    class ClientContext;
    class ThunderSdk;
    class FloatController;
    class FloatControllerPanel;
    class MessageListener;
    class SvgLable;
    class Settings;
    class Thread;
    class MediaRecordSignLab;

    class GameView : public QWidget {
    public:
        GameView(const std::shared_ptr<ClientContext>& ctx, std::shared_ptr<ThunderSdk>& sdk, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent);
        ~GameView() override;
        void resizeEvent(QResizeEvent* event) override;
        void enterEvent(QEnterEvent* event) override;
        void leaveEvent(QEvent* event) override;
        bool eventFilter(QObject* watched, QEvent* event) override;
        bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
        void showEvent(QShowEvent* event) override;
        void RefreshCapturedMonitorInfo(const SdkCaptureMonitorInfo& mon_info);
        void RefreshImage(const std::shared_ptr<RawImage>& image);
        void RefreshI420Image(const std::shared_ptr<RawImage>& image);
        void RefreshI444Image(const std::shared_ptr<RawImage>& image);
        void SendKeyEvent(quint32 vk, bool down);
        void SetActiveStatus(bool active);
        bool GetActiveStatus() const;
        void SetMonitorName(const std::string& mon_name);
        void SwitchToFullWindow();
        void CalculateAspectRatio();
        void SetMainView(bool main_view);
        bool IsMainView() const;
        void SnapshotStream();
        HWND GetVideoHwnd();
        std::string GetRenderTypeName();
    public:
        static bool s_mouse_in_;

    private:
        Settings* settings_ = nullptr;
        VideoWidget* video_widget_ = nullptr;
    #if TEST_SDL
        SDLVideoWidget* sdl_video_widget_ = nullptr;
    #endif
    #if TEST_D3D11
        //VideoWidget* d3d11_video_widget_ = nullptr;
    #endif
        std::shared_ptr<ClientContext> ctx_ = nullptr;
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        std::shared_ptr<ThunderSdkParams> params_;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::string monitor_name_;
        bool active_ = false;
        bool is_main_view_ = false;

        FloatController* float_controller_ = nullptr;
        FloatControllerPanel* controller_panel_ = nullptr;

        MediaRecordSignLab* recording_sign_lab_ = nullptr;

        bool need_recalculate_aspect_ = true;

        std::shared_ptr<Thread> thread_ = nullptr;

    private:
        void InitFloatController();
        void RegisterControllerPanelListeners();
    };

}