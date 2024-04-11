#ifndef VIDEONOTEUIDECORATOR_H
#define VIDEONOTEUIDECORATOR_H

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QCheckBox>

namespace tc
{
    class WidgetDecorator {
    public:
        WidgetDecorator();


        static void
        DecorateSearchBar(QLineEdit *search, QPushButton *btn_search, QPushButton *btn_clear, const QString &msg = "");

        static void DecorateListWidget(QListWidget *list);

        static void DecorateBottomBar();

        static void DecoratePopMenu(QMenu *menu);

        static void DecorateIndexBtn(QPushButton *btn);

        static void DecorateDialogBtn(QPushButton *sure, QPushButton *cancel);

        static void DecorateMenuBar(QMenuBar *mb);

        static void DecoratePropertyEdit(QLineEdit *edit);

        static void DecorateParticleIcon(QPushButton *btn, int size = 0);

        static void DecorateLineEdit(QLineEdit *edit, int margin_left, int margin_right);

        static void DecorateLabel(QLabel *label, int margin_left, int margin_right, int font_size);

        static void DecorateComboBox(QComboBox *box, int height);

        static void DecorateCheckBox(QCheckBox *box, int size = 14);

    };
}
#endif // VIDEONOTEUIDECORATOR_H
