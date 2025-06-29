#include "file_popup_widgets.h"
#include <mutex>
#include <iostream>
#include <qsvgrenderer.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qboxlayout.h>
#include "tc_label.h"

namespace tc {

static QString s_lab_title_style = R"(
QLabel {font-size: %1px; font-family: Microsoft YaHei; color: #333333;line-height: %2px;}
)";

static QString s_lab_content_style = R"(
QLabel {font-size: %1px; font-family: Microsoft YaHei; color: #333333;line-height: %2px;}
)";

static QString s_btn_close_style = R"(
QPushButton{border:0px; background-color:#ffffff; image:url(:/resource/close_normal.svg);}
QPushButton::hover{border:0px; background-color:#bfbfbf; image:url(:/resource/close_hover.svg);} 
QPushButton::pressed{border:0px; background-color:#999999; image:url(:/resource/close_press.svg);}
)";

static QString s_btn_warning_style = R"(
QPushButton{border:0px; background-color:#ffffff; image:url(:/resource/file_exists_warning.svg);}
)";

static QString s_btn_cover_style = R"(
QPushButton{border:0px; border-radius:6px; background-color:#2979ff; font-size: %1px; font-family: Microsoft YaHei; color: #FFFFFF;}
QPushButton::hover{border:0px; border-radius:6px; background-color:#2059ee; font-size: %2px; font-family: Microsoft YaHei; color: #FFFFFF;}
QPushButton::pressed{border:0px; border-radius:6px; background-color:#1549dd; font-size: %3px; font-family: Microsoft YaHei; color: #FFFFFF;}
)";

FileCoverDialog::FileCoverDialog(QWidget* parent) :QDialog(parent) {
	Init();
	InitSigChannel();
}

void FileCoverDialog::mouseMoveEvent(QMouseEvent* event)
{
	if (pressed_) {
		move(event->pos() - point_ + pos());
	}
	QDialog::mouseMoveEvent(event);
}

void FileCoverDialog::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	pressed_ = false;
	QDialog::mouseReleaseEvent(event);
}

void FileCoverDialog::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		pressed_ = true;
		point_ = event->pos();
	}
	QDialog::mousePressEvent(event);
}

void FileCoverDialog::Init() {
	static std::once_flag flag;
	std::call_once(flag, []() { // init css
		s_lab_title_style = s_lab_title_style.arg(14).arg(20);
		s_lab_content_style = s_lab_content_style.arg(14).arg(20);
		s_btn_cover_style = s_btn_cover_style.arg(13).arg(13).arg(13);
		});
	setFixedSize(580, 246);
	setWindowFlags(Qt::FramelessWindowHint);
	setStyleSheet("QDialog {background-color: #ffffff; border-radius:6px; border: 1px solid #666666;}");
	setAttribute(Qt::WA_StyledBackground);

	main_vbox_layout_ = new QVBoxLayout();
	main_vbox_layout_->setContentsMargins(0,0,0,0);
	main_vbox_layout_->setSpacing(0);
	setLayout(main_vbox_layout_);
	// 标题栏 关闭按钮
	title_hbox_layout_ = new QHBoxLayout();
	title_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	title_hbox_layout_->setSpacing(0);
	lab_title_ = new QLabel(this);
	lab_title_->setFixedSize(60, 20);
	//lab_title_->setText(QStringLiteral("上传提示"));
	lab_title_->setText(tcTr("id_file_trans_upload_prompt"));
	lab_title_->setStyleSheet(s_lab_title_style);

	btn_close_ = new QPushButton(this);
	btn_close_->setFixedSize(16, 16);
	btn_close_->setStyleSheet(s_btn_close_style);
	title_hbox_layout_->addSpacing(12);
	title_hbox_layout_->addWidget(lab_title_);
	title_hbox_layout_->addStretch();
	title_hbox_layout_->addWidget(btn_close_);
	title_hbox_layout_->addSpacing(12);
	main_vbox_layout_->addSpacing(8);
	main_vbox_layout_->addLayout(title_hbox_layout_);

	//文件已存在
	hint_hbox_layout_ = new QHBoxLayout();
	hint_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	hint_hbox_layout_->setSpacing(0);

	btn_icon_warning_ = new QPushButton(this);
	btn_icon_warning_->setFixedSize(24, 24);
	btn_icon_warning_->setStyleSheet(s_btn_warning_style);
	lab_hint_text_ = new QLabel(this);
	lab_hint_text_->setFixedSize(70, 20);
	lab_hint_text_->setStyleSheet(s_lab_title_style);
	//lab_hint_text_->setText(QStringLiteral("文件已存在"));
	lab_hint_text_->setText(tcTr("id_file_trans_file_already_exist"));
	hint_hbox_layout_->addSpacing(22);
	hint_hbox_layout_->addWidget(btn_icon_warning_);
	hint_hbox_layout_->addSpacing(12);
	hint_hbox_layout_->addWidget(lab_hint_text_);
	hint_hbox_layout_->addStretch();
	main_vbox_layout_->addSpacing(20);
	main_vbox_layout_->addLayout(hint_hbox_layout_);

	// 文件名称
	file_name_hbox_layout_ = new QHBoxLayout();
	file_name_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	file_name_hbox_layout_->setSpacing(0);
	lab_file_name_ = new QLabel(this);
	lab_file_name_->setFixedHeight(20);
	lab_file_name_->setStyleSheet(s_lab_title_style);
	//lab_file_name_->setText(QStringLiteral("文件名称"));
	lab_file_name_->setText(tcTr("id_file_trans_file_name"));
	lab_file_name_->setAlignment(Qt::AlignLeft);
	lab_file_name_->adjustSize();

	file_name_hbox_layout_->addSpacing(22);
	file_name_hbox_layout_->addWidget(lab_file_name_);
	file_name_hbox_layout_->addStretch();
	main_vbox_layout_->addSpacing(20);
	main_vbox_layout_->addLayout(file_name_hbox_layout_);

	// 文件信息
	local_file_info_layout_ = new QHBoxLayout();
	local_file_info_layout_->setContentsMargins(0, 0, 0, 0);
	local_file_info_layout_->setSpacing(0);

	lab_local_file_name_ = new QLabel(this);
	lab_local_file_name_->setFixedSize(250, 20);
	//lab_local_file_name_->setText(QStringLiteral("文件名称"));
	lab_local_file_name_->setText(tcTr("id_file_trans_file_name"));
	lab_local_file_name_->setStyleSheet(s_lab_content_style);
	lab_local_file_name_->setAlignment(Qt::AlignLeft);

	lab_local_file_size_ = new QLabel(this);
	lab_local_file_size_->setFixedSize(80, 20);
	lab_local_file_size_->setText(QStringLiteral("1000MB"));
	lab_local_file_size_->setStyleSheet(s_lab_content_style);

	lab_local_file_place_ = new QLabel(this);
	lab_local_file_place_->setFixedSize(30, 20);
	//lab_local_file_place_->setText(QStringLiteral("本地"));
	lab_local_file_place_->setText(tcTr("id_file_trans_local"));
	lab_local_file_place_->setStyleSheet(s_lab_content_style);

	lab_local_file_time_ = new QLabel(this);
	lab_local_file_time_->setFixedSize(140, 20);
	lab_local_file_time_->setText(QStringLiteral("2024-02-01 13:15"));
	lab_local_file_time_->setStyleSheet(s_lab_content_style);

	local_file_info_layout_->addSpacing(20);
	local_file_info_layout_->addWidget(lab_local_file_name_, 5);
	local_file_info_layout_->addSpacing(12);
	local_file_info_layout_->addWidget(lab_local_file_size_, 2);
	local_file_info_layout_->addSpacing(12);
	local_file_info_layout_->addWidget(lab_local_file_place_, 1);
	local_file_info_layout_->addSpacing(12);
	local_file_info_layout_->addWidget(lab_local_file_time_, 3);
	local_file_info_layout_->addSpacing(12);

	main_vbox_layout_->addSpacing(18);
	main_vbox_layout_->addLayout(local_file_info_layout_);

	remote_file_info_layout_ = new QHBoxLayout();
	remote_file_info_layout_->setContentsMargins(0, 0, 0, 0);
	remote_file_info_layout_->setSpacing(0);

	lab_remote_file_name_ = new QLabel(this);
	lab_remote_file_name_->setFixedSize(250, 20);
	//lab_remote_file_name_->setText(QStringLiteral("文件名称"));
	lab_remote_file_name_->setText(tcTr("id_file_trans_file_name"));
	lab_remote_file_name_->setStyleSheet(s_lab_content_style);
	lab_remote_file_name_->setAlignment(Qt::AlignLeft);

	lab_remote_file_size_ = new QLabel(this);
	lab_remote_file_size_->setFixedSize(80, 20);
	lab_remote_file_size_->setText(QStringLiteral(""));
	lab_remote_file_size_->setStyleSheet(s_lab_content_style);

	lab_remote_file_place_ = new QLabel(this);
	lab_remote_file_place_->setFixedSize(30, 20);
	//lab_remote_file_place_->setText(QStringLiteral("远端"));
	lab_remote_file_place_->setText(tcTr("id_file_trans_remote"));
	lab_remote_file_place_->setStyleSheet(s_lab_content_style);

	lab_remote_file_time_ = new QLabel(this);
	lab_remote_file_time_->setFixedSize(140, 20);
	lab_remote_file_time_->setText(QStringLiteral("2024-02-01 13:15"));
	lab_remote_file_time_->setStyleSheet(s_lab_content_style);

	remote_file_info_layout_->addSpacing(20);
	remote_file_info_layout_->addWidget(lab_remote_file_name_, 5);
	remote_file_info_layout_->addSpacing(12);
	remote_file_info_layout_->addWidget(lab_remote_file_size_, 2);
	remote_file_info_layout_->addSpacing(12);
	remote_file_info_layout_->addWidget(lab_remote_file_place_, 1);
	remote_file_info_layout_->addSpacing(12);
	remote_file_info_layout_->addWidget(lab_remote_file_time_, 3);
	remote_file_info_layout_->addSpacing(12);

	main_vbox_layout_->addSpacing(8);
	main_vbox_layout_->addLayout(remote_file_info_layout_);

	// 相关操作
	operation_hbox_layout_ = new QHBoxLayout();
	operation_hbox_layout_->setContentsMargins(0, 0, 0, 0);
	operation_hbox_layout_->setSpacing(0);

	btn_cover_ = new QPushButton(this);
	btn_cover_->setFixedSize(96, 30);
	btn_cover_->setStyleSheet(s_btn_cover_style);
	//btn_cover_->setText(QStringLiteral("覆盖"));
	btn_cover_->setText(tcTr("id_file_trans_cover"));

	btn_all_cover_ = new QPushButton(this);
	btn_all_cover_->setFixedSize(96, 30);
	btn_all_cover_->setStyleSheet(s_btn_cover_style);
	//btn_all_cover_->setText(QStringLiteral("全部覆盖"));
	btn_all_cover_->setText(tcTr("id_file_trans_overwrite_all"));

	btn_skip_ = new QPushButton(this);
	btn_skip_->setFixedSize(96, 30);
	btn_skip_->setStyleSheet(s_btn_cover_style);
	//btn_skip_->setText(QStringLiteral("跳过"));
	btn_skip_->setText(tcTr("id_file_trans_skip"));

	btn_all_skip_ = new QPushButton(this);
	btn_all_skip_->setFixedSize(96, 30);
	btn_all_skip_->setStyleSheet(s_btn_cover_style);
	//btn_all_skip_->setText(QStringLiteral("全部跳过"));
	btn_all_skip_->setText(tcTr("id_file_trans_skip_all"));

	btn_cancel_ = new QPushButton(this);
	btn_cancel_->setFixedSize(96, 30);
	btn_cancel_->setStyleSheet(s_btn_cover_style);
	//btn_cancel_->setText(QStringLiteral("取消"));
	btn_cancel_->setText(tcTr("id_file_trans_cancel"));

	operation_hbox_layout_->addSpacing(12);
	operation_hbox_layout_->addWidget(btn_cover_);
	operation_hbox_layout_->addStretch();
	operation_hbox_layout_->addWidget(btn_all_cover_);
	operation_hbox_layout_->addStretch();
	operation_hbox_layout_->addWidget(btn_skip_);
	operation_hbox_layout_->addStretch();
	operation_hbox_layout_->addWidget(btn_all_skip_);
	operation_hbox_layout_->addStretch();
	operation_hbox_layout_->addWidget(btn_cancel_);
	operation_hbox_layout_->addSpacing(12);

	main_vbox_layout_->addSpacing(18);
	main_vbox_layout_->addLayout(operation_hbox_layout_);
	main_vbox_layout_->addSpacing(20);
}

void FileCoverDialog::SetData(bool is_download, const QString& file_name, const QString& local_file_size, const QString& local_file_time,
	const QString& remote_file_size, const QString& remote_file_time) {

	if (is_download) {
		//lab_title_->setText(QStringLiteral("下载提示"));
		lab_title_->setText(tcTr("id_file_trans_down_tips"));
	}
	lab_file_name_->setText(file_name);
	lab_local_file_name_->setText(file_name);
	lab_remote_file_name_->setText(file_name);

	lab_local_file_size_->setText(local_file_size);
	lab_remote_file_size_->setText(remote_file_size);

	lab_local_file_time_->setText(local_file_time);
	lab_remote_file_time_->setText(remote_file_time);
}

void FileCoverDialog::InitSigChannel() {
	connect(btn_close_, &QPushButton::clicked, this, [=]() {
		operation_type_ = EOperationType::kCancel;
		done(QDialog::Accepted);
		});

	connect(btn_cover_, &QPushButton::clicked, this, [=]() {
		operation_type_ = EOperationType::kCover;
		done(QDialog::Accepted);
		});

	connect(btn_all_cover_, &QPushButton::clicked, this, [=]() {
		operation_type_ = EOperationType::kAllCover;
		done(QDialog::Accepted);
		});

	connect(btn_skip_, &QPushButton::clicked, this, [=]() {
		operation_type_ = EOperationType::kSkip;
		done(QDialog::Accepted);
		});

	connect(btn_all_skip_, &QPushButton::clicked, this, [=]() {
		operation_type_ = EOperationType::kAllSkip;
		done(QDialog::Accepted);
		});

	connect(btn_cancel_, &QPushButton::clicked, this, [=]() {
		operation_type_ = EOperationType::kCancel;
		done(QDialog::Accepted);
		});
}

}