#pragma once

#include <qwidget.h>

class QHBoxLayout;
class QVBoxLayout;
class QLabel;


// 文件展示，包含本地文件与远端文件
namespace tc {

class FileOverviewWidget;

class FileShowWidget : public QWidget {
	Q_OBJECT
public:
	static int file_transfer_direction_control_;
	FileShowWidget(QWidget* parent = nullptr, QString local_path = "", QString remote_path = "");
	~FileShowWidget();
	void Init(QString local_path = "", QString remote_path = "");
	void RefreshByComboboxSelectDirContent();
	void SetDevicesId(const QString& local_device_id, const QString& remove_device_id);
signals:
	void SigLocalCurrentDirChangePath(QString);
	void SigRemoteCurrentDirChangePath(QString);
	void SigNoFileTransmitPermission(int direction);
private:
	QHBoxLayout* main_hbox_layout_ = nullptr;
	FileOverviewWidget* local_file_overview_widget_ = nullptr;
	FileOverviewWidget* remote_file_overview_widget_ = nullptr;

	bool local_file_list_has_disk_ = false;
	bool remote_file_list_has_disk_ = false;
	void SetSendBtnState();

	void InitSigChannel();
};
}