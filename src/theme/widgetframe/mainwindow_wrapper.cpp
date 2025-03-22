//
// Created by RGAA on 20/03/2025.
//

#include "mainwindow_wrapper.h"
#include "widgets/widgetwindowagent.h"
#include "theme/widgetframe/windowbar.h"
#include "theme/widgetframe/windowbutton.h"
#include "titlebar_messages.h"
#include "tc_common_new/message_notifier.h"
#include <QTimer>

namespace tc
{

    static inline void emulateLeaveEvent(QWidget *widget) {
        Q_ASSERT(widget);
        if (!widget) {
            return;
        }
        QTimer::singleShot(0, widget, [widget]() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
            const QScreen *screen = widget->screen();
#else
            const QScreen *screen = widget->windowHandle()->screen();
#endif
            const QPoint globalPos = QCursor::pos(screen);
            if (!QRect(widget->mapToGlobal(QPoint{0, 0}), widget->size()).contains(globalPos)) {
                QCoreApplication::postEvent(widget, new QEvent(QEvent::Leave));
                if (widget->testAttribute(Qt::WA_Hover)) {
                    const QPoint localPos = widget->mapFromGlobal(globalPos);
                    const QPoint scenePos = widget->window()->mapFromGlobal(globalPos);
                    static constexpr const auto oldPos = QPoint{};
                    const Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
                    const auto event =
                            new QHoverEvent(QEvent::HoverLeave, scenePos, globalPos, oldPos, modifiers);
                    Q_UNUSED(localPos);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
                    const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, globalPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#else
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#endif
                    QCoreApplication::postEvent(widget, event);
                }
            }
        });
    }

    MainWindowWrapper::MainWindowWrapper(const std::shared_ptr<MessageNotifier>& notifier, QMainWindow* window) {
        notifier_ = notifier;
        window_ = window;
    }

    void MainWindowWrapper::Setup(const QString& title) {
        window_->setAttribute(Qt::WA_DontCreateNativeAncestors);
        auto windowAgent = new QWK::WidgetWindowAgent(window_);
        windowAgent->setup(window_);

        auto titleLabel = new QLabel();
        titleLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        titleLabel->setObjectName(QStringLiteral("win-title-label"));
        titleLabel->setText(title);
#ifndef Q_OS_MAC
        auto iconButton = new QWK::WindowButton();
        iconButton->setObjectName(QStringLiteral("icon-button"));
        iconButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        auto btn_avatar = new QWK::WindowButton();
        btn_avatar->setCheckable(true);
        btn_avatar->setObjectName(QStringLiteral("avatar-button"));
        btn_avatar->setProperty("system-button", true);
        btn_avatar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        auto btn_settings = new QWK::WindowButton();
        btn_settings->setCheckable(true);
        btn_settings->setObjectName(QStringLiteral("settings-button"));
        btn_settings->setProperty("system-button", true);
        btn_settings->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        auto pinButton = new QWK::WindowButton();
        pinButton->setCheckable(true);
        pinButton->setObjectName(QStringLiteral("pin-button"));
        pinButton->setProperty("system-button", true);
        pinButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        auto minButton = new QWK::WindowButton();
        minButton->setObjectName(QStringLiteral("min-button"));
        minButton->setProperty("system-button", true);
        minButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        auto maxButton = new QWK::WindowButton();
        maxButton->setCheckable(true);
        maxButton->setObjectName(QStringLiteral("max-button"));
        maxButton->setProperty("system-button", true);
        maxButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        auto closeButton = new QWK::WindowButton();
        closeButton->setObjectName(QStringLiteral("close-button"));
        closeButton->setProperty("system-button", true);
        closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
#endif

        auto windowBar = new QWK::WindowBar();
#ifndef Q_OS_MAC
        windowBar->setIconButton(iconButton);
        windowBar->setAvatarButton(btn_avatar);
        windowBar->setSettingsButton(btn_settings);
        windowBar->setPinButton(pinButton);
        windowBar->setMinButton(minButton);
        windowBar->setMaxButton(maxButton);
        windowBar->setCloseButton(closeButton);
#endif
        //windowBar->setMenuBar(menuBar);
        windowBar->setTitleLabel(titleLabel);
        windowBar->setHostWidget(window_);

        windowAgent->setTitleBar(windowBar);
#ifndef Q_OS_MAC
        windowAgent->setHitTestVisible(pinButton, true);
        windowAgent->setHitTestVisible(btn_avatar, true);
        windowAgent->setHitTestVisible(btn_settings, true);
        windowAgent->setSystemButton(QWK::WindowAgentBase::WindowIcon, iconButton);
        windowAgent->setSystemButton(QWK::WindowAgentBase::Minimize, minButton);
        windowAgent->setSystemButton(QWK::WindowAgentBase::Maximize, maxButton);
        windowAgent->setSystemButton(QWK::WindowAgentBase::Close, closeButton);
#endif
        //windowAgent->setHitTestVisible(menuBar, true);
        window_->setMenuWidget(windowBar);

#ifndef Q_OS_MAC
        QObject::connect(windowBar, &QWK::WindowBar::pinRequested, window_, [this, pinButton](bool pin){
            if (window_->isHidden() || window_->isMinimized() || window_->isMaximized() || window_->isFullScreen()) {
                return;
            }
            window_->setWindowFlag(Qt::WindowStaysOnTopHint, pin);
            window_->show();
            pinButton->setChecked(pin);
        });
        QObject::connect(windowBar, &QWK::WindowBar::minimizeRequested, window_, &QWidget::showMinimized);
        QObject::connect(windowBar, &QWK::WindowBar::maximizeRequested, window_, [this, maxButton](bool max) {
            if (max) {
                window_->showMaximized();
            } else {
                window_->showNormal();
            }

            // It's a Qt issue that if a QAbstractButton::clicked triggers a window's maximization,
            // the button remains to be hovered until the mouse move. As a result, we need to
            // manually send leave events to the button.
            emulateLeaveEvent(maxButton);
        });
        QObject::connect(windowBar, &QWK::WindowBar::closeRequested, window_, &QWidget::close);

        QObject::connect(windowBar, &QWK::WindowBar::settingsRequested, window_, [=, this]() {
            notifier_->SendAppMessage(MsgTitleBarSettingsClicked{});
        });

        QObject::connect(windowBar, &QWK::WindowBar::avatarRequested, window_, [=, this]() {
            notifier_->SendAppMessage(MsgTitleBarAvatarClicked{});
        });

#endif
    }

}