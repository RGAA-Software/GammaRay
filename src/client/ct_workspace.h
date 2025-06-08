//
// Created by RGAA on 2023-12-27.
//

#ifndef TC_CLIENT_PC_WORKSPACE_H
#define TC_CLIENT_PC_WORKSPACE_H

#include <QWidget>
#include <QMainWindow>
#include <QLibrary>
#include <map>
#include <vector>
#include <qlist.h>
#include "thunder_sdk.h"
#include "theme/QtAdvancedStylesheet.h"
#include "client/ct_app_message.h"
#include "ct_base_workspace.h"

namespace tc
{
    /*
    1. 多屏分离显示的功能，抽象到 ct_workspace.cpp里面了， ct_base_workspace.cpp 就是普通的 多屏切换
    2. 以后有不想开源的，又不适合做成插件的 高级功能，就加到  ct_workspace.cpp里面，通用的基础功能 就加到  ct_base_workspace.cpp 
    */

    class Workspace : public BaseWorkspace {
    public:

        explicit Workspace(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdkParams>& params, QWidget* parent = nullptr);
        ~Workspace() override;

        void Init() override;
        bool eventFilter(QObject* watched, QEvent* event) override;
        void SendWindowsKey(unsigned long vk, bool down) override;
    protected:
        void InitGameView(const std::shared_ptr<ThunderSdkParams>& params) override;
        void InitListener() override;
        void RegisterBaseListeners() override;
    private:
        void RegisterSdkMsgCallbacks() override;
        void CalculateAspectRatio() override;
        void SwitchToFullWindow() override;
        void UpdateGameViewsStatus() override;
        void OnGetCaptureMonitorsCount(int monitors_count) override;
        void OnGetCaptureMonitorName(std::string monitor_name) override;
    private:
        // 扩展屏
        // to do:
        // 1.将gameview这个类抽象的比较完善, 后面都可以使用 game_views_ 来表示，主屏 与 扩展屏 放在一个vector里 逻辑清晰
        // 2.可能连上对端后，对端设置的显示器采集模式 就是 all，那么客户端默认 kTab 就不合理了 (已完成)
        // 3.70这台电脑,如果dda采集失败了，切换到GDI采集，研究下，GDI采集是否能单独采集某个屏幕
        // 4.game_view 产生的位置要错开(已完成)
        // 5.现在是每秒同步一次显示器信息,导致关闭掉的扩展屏 会再次显示，要改为监听win消息的方式，有变化再通知(已完成)
        // 6.屏幕切换的图标数量 (已完成)
        // 7.获取任一game_view的关闭事件(已完成)
        // 8.每个game_view的标题名字(已完成)
        // 9.切换屏幕按钮，以及分屏合并操作，要刷新桌面 (已完成)
        // 10.关闭弹窗显示在 点击关闭按钮所在的屏幕上(已完成)
        // 11.全屏动作，扩展屏未生效 (已完成)
        // 12.关闭的时候，将widget显示到前面,尤其是在任务栏关闭(已完成)
        // 13.点击文件传输按钮, 文件传输页面显示到前面(已完成)
        // 14.setting 保存各种模式的时候，不需要重复保存，避免重复保存
        // 15.测试一开始 又出现没有当前显示器标识的情况
        // 16.分辨率还要传递回来 当前真正使用的分辨率
        // 17.全屏的时候，多屏之间 要同步全屏或者退出全屏(已完成)
        // 18.不知道为什么会闪烁(已解决)
        // 19.如果对端第一次启动,好像配置信息没有正确传递过来
        // 20.文件传输加接口 控制速度等
        // 21.game_view上的小球，点击后，会full window,需要找下原因(已完成)
        // 22.刷新 参考 vnc
        // 23.流路一开始就执行了UpdateVideoWidgetSize，但流路画面还是没有铺满widget
        //QString origin_title_name_;
        std::vector<GameView*> game_views_;  
  
        EMultiMonDisplayMode multi_display_mode_ = EMultiMonDisplayMode::kTab;     
    private:
        void ListenMultiMonDisplayModeMessage();
    };

}

#endif //TC_CLIENT_PC_WORKSPACE_H
