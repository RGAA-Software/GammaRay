//
// Created by RGAA on 2023/8/14.
//

#ifndef SAILFISH_CLIENT_PC_APPMENU_H
#define SAILFISH_CLIENT_PC_APPMENU_H

#include <QWidget>
#include <QString>
#include <QLabel>

#include <memory>
#include <functional>

namespace tc
{

    using OnItemClickedCallback = std::function<void(const QString& name, int idx)>;

    class AppMenuItem : public QWidget {
    public:

        AppMenuItem(const QString& name, int idx, const QString& icon, QWidget* parent = nullptr);
        ~AppMenuItem() override;

        void SetOnItemClickedCallback(const OnItemClickedCallback& cbk);
        void Select();
        void UnSelect();
        QString GetName();
        bool IsSelected();

    protected:
        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

    private:

        bool entered_ = false;
        bool pressed_ = false;
        bool selected_ = false;

        float round_radius_ = 6.0f;

        QString name_;
        int idx_;
        OnItemClickedCallback callback_;

        QLabel* icon_ = nullptr;
        QLabel* text_ = nullptr;
    };

    class AppItemDesc {
    public:
        QString name_;
        QString url_;
    };

    class AppMenu : public QWidget {
    public:

        explicit AppMenu(const std::vector<AppItemDesc>& items, QWidget* parent = nullptr);
        ~AppMenu() override;

        void SetOnItemClickedCallback(OnItemClickedCallback cbk);

        void paintEvent(QPaintEvent *event) override;

    private:

        void CreateLayout();

    private:

        OnItemClickedCallback callback_;
        std::vector<AppMenuItem*> app_items;

    };

}

#endif //SAILFISH_CLIENT_PC_APPMENU_H
