#include "file_operation_btn.h"
#include <iostream>
#include <qdebug.h>

namespace tc {
static QString s_style = R"(
	QPushButton{border:0px; border-radius:4px; background-color:#ffffff; image:url(%1); padding: 7px;}
	QPushButton::hover{border:0px; border-radius:4px; background-color:#E0E4EB; image:url(%2); padding: 7px;} 
	QPushButton::pressed{border:0px; border-radius:4px; background-color:#C7CDDB; image:url(%3);padding: 7px;}
	QPushButton::disabled{border:0px; border-radius:4px; background-color:#FFFFFF; image:url(%4);padding: 7px;}
)";


FileOperationBtn::FileOperationBtn(const QString& normal_img, const QString& hover_img, const QString& pressed_img,
	const QString& disabled_img, FileOperationType type, QWidget* parent) : operation_type_(type), QPushButton(parent)
{
	setAttribute(Qt::WA_StyledBackground);
	auto style_str = s_style;
	style_str = style_str.arg(normal_img, hover_img, pressed_img, disabled_img);
	setStyleSheet(style_str);
	setFixedSize(30, 30);
}

FileOperationBtn:: ~FileOperationBtn() {

}

}

