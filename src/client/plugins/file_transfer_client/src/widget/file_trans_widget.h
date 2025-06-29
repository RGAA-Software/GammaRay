#pragma once
#include <qwidget.h>
#include <qstring.h>
#include "file_transmit_task_state.h"
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QStackedWidget;
class QPushButton;
class QTimer;

namespace tc {

class FileShowWidget;
class FileTransRecord;
class FileTransmitLogWidget;
class FileTransLogSwitchButton;

class StatisticsPanel : public QWidget {
	Q_OBJECT
public:
    StatisticsPanel(QWidget* parent = nullptr);
    ~StatisticsPanel();
	void SetData(int total, int completed, int failed);
    void SetSpeed(EFileTransmitTaskType type, const QString& speed);
private:
	void InitView();
private:
	QHBoxLayout* hbox_main_layout_ = nullptr;
    QLabel* total_lab_ = nullptr;
	QLabel* total_value_lab_ = nullptr;
	QLabel* completed_lab_ = nullptr;
    QLabel* completed_value_lab_ = nullptr;
    QLabel* failed_lab_ = nullptr;
	QLabel* failed_value_lab_ = nullptr;
    QLabel* upload_speed_lab_ = nullptr;
    QLabel* upload_speed_value_lab_ = nullptr;
    QLabel* download_speed_lab_ = nullptr;
    QLabel* download_speed_value_lab_ = nullptr;

	
};

// 文件传输主页
class FileTransWidget : public QWidget {
	Q_OBJECT
public:
	FileTransWidget(QWidget* parent = nullptr);
	~FileTransWidget();
	void Init();
	void RefreshByComboboxSelectDirContent();
	static void SetFileTransmitDirectionControlPermission(int permission);
	void resizeEvent(QResizeEvent* event) override;

	void focusOutEvent(QFocusEvent* event) override;
	bool eventFilter(QObject* object, QEvent* event) override;
	static void SaveCurrentVisitPath();
	void SetDevicesId(const QString& local_device_id, const QString& remove_device_id);
private:
	void InitSigChannel();
	void ClearCompletedTasksRecord();
private:
	QVBoxLayout* main_vbox_layout_ = nullptr;

	// 文件操作与展示 
	QHBoxLayout* file_show_hbox_layout_ = nullptr;
	FileShowWidget* file_show_widget_ = nullptr;

	// 记录与日志标签
	QHBoxLayout* record_or_log_switch_hbox_layout_ = nullptr;
	FileTransLogSwitchButton* btn_record_table_ = nullptr;
	FileTransLogSwitchButton* btn_record_log_ = nullptr;
	StatisticsPanel* statistics_panel_ = nullptr;
	QPushButton* clear_completed_btn_ = nullptr;

	// 文件传输列表
	QHBoxLayout* file_trans_record_hbox_layout_ = nullptr;
	QStackedWidget* stack_widget_ = nullptr;
	FileTransRecord* file_trans_record_ = nullptr;
	FileTransmitLogWidget* file_trans_log_ = nullptr;

	QTimer* statistics_timer_ = nullptr;

	QLabel* logo_lab_ = nullptr;

	static QString local_current_dir_path_;
	static QString remote_current_dir_path_;
public slots:
	void OnSwitchRecordOrLog();
};
}