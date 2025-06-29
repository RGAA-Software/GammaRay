#include "file_trans_widget.h"
#include <qfont.h>
#include <qsettings.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qstackedwidget.h>
#include "file_show_widget.h"
#include "file_trans_record.h"
#include "file_transmit_single_task_manager.h"
#include "core/file_sdk_interface.h"
#include "file_transmit_log_widget.h"
#include "file_const_def.h"
#include "file_trans_log_switch_button.h"
#include "core/file_transmit_sdk.h"
#include "tc_common_new/shared_preference.h"
#include "tc_qt_widget/widget_helper.h"
#include "tc_label.h"
// to do : use leveldb save current path


namespace tc {

static QString s_switch_curr_btn_style = "QPushButton{border:0px;font-family:Microsoft YaHei;font-size:%1px; color:#000000;background-color:#ffffff;}";
static QString s_switch_no_curr_btn_style = "QPushButton{border:0px;font-family:Microsoft YaHei;font-size:%1px; color:#333333;background-color:#ffffff;}";

QString FileTransWidget::local_current_dir_path_;
QString FileTransWidget::remote_current_dir_path_;


StatisticsPanel::StatisticsPanel(QWidget* parent) : QWidget(parent) {
	InitView();
}

StatisticsPanel::~StatisticsPanel() {
}


void StatisticsPanel::SetData(int total, int completed, int failed) {
	total_value_lab_->setText(QString::number(total));
    completed_value_lab_->setText(QString::number(completed));
    failed_value_lab_->setText(QString::number(failed));
}

void StatisticsPanel::SetSpeed(EFileTransmitTaskType type, const QString& speed) {
	if (EFileTransmitTaskType::kDownload == type) {
		download_speed_value_lab_->setText(speed);
	}
	else if (EFileTransmitTaskType::kUpload == type) {
		upload_speed_value_lab_->setText(speed);
	}
}

void StatisticsPanel::InitView() {
	setAttribute(Qt::WA_StyledBackground);
	hbox_main_layout_ = new QHBoxLayout(this);
	hbox_main_layout_->setContentsMargins(0, 0, 0, 0);
	hbox_main_layout_->setSpacing(0);

	total_lab_ = new QLabel(this); 
	//total_lab_->setText(QStringLiteral("传输总数:"));
	total_lab_->setText(tcTr("id_file_trans_st_count_of_trans"));
	total_value_lab_ = new QLabel(this);

	completed_lab_ = new QLabel(this);
	//completed_lab_->setText(QStringLiteral("已完成:"));
	completed_lab_->setText(tcTr("id_file_trans_st_completed"));
	completed_value_lab_ = new QLabel(this);

	failed_lab_ = new QLabel(this);
	//failed_lab_->setText(QStringLiteral("失败:"));
	failed_lab_->setText(tcTr("id_file_trans_st_failed"));
	failed_value_lab_ = new QLabel(this);

	upload_speed_lab_ = new QLabel(this);
    //upload_speed_lab_->setText(QStringLiteral("上传速度:"));
	upload_speed_lab_->setText(tcTr("id_file_trans_st_upload_speed"));
    upload_speed_value_lab_ = new QLabel(this);
	upload_speed_value_lab_->setFixedWidth(60);
	upload_speed_value_lab_->setText("0");

	download_speed_lab_ = new QLabel(this);
    //download_speed_lab_->setText(QStringLiteral("下载速度:"));
	download_speed_lab_->setText(tcTr("id_file_trans_st_down_speed"));
    download_speed_value_lab_ = new QLabel(this);
	download_speed_value_lab_->setFixedWidth(60);
	download_speed_value_lab_->setText("0");

	hbox_main_layout_->addWidget(total_lab_);
	hbox_main_layout_->addSpacing(6);
	hbox_main_layout_->addWidget(total_value_lab_);
	hbox_main_layout_->addSpacing(12);
	hbox_main_layout_->addWidget(completed_lab_);
	hbox_main_layout_->addSpacing(6);
	hbox_main_layout_->addWidget(completed_value_lab_);
	hbox_main_layout_->addSpacing(12);
	hbox_main_layout_->addWidget(failed_lab_);
	hbox_main_layout_->addSpacing(6);
	hbox_main_layout_->addWidget(failed_value_lab_);
	hbox_main_layout_->addSpacing(12);
	hbox_main_layout_->addWidget(upload_speed_lab_);
    hbox_main_layout_->addSpacing(4);
	hbox_main_layout_->addWidget(upload_speed_value_lab_);
	hbox_main_layout_->addSpacing(6);
	hbox_main_layout_->addWidget(download_speed_lab_);
	hbox_main_layout_->addSpacing(4);
	hbox_main_layout_->addWidget(download_speed_value_lab_);

}

FileTransWidget::FileTransWidget(QWidget* parent) : QWidget(parent) {
	QSettings configIniWrite(QSettings::IniFormat, QSettings::UserScope, FILE_ORGANIZATION, FILE_APPLICATION);
	// to do : s_stream_id_ 对一个流路来说，s_stream_id_ 是否是固定的 有待明确
	local_current_dir_path_ = configIniWrite.value("local_" + FileTransmitSDK::s_stream_id_).toString();
	remote_current_dir_path_ = configIniWrite.value("remote_" + FileTransmitSDK::s_stream_id_).toString();
	qRegisterMetaType<FileContainer>("FileContainer");
	qRegisterMetaType<EFileTransmitTaskType>("EFileTransmitTaskType");
	qRegisterMetaType<EFileTransmitTaskState>("EFileTransmitTaskState");
	qRegisterMetaType<EFileTransmitTaskErrorCause>("EFileTransmitTaskErrorCause");
	Init();
}

FileTransWidget::~FileTransWidget() {

}

void FileTransWidget::Init() {
	setAttribute(Qt::WA_StyledBackground);
	resize(1366, 768);
	//setWindowTitle(QStringLiteral("文件传输"));
	setStyleSheet("QWidget {background-color: #FFFFFF;}");
	main_vbox_layout_ = new QVBoxLayout();
	setLayout(main_vbox_layout_);
	main_vbox_layout_->setContentsMargins(4,4,4,4);
	main_vbox_layout_->setSpacing(0);
	main_vbox_layout_->addSpacing(2);

	// 文件操作与展示区域
	file_show_hbox_layout_ = new QHBoxLayout();
	file_show_hbox_layout_->setContentsMargins(6, 6, 6, 6);
	file_show_hbox_layout_->setSpacing(0);
	file_show_widget_ = new FileShowWidget(this, local_current_dir_path_.isEmpty() ? QStringLiteral("/") : local_current_dir_path_, remote_current_dir_path_.isEmpty() ? QStringLiteral("/") : remote_current_dir_path_);
	file_show_hbox_layout_->addWidget(file_show_widget_);
	main_vbox_layout_->addSpacing(2);
	main_vbox_layout_->addLayout(file_show_hbox_layout_, 374);

	// 切换按钮布局
	record_or_log_switch_hbox_layout_ = new QHBoxLayout();
	record_or_log_switch_hbox_layout_->setContentsMargins(0,0,0,0);
	record_or_log_switch_hbox_layout_->setSpacing(0);
	s_switch_curr_btn_style = s_switch_curr_btn_style.arg(10);
	s_switch_no_curr_btn_style = s_switch_no_curr_btn_style.arg(10);

	btn_record_table_ = new FileTransLogSwitchButton(this);
	btn_record_table_->setFixedSize(90, 24);
	//btn_record_table_->setText(QStringLiteral("传输记录"));
	btn_record_table_->setText(tcTr("id_file_trans_records"));
	btn_record_table_->setStyleSheet(s_switch_curr_btn_style);
	btn_record_table_->setActive(true);


	btn_record_log_ = new FileTransLogSwitchButton(this);
	btn_record_log_->setFixedSize(60, 24);
	//btn_record_log_->setText(QStringLiteral(" 日 志 "));
	btn_record_log_->setText(tcTr("id_file_trans_log"));
	btn_record_log_->setStyleSheet(s_switch_no_curr_btn_style);
	btn_record_log_->setActive(false);

	btn_record_table_->setOnClickedCallback([=]() {
		this->btn_record_table_->setActive(true);
		this->btn_record_log_->setActive(false);
	});

	btn_record_log_->setOnClickedCallback([=]() {
		this->btn_record_log_->setActive(true);
		this->btn_record_table_->setActive(false);
	});
	btn_record_log_->setCursor(QCursor(Qt::PointingHandCursor));
	btn_record_table_->setCursor(QCursor(Qt::PointingHandCursor));


	statistics_panel_ = new StatisticsPanel(this);

	clear_completed_btn_ = new QPushButton(this);
	//clear_completed_btn_->setText(QStringLiteral("清除已完成"));
	clear_completed_btn_->setText(tcTr("id_file_trans_clear_completed"));
	clear_completed_btn_->setFixedSize(90, 24);
	clear_completed_btn_->setStyleSheet(R"(
		QPushButton {border:0px;font-family:Microsoft YaHei;font-size:10px; color:#000000;background-color:#ffffff;}
		QPushButton::hover {border:0px;font-family:Microsoft YaHei;font-size:10px; color:#000000;background-color:#f0f0f0;}
		QPushButton::pressed {border:0px;font-family:Microsoft YaHei;font-size:10px; color:#000000;background-color:#d0d0d0;}
	")");
	clear_completed_btn_->setCursor(QCursor(Qt::PointingHandCursor));

	record_or_log_switch_hbox_layout_->addSpacing(12);
	record_or_log_switch_hbox_layout_->addWidget(btn_record_table_);
	record_or_log_switch_hbox_layout_->addSpacing(8);
	record_or_log_switch_hbox_layout_->addWidget(btn_record_log_);
	record_or_log_switch_hbox_layout_->addStretch(1);
	record_or_log_switch_hbox_layout_->addWidget(statistics_panel_);
	record_or_log_switch_hbox_layout_->addSpacing(12);
	record_or_log_switch_hbox_layout_->addWidget(clear_completed_btn_);
	record_or_log_switch_hbox_layout_->addSpacing(12);
	main_vbox_layout_->addSpacing(0);
	main_vbox_layout_->addLayout(record_or_log_switch_hbox_layout_);

	// 文件传输日志记录
	file_trans_record_hbox_layout_ = new QHBoxLayout();
	file_trans_record_hbox_layout_->setContentsMargins(6, 6, 6, 6);
	file_trans_record_hbox_layout_->setSpacing(6);

	stack_widget_ = new QStackedWidget(this);
	file_trans_record_ = new FileTransRecord(this);
	file_trans_log_ = new FileTransmitLogWidget(this);
	stack_widget_->addWidget(file_trans_record_);
	stack_widget_->addWidget(file_trans_log_);
	stack_widget_->setCurrentWidget(file_trans_record_);
	file_trans_record_hbox_layout_->addWidget(stack_widget_);

	main_vbox_layout_->addLayout(file_trans_record_hbox_layout_, 168);


	logo_lab_ = new QLabel(this);
	logo_lab_->setFixedSize(120, 50);
	logo_lab_->setAttribute(Qt::WA_StyledBackground, true);
	logo_lab_->setStyleSheet(R"(image: url(:/resource/logo_text.png);
                                    background-repeat:no-repeat;
                                    background-position: center center;)");
	logo_lab_->move(this->width() * 0.88, this->height() * 0.92);


	activateWindow();
	setFocus();
	this->installEventFilter(this);
	InitSigChannel();
}

void FileTransWidget::SetDevicesId(const QString& local_device_id, const QString& remove_device_id) {
	file_show_widget_->SetDevicesId(local_device_id, remove_device_id);
}

void FileTransWidget::SetFileTransmitDirectionControlPermission(int permission) {
	FileShowWidget::file_transfer_direction_control_ = permission;
}

void FileTransWidget::InitSigChannel() {
	connect(file_show_widget_, &FileShowWidget::SigLocalCurrentDirChangePath, this, [=](QString path) {
		local_current_dir_path_ = path;
		});
	connect(file_show_widget_, &FileShowWidget::SigRemoteCurrentDirChangePath, this, [=](QString path) {
		remote_current_dir_path_ = path;
		});

	connect(file_show_widget_, &FileShowWidget::SigNoFileTransmitPermission, this, [=](int direction) {
		// to do 权限控制
	});

	connect(btn_record_table_, &QPushButton::clicked, this, &FileTransWidget::OnSwitchRecordOrLog);
	connect(btn_record_log_, &QPushButton::clicked, this, &FileTransWidget::OnSwitchRecordOrLog);
	connect(clear_completed_btn_, &QPushButton::clicked, this, &FileTransWidget::ClearCompletedTasksRecord);

	statistics_timer_ = new QTimer(this);
	statistics_timer_->setInterval(2000);
	connect(statistics_timer_, &QTimer::timeout, this, [=, this]() {
		auto info_res = file_trans_record_->GetStatisticsInfo();
        statistics_panel_->SetData(std::get<0>(info_res), std::get<1>(info_res), std::get<2>(info_res));
	});
	statistics_timer_->start();

    connect(FileTransmitSingleTaskManager::Instance(), &FileTransmitSingleTaskManager::SigSpeed, this, [=](EFileTransmitTaskType type, QString speed) {
		statistics_panel_->SetSpeed(type, speed);
	});
}

void FileTransWidget::resizeEvent(QResizeEvent* event) {
	logo_lab_->move(this->width() * 0.88, this->height() * 0.92);
	QWidget::resizeEvent(event);
}

void FileTransWidget::focusOutEvent(QFocusEvent* event) {
	QWidget::focusOutEvent(event);
}


bool FileTransWidget::eventFilter(QObject* object, QEvent* event) {
	if (event->type() == QEvent::FocusOut && object == this) {

	}
    else if (event->type() == QEvent::Show && object == this) {
        WidgetHelper::SetTitleBarColor(this);
    }
	return QObject::eventFilter(object, event);
}

void FileTransWidget::OnSwitchRecordOrLog() {
	if (btn_record_table_ == qobject_cast<QPushButton*>(QObject::sender())) {
		btn_record_table_->setStyleSheet(s_switch_curr_btn_style);
		btn_record_log_->setStyleSheet(s_switch_no_curr_btn_style);
		stack_widget_->setCurrentWidget(file_trans_record_);
	}
	else {
		btn_record_log_->setStyleSheet(s_switch_curr_btn_style);
		btn_record_table_->setStyleSheet(s_switch_no_curr_btn_style);
		stack_widget_->setCurrentWidget(file_trans_log_);
	}
}

void FileTransWidget::SaveCurrentVisitPath() {
	QSettings configIniWrite(QSettings::IniFormat, QSettings::UserScope, FILE_ORGANIZATION, FILE_APPLICATION);
	configIniWrite.setValue("local_" + FileTransmitSDK::s_stream_id_, local_current_dir_path_);
	configIniWrite.setValue("remote_" + FileTransmitSDK::s_stream_id_, remote_current_dir_path_);
	configIniWrite.sync();
}

void FileTransWidget::RefreshByComboboxSelectDirContent() {
	file_show_widget_->RefreshByComboboxSelectDirContent();
}

void FileTransWidget::ClearCompletedTasksRecord() {
	file_trans_record_->ClearCompletedTasksRecord();
	auto info_res = file_trans_record_->GetStatisticsInfo();
	statistics_panel_->SetData(std::get<0>(info_res), std::get<1>(info_res), std::get<2>(info_res));
}

}