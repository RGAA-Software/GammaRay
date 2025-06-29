#pragma once
#include <qevent.h>
#include <qdialog.h>
class QWidget;
class QPushButton;
class QLabel;
class QHBoxLayout;
class QVBoxLayout;


namespace tc {
// 覆盖弹窗
class FileCoverDialog : public QDialog {
	Q_OBJECT
public:
	enum class EOperationType {
		kUnknow,
		kCover,
		kAllCover,
		kSkip,
		kAllSkip,
		kCancel,
	};

	FileCoverDialog(QWidget* parent = nullptr);
	EOperationType operation_type_ = EOperationType::kCancel;
	void Init();
	void SetData(bool is_download, const QString& file_name, const QString& local_file_size, const QString& local_file_time,
		const QString& remote_file_size, const QString& remote_file_time);

	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
private:
	QVBoxLayout* main_vbox_layout_ = nullptr;
	// 标题、关闭按钮
	QHBoxLayout* title_hbox_layout_ = nullptr;
	QLabel* lab_title_ = nullptr;
	QPushButton* btn_close_ = nullptr;

	// 文件已存在
	QHBoxLayout* hint_hbox_layout_ = nullptr;
	QPushButton* btn_icon_warning_ = nullptr;
	QLabel* lab_hint_text_ = nullptr;

	// 文件名称
	QHBoxLayout* file_name_hbox_layout_ = nullptr;
	QLabel* lab_file_name_ = nullptr;

	// 本地文件信息
	QHBoxLayout* local_file_info_layout_ = nullptr;
	QLabel* lab_local_file_name_ = nullptr;
	QLabel* lab_local_file_size_ = nullptr;
	QLabel* lab_local_file_place_ = nullptr;
	QLabel* lab_local_file_time_ = nullptr;

	// 远端文件信息
	QHBoxLayout* remote_file_info_layout_ = nullptr;
	QLabel* lab_remote_file_name_ = nullptr;
	QLabel* lab_remote_file_size_ = nullptr;
	QLabel* lab_remote_file_place_ = nullptr;
	QLabel* lab_remote_file_time_ = nullptr;

	// 相关操作
	QHBoxLayout* operation_hbox_layout_ = nullptr;
	QPushButton* btn_cover_ = nullptr;
	QPushButton* btn_all_cover_ = nullptr;
	QPushButton* btn_skip_ = nullptr;
	QPushButton* btn_all_skip_ = nullptr;
	QPushButton* btn_cancel_ = nullptr;

	bool pressed_ = false;
	QPoint point_;

	void InitSigChannel();
};

}