#pragma once
#include <qpushbutton.h>

namespace tc {
enum class FileOperationType {
	kGoBack,
	kGoParentDir,
	kGoIndex,
	kRefresh
};

class FileOperationBtn : public QPushButton {
public:
	FileOperationBtn(const QString& normal_img, const QString& hover_img, const QString& pressed_img,
		const QString& disabled_img, FileOperationType type, QWidget* parent = nullptr);
	~FileOperationBtn();
private:
	FileOperationType operation_type_ = FileOperationType::kGoBack;
};

}