#include "connected_info_panel.h"
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include "no_margin_layout.h"
#include "tc_label.h"
#include "tc_pushbutton.h"

namespace tc {

	ConnectedInfoPanel::ConnectedInfoPanel(QWidget* parent) : QWidget(parent) {
		InitView();
	}
	
	void ConnectedInfoPanel::InitView() {
		setAttribute(Qt::WA_StyledBackground);
		setFixedSize(500, 200);
		root_vbox_layout_ = new NoMarginVLayout();
		setLayout(root_vbox_layout_);

		// logo标识
		logo_hbox_layout_ = new NoMarginHLayout();
		logo_lab_ = new TcLabel(this);
		QPixmap logo_pixmap(":/resources/tc_icon.png");
		logo_pixmap = logo_pixmap.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		logo_lab_->setPixmap(logo_pixmap);
		logo_lab_->setFixedSize(40, 40);
		logo_name_lab_ = new TcLabel(this);
		logo_name_lab_->setText("GammaRay");
		logo_name_lab_->setStyleSheet("font-size: 18px; font-weight: 500;");
		logo_hbox_layout_->addSpacing(12);
		logo_hbox_layout_->addWidget(logo_lab_);
		logo_hbox_layout_->addSpacing(6);
		logo_hbox_layout_->addWidget(logo_name_lab_);
		root_vbox_layout_->addLayout(logo_hbox_layout_);

		// 连接者信息
		avatar_name_hbox_layout_ = new NoMarginHLayout();
		avatar_lab_ = new TcLabel(this);
		avatar_lab_->setFixedSize(50, 50);
		QPixmap avatar_pixmap(":/resources/avatar.png");
		avatar_pixmap = avatar_pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		avatar_lab_->setPixmap(avatar_pixmap);
		name_lab_ = new TcLabel(this);
		name_lab_->setStyleSheet("font-size: 14px; font-weight: 500;");
		name_lab_->setText("Test");
		conn_prompt_lab_ = new TcLabel(this);
		conn_prompt_lab_->setStyleSheet("font-size: 14px; font-weight: 400;");
		conn_prompt_lab_->SetTextId("id_remote_local_machine");
		disconnect_btn_ = new TcPushButton(this);
		disconnect_btn_->SetTextId("id_disconncet");

		avatar_name_hbox_layout_->addSpacing(12);
		avatar_name_hbox_layout_->addWidget(avatar_lab_);
		avatar_name_hbox_layout_->addSpacing(6);
		avatar_name_hbox_layout_->addWidget(name_lab_);
		avatar_name_hbox_layout_->addSpacing(6);
		avatar_name_hbox_layout_->addWidget(conn_prompt_lab_);
		avatar_name_hbox_layout_->addSpacing(6);
		avatar_name_hbox_layout_->addWidget(disconnect_btn_);
		root_vbox_layout_->addLayout(avatar_name_hbox_layout_);

		// 允许访问
		promtp_hbox_layout_ = new NoMarginHLayout();
		prompt_lab_ = new TcLabel(this);
		prompt_lab_->setStyleSheet("font-size: 14px; font-weight: 400;");
		prompt_lab_->SetTextId("id_allow_access_to");
		promtp_hbox_layout_->addSpacing(12);
		promtp_hbox_layout_->addWidget(prompt_lab_);
		root_vbox_layout_->addLayout(promtp_hbox_layout_);

		// 权限控制
		access_control_hbox_layout_ = new NoMarginHLayout();
		voice_lab_ = new TcLabel(this);
		voice_lab_->SetTextId("id_voice");
		voice_lab_->setStyleSheet("font-size: 14px; font-weight: 400;");
		voice_cbox_ = new QCheckBox(this);

		key_mouse_lab_ = new TcLabel(this);
		key_mouse_lab_->SetTextId("id_key_mouse");
		key_mouse_lab_->setStyleSheet("font-size: 14px; font-weight: 400;");
		key_mouse_cbox_ = new QCheckBox(this);

		file_lab_ = new TcLabel(this);
		file_lab_->SetTextId("id_file");
		file_lab_->setStyleSheet("font-size: 14px; font-weight: 400;");
		file_cbox_ = new QCheckBox(this);

		access_control_hbox_layout_->addSpacing(12);
		access_control_hbox_layout_->addWidget(voice_lab_);
		access_control_hbox_layout_->addSpacing(4);
		access_control_hbox_layout_->addWidget(voice_cbox_);
		access_control_hbox_layout_->addSpacing(12);
		access_control_hbox_layout_->addWidget(key_mouse_lab_);
		access_control_hbox_layout_->addSpacing(4);
		access_control_hbox_layout_->addWidget(key_mouse_cbox_);
		access_control_hbox_layout_->addSpacing(12);
		access_control_hbox_layout_->addWidget(file_lab_);
		access_control_hbox_layout_->addSpacing(4);
		access_control_hbox_layout_->addWidget(file_cbox_);
		root_vbox_layout_->addLayout(access_control_hbox_layout_);
	}
}

