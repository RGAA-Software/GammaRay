#include "file_send_btn.h"

#include <QHBoxLayout>
#include <QLabel>
#include <qpixmap.h>
#include "tc_label.h"

namespace tc {

static QString s_style = R"(
QPushButton{border:0px; border-radius:%1px; background-color:#2979ff;}
QPushButton::hover{border:0px; border-radius:%1px; background-color:#2059ee;}
QPushButton::pressed{border:0px; border-radius:%1px; background-color:#1549dd;}
QPushButton::disabled{border:0px; border-radius:%1px; background-color:#d9d9d6;}
)";

FileSendBtn::FileSendBtn(FileAffiliationType type) : QPushButton()
{
	setAttribute(Qt::WA_StyledBackground);
	auto layout = new QHBoxLayout(this);

	layout->setSpacing(2);
	s_style = s_style.arg(4);
	setStyleSheet(s_style);
	//auto textLab = new QLabel(QStringLiteral("发送"));
	auto textLab = new QLabel(tcTr("id_file_trans_sending"));
	textLab->setStyleSheet("color:#ffffff;background-color: transparent;");

	QLabel* imageLab = new QLabel();
	layout->addWidget(imageLab);
	imageLab->setStyleSheet("background-color: transparent;");

	switch (type)
	{
	case FileAffiliationType::kLocal:
	{
		layout->setContentsMargins(16, 9, 8, 8);
		//QPixmap pixmap = PixmapHelper::PixmapByResolution1xIn(QStringLiteral(":/resource/local_send.png"));
		QPixmap pixmap(QStringLiteral(":/resource/local_send.png"));
		imageLab->setPixmap(pixmap);
		layout->addWidget(textLab);
		layout->addWidget(imageLab);
	}
	break;
	case FileAffiliationType::kRemote:
	{
		layout->setContentsMargins(8, 9, 16, 8);
		//QPixmap pixmap = PixmapHelper::PixmapByResolution1xIn(QStringLiteral(":/resource/remote_send.png"));
		QPixmap pixmap(QStringLiteral(":/resource/remote_send.png"));
		imageLab->setPixmap(pixmap);
		layout->addWidget(imageLab);
		layout->addWidget(textLab);
	}
	break;
	default:
		break;
	}

	setFixedSize(70, 30);
}

}