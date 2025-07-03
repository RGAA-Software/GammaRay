#include "connected_info_panel.h"
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qtimer.h>
#include "no_margin_layout.h"
#include "tc_label.h"
#include "tc_pushbutton.h"
#include "render_panel/gr_context.h"
#include "tc_qt_widget/widget_helper.h"
#include "render_panel/gr_settings.h"
#include "tc_render_panel_message.pb.h"
#include "render_panel/gr_application.h"
#include "tc_common_new/client_id_extractor.h"

namespace tc {

	ConnectedInfoPanel::ConnectedInfoPanel(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : QWidget(parent), ctx_(ctx) {
		GrSettings* settings = GrSettings::Instance();
		if (!settings) {
			return;
		}
		settings_ = settings;
		InitView();
		WidgetHelper::AddShadow(this, 0xbbbbbb, 8);
		InitData();
		InitSigChannel();
	}
	
	void ConnectedInfoPanel::InitView() {
        setAttribute(Qt::WA_TranslucentBackground);

		setFixedSize(500, 200);
		root_vbox_layout_ = new NoMarginVLayout();
		setLayout(root_vbox_layout_);
		root_vbox_layout_->addSpacing(20);

		// logo标识
		logo_hbox_layout_ = new NoMarginHLayout();
		logo_lab_ = new TcLabel(this);
		QPixmap logo_pixmap(":/resources/tc_icon.png");
		logo_pixmap = logo_pixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		logo_lab_->setPixmap(logo_pixmap);
		logo_lab_->setFixedSize(20, 20);
		logo_name_lab_ = new TcLabel(this);
		logo_name_lab_->setText("GammaRay");
		logo_name_lab_->setStyleSheet("font-size: 14px; font-weight: 500; color: #2979FF;");
		logo_hbox_layout_->addSpacing(18);
		logo_hbox_layout_->addWidget(logo_lab_);
		logo_hbox_layout_->addSpacing(6);
		logo_hbox_layout_->addWidget(logo_name_lab_);
		root_vbox_layout_->addLayout(logo_hbox_layout_);

		// 连接者信息
		avatar_name_hbox_layout_ = new NoMarginHLayout();
		avatar_lab_ = new TcLabel(this);
		avatar_lab_->setFixedSize(30, 30);
		QPixmap avatar_pixmap(":/resources/avatar.png");
		avatar_pixmap = avatar_pixmap.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		avatar_lab_->setPixmap(avatar_pixmap);

		auto info_vbox_layout = new NoMarginVLayout();
		key_1_lab_ = new TcLabel(this);
		key_1_lab_->setStyleSheet("font-size: 14px; font-weight: 800;");
		key_1_lab_->setText("");

		key_2_lab_ = new TcLabel(this);
		key_2_lab_->setStyleSheet("font-size: 12px; font-weight: 500;");
		key_2_lab_->setText("");

		info_vbox_layout->addWidget(key_1_lab_);
		info_vbox_layout->addSpacing(4);
		info_vbox_layout->addWidget(key_2_lab_);

		conn_prompt_lab_ = new TcLabel(this);
		conn_prompt_lab_->setStyleSheet("font-size: 14px; font-weight: 400;");
		conn_prompt_lab_->SetTextId("id_remote_local_machine");
		conn_prompt_lab_->adjustSize();
		disconnect_btn_ = new TcPushButton(this);
		disconnect_btn_->setFixedWidth(90);
		disconnect_btn_->SetTextId("id_disconncet");
        connect(disconnect_btn_, &QPushButton::clicked, this, [=, this]() {
            if (!info_) {
                return;
            }
            tcrp::RpMessage msg;
            msg.set_type(tcrp::kRpDisconnectConnection);
            auto sub = msg.mutable_disconnect_connection();
            sub->set_device_id(info_->device_id());
            sub->set_stream_id(info_->stream_id());
            sub->set_room_id(info_->room_id());
            sub->set_device_name(info_->device_name());
            ctx_->GetApplication()->PostMessage2Renderer(msg.SerializeAsString());
        });
		disconnect_btn_->setProperty("class", "danger");

		avatar_name_hbox_layout_->addSpacing(18);
		avatar_name_hbox_layout_->addWidget(avatar_lab_);
		avatar_name_hbox_layout_->addSpacing(6);
		avatar_name_hbox_layout_->addLayout(info_vbox_layout);
		avatar_name_hbox_layout_->addSpacing(6);
		avatar_name_hbox_layout_->addWidget(conn_prompt_lab_, 0, Qt::AlignTop);
		avatar_name_hbox_layout_->addSpacing(6);
		avatar_name_hbox_layout_->addStretch(1);
		avatar_name_hbox_layout_->addWidget(disconnect_btn_);
		avatar_name_hbox_layout_->addSpacing(14);
		root_vbox_layout_->addSpacing(12);
		root_vbox_layout_->addLayout(avatar_name_hbox_layout_);
		root_vbox_layout_->addStretch(1);

		// 允许访问
		promtp_hbox_layout_ = new NoMarginHLayout();
		promtp_hbox_layout_->setAlignment(Qt::AlignLeft);
		prompt_lab_ = new TcLabel(this);
		prompt_lab_->setStyleSheet("font-size: 14px; font-weight: 400;");
		prompt_lab_->SetTextId("id_allow_access_to");

		access_hint_lab_ = new TcLabel(this);
		access_hint_lab_->SetTextId("id_access_hint");
		access_hint_lab_->setStyleSheet("font-size: 14px; font-weight: 400;");
		access_hint_lab_->hide();

		promtp_hbox_layout_->addSpacing(18);
		promtp_hbox_layout_->addWidget(prompt_lab_);
		promtp_hbox_layout_->addSpacing(12);
		promtp_hbox_layout_->addWidget(access_hint_lab_);
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

		access_control_hbox_layout_->addSpacing(18);
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
		access_control_hbox_layout_->addSpacing(6);
		access_control_hbox_layout_->addStretch(1);
		root_vbox_layout_->addSpacing(8);
		root_vbox_layout_->addLayout(access_control_hbox_layout_);
		root_vbox_layout_->addStretch(1);
		root_vbox_layout_->addSpacing(20);
	}

	void ConnectedInfoPanel::paintEvent(QPaintEvent* event) {
		QWidget::paintEvent(event);
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(0xffffff));
        painter.drawRoundedRect(0, 5, this->width(), this->height()-10, 5, 5);
	}

	void ConnectedInfoPanel::UpdateInfo(const std::shared_ptr<tcrp::RpConnectedClientInfo>& info) {
        info_ = info;
		key_1_lab_->setText(ExtractClientId(info_->device_id()).c_str());
		key_1_lab_->adjustSize();
		key_2_lab_->setText(info_->device_name().c_str());
		key_2_lab_->adjustSize();
	}

	void ConnectedInfoPanel::InitData() {
		if (!settings_) {
			return;
		}
		bool audio_access = settings_->IsCaptureAudioEnabled();
		voice_cbox_->setChecked(audio_access);

		bool file_access = settings_->IsFileTransferEnabled();
		file_cbox_->setChecked(file_access);

		bool key_mouse_access = settings_->IsBeingOperatedEnabled();
		key_mouse_cbox_->setChecked(key_mouse_access);
	}

	void ConnectedInfoPanel::InitSigChannel() {
		if (!settings_) {
			return;
		}
		connect(voice_cbox_, &QCheckBox::toggled, this, [=] {
			bool audio_access = settings_->IsCaptureAudioEnabled();
			voice_cbox_->setChecked(audio_access);
			ShowAccessHint();
		});
		
		connect(file_cbox_, &QCheckBox::toggled, this, [=] {
			bool file_access = settings_->IsFileTransferEnabled();
			file_cbox_->setChecked(file_access);
			ShowAccessHint();
		});

		connect(key_mouse_cbox_, &QCheckBox::toggled, this, [=] {
			bool key_mouse_access = settings_->IsBeingOperatedEnabled();
			key_mouse_cbox_->setChecked(key_mouse_access);
			ShowAccessHint();
		});
	}

	void ConnectedInfoPanel::ShowAccessHint() {
		access_hint_lab_->show();
		QTimer::singleShot(3000, this, [=, this] {
			access_hint_lab_->hide();
		});
	}
}

