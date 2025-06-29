#include "file_transmit_log_widget.h"
#include <qtextedit.h>
#include <qscrollbar.h>
#include <qstyleoption.h>
#include <qboxlayout.h>
#include "file_log_manager.h"
#include "file_table_view_style.h"


namespace tc {

static QString s_text_style = R"(
QTextEdit {font-size: %1px; font-family: Microsoft YaHei; color: #333333;line-height: %2px; border: 0px;background-color:#ffffff;}
)";

static QString s_widget_style = R"(
"QWidget {background-color:#F5F5F5;}"
)";

FileTransmitLogWidget::FileTransmitLogWidget(QWidget* parent) : QWidget(parent) {
	Init();
	connect(FileLogManager::Instance(), &FileLogManager::SigLog, this, [=](QString log_text) {
		text_edit_->append(log_text);
		});
}
FileTransmitLogWidget::~FileTransmitLogWidget() {

}

void FileTransmitLogWidget::paintEvent(QPaintEvent* qpevent) {
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.save();
	painter.setBrush(QColor(0xed, 0xf0, 0xf6));
	painter.drawRoundedRect(0, 0, this->width(), this->height(), 6, 6);
	painter.restore();
}

void FileTransmitLogWidget::Init() {
	setAttribute(Qt::WA_StyledBackground);
	setStyleSheet(s_widget_style);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	s_text_style = s_text_style.arg(11).arg(13);

	main_vbox_layout_ = new QVBoxLayout();
	main_vbox_layout_->setContentsMargins(6, 6, 6, 6);
	main_vbox_layout_->setSpacing(0);
	setLayout(main_vbox_layout_);
	text_edit_ = new QTextEdit(this);
	text_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	text_edit_->setReadOnly(true);
	text_edit_->setWordWrapMode(QTextOption::WrapAnywhere);
	text_edit_->verticalScrollBar()->setStyleSheet(QString::fromStdString(kScrollBarStyle));
	text_edit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	text_edit_->setStyleSheet(s_text_style);
	main_vbox_layout_->addWidget(text_edit_);
}

}