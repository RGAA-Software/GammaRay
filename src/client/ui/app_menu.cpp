//
// Created by RGAA on 2023/8/14.
//

#include "app_menu.h"

#include <utility>
#include <QVBoxLayout>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <format>

#include "app_color_theme.h"

namespace tc
{

    AppMenuItem::AppMenuItem(const QString& name, int idx, const QString& icon, QWidget* parent) : QWidget(parent) {
        name_ = name;
        idx_ = idx;

        auto inner_layout = new QHBoxLayout();
        inner_layout->setSpacing(0);
        inner_layout->setContentsMargins(0,0,0,0);
        icon_ = new QLabel(this);
        icon_->setFixedSize(QSize(20, 20));
        icon_->setStyleSheet(std::format("background-image: url({});", icon.toStdString()).c_str());
        text_ = new QLabel(this);
        text_->setText(name);
        text_->setStyleSheet("color:#FFFFFF;font-weight:bold;");

        inner_layout->addSpacing(18);
        inner_layout->addWidget(icon_);
        inner_layout->addSpacing(10);
        inner_layout->addWidget(text_);

        inner_layout->addStretch();

        setLayout(inner_layout);
    }

    AppMenuItem::~AppMenuItem() {

    }

    void AppMenuItem::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setRenderHint(QPainter::RenderHint::Antialiasing);
        int target_color = AppColorTheme::kAppMenuItemBgNormalColor;
        if (pressed_ || selected_) {
            target_color = AppColorTheme::kAppMenuItemBgPressColor;
        }
        else if (entered_) {
            target_color = AppColorTheme::kAppMenuItemBgHoverColor;
        }
        round_radius_ = this->height()/2;
        painter.setBrush(QBrush(QColor(target_color)));
        painter.drawRoundedRect(0, 0, this->width(), this->height(), round_radius_, round_radius_);

        QWidget::paintEvent(event);
    }

    void AppMenuItem::enterEvent(QEnterEvent *event) {
        QWidget::enterEvent(event);
        entered_ = true;
        update();
    }

    void AppMenuItem::leaveEvent(QEvent *event) {
        QWidget::leaveEvent(event);
        entered_ = false;
        update();
    }

    void AppMenuItem::mousePressEvent(QMouseEvent *event) {
        QWidget::mousePressEvent(event);
        pressed_ = true;
        update();
    }

    void AppMenuItem::mouseReleaseEvent(QMouseEvent *event) {
        QWidget::mouseReleaseEvent(event);
        pressed_ = false;
        selected_ = true;
        update();

        if (callback_) {
            callback_(name_, idx_);
        }
    }

    void AppMenuItem::SetOnItemClickedCallback(const OnItemClickedCallback& cbk) {
        callback_ = cbk;
    }

    void AppMenuItem::Select() {
        selected_ = true;
        update();
    }

    void AppMenuItem::UnSelect() {
        selected_ = false;
        update();
    }

    QString AppMenuItem::GetName() {
        return name_;
    }

    bool AppMenuItem::IsSelected() {
        return selected_;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

    AppMenu::AppMenu(const std::vector<AppItemDesc>& items, QWidget* parent) : QWidget(parent) {
        setFixedWidth(230);

        auto root_layout = new QHBoxLayout();
        root_layout->setSpacing(0);
        root_layout->setContentsMargins(0,0,0,0);
        root_layout->addStretch();

        auto item_layout = new QVBoxLayout();
        item_layout->setAlignment(Qt::AlignHCenter);
        item_layout->setSpacing(0);
        item_layout->setContentsMargins(0,0,0,0);
        item_layout->addSpacing(35);

        auto logo_layout = new QHBoxLayout();
        auto logo = new QLabel(this);
        auto image = new QImage(":resources/image/tc_icon.png");
        auto pixmap = QPixmap::fromImage(*image);
        pixmap = pixmap.scaled(pixmap.width()/2.5, pixmap.height()/2.5, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        logo->setPixmap(pixmap);

        logo_layout->addStretch();
        logo_layout->addWidget(logo);
        logo_layout->addStretch();

        item_layout->addLayout(logo_layout);
        item_layout->addSpacing(35);

        int idx = 0;
        for (const auto& item : items) {

            auto app_item = new AppMenuItem(item.name_, idx++, item.url_, this);
            if (idx == 1) {
                app_item->Select();
            }

            app_items.push_back(app_item);
            app_item->setFixedSize(180, 40);
            app_item->SetOnItemClickedCallback([=, this](const QString& name, int idx) {
                if (callback_) {
                    callback_(name, idx);
                }

                for (const auto& it : app_items) {
                    if (it->GetName() != name) {
                        it->UnSelect();
                    }
                }
            });
            item_layout->addWidget(app_item);
            item_layout->addSpacing(10);
        }
        item_layout->addStretch();

        root_layout->addLayout(item_layout);
        root_layout->addStretch();

        setLayout(root_layout);
    }

    AppMenu::~AppMenu() {

    }

    void AppMenu::paintEvent(QPaintEvent *event) {
        QWidget::paintEvent(event);

        //QPainter painter(this);
        //painter.setPen(Qt::NoPen);
        //painter.setBrush(QBrush(QColor(0xFFFFFF)));
        //painter.drawRect(0, 0, QWidget::width(), QWidget::height());
    }

    void AppMenu::SetOnItemClickedCallback(OnItemClickedCallback cbk) {
        callback_ = std::move(cbk);
    }

    void AppMenu::CreateLayout() {

    }

}