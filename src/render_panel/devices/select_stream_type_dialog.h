//
// Created by RGAA on 19/05/2025.
//

#ifndef GAMMARAY_SELECT_STREAM_TYPE_DIALOG_H
#define GAMMARAY_SELECT_STREAM_TYPE_DIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QPainter>

#include "render_panel/database/stream_item.h"
#include "tc_qt_widget/tc_custom_titlebar_dialog.h"

namespace tc
{

    class GrContext;

    class SelectStreamTypeDialog : public TcCustomTitleBarDialog {
    public:
        explicit SelectStreamTypeDialog(const std::shared_ptr<GrContext>& ctx, QWidget* parent = nullptr);

    private:
    };

}

#endif //GAMMARAY_SELECT_STREAM_TYPE_DIALOG_H
