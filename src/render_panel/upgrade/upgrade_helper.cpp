#include "upgrade_helper.h"
#include <qwindow.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <Windows.h>
#include <dwmapi.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qtextedit.h>
#include <qfile.h>
#include <qfiledevice.h>
#include <qfileinfo.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qprocess.h>
#include <qurl.h>
#include <qurlquery.h>
#include <qjsonarray.h>
#include <qstandardpaths.h>
#include <tc_common_new/log.h>
#include <tc_common_new/gd_md5.h>
#include "tc_qt_widget/tc_dialog.h"
#include "translator/tc_translator.h"
#include "gd_button.h"
#include "gd_custom_progress_bar.h"
#include "version_config.h"
#include "render_panel/gr_settings.h"



namespace tc {
	static const int kUpgradeApiOkValue = 200;
	static const std::string kUpgradeServerPort = "30699";
	static const std::string kUpgradeQueryPath = "/query_update_info";
	static const std::string kUpgradeDownloadPath = "/download";

	void UpgradeHelperWidget::paintEvent(QPaintEvent* event) {
		QPainter painter(this);
		// 启用抗锯齿
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::SmoothPixmapTransform, true); 
		// 获取窗口尺寸
		int windowHeight = rect().height();
		int oneThirdHeight = windowHeight / 10;
		painter.fillRect(0, 0, rect().width(), oneThirdHeight, QColor(0xe0, 0xf1, 0xff));
		painter.fillRect(0, oneThirdHeight, rect().width(), windowHeight - oneThirdHeight, QColor(0xff, 0xff, 0xff));
		static QString pixmap_path = QStringLiteral(":/resources/image/update/update_bk.png");
		static QPixmap pixmap(pixmap_path);
		pixmap.setDevicePixelRatio(2.0);
		QRect targetRect(0, 0, 320, 150);
		painter.drawPixmap(targetRect, pixmap);
	}

	UpgradeHelperWidget::UpgradeHelperWidget(QWidget* parent) : QDialog(parent) {
		InitUI();
		ApplyWindowsShadow();
		InitSigChannel();
	}

	UpgradeHelperWidget::~UpgradeHelperWidget() {
	
	}

	void UpgradeHelperWidget::SetForced(bool f) {
		forced_ = f;
		if (forced_) {
			stack_widget_->setCurrentWidget(forced_update_widget_);
		}
		else {
			stack_widget_->setCurrentWidget(notify_update_widget_);
		}
	}

	void UpgradeHelperWidget::keyPressEvent(QKeyEvent* event) {
		if (event->key() == Qt::Key_Escape) {
			event->ignore();
			event->accept();
			return;
		}
	}

	void UpgradeHelperWidget::keyReleaseEvent(QKeyEvent* event) {
		if (event->key() == Qt::Key_Escape) {
			event->ignore();
			event->accept();
			if (!forced_) {
				QMetaObject::invokeMethod(this, [this]() {
					TcDialog dialog(tcTr("id_tips"), tcTr("id_upgrade_are_you_sure_exit"), this);
					if (QDialog::Rejected == dialog.exec()) {
						return;
					}
					else {
						this->done(QDialog::Rejected);
					}
				});
			}
			return;
		}
	}

	void UpgradeHelperWidget::closeEvent(QCloseEvent* event) {
		event->ignore();
		if (!forced_) {
			QMetaObject::invokeMethod(this, [this]() {
				TcDialog dialog(tcTr("id_tips"), tcTr("id_upgrade_are_you_sure_exit"), this);
				if (QDialog::Rejected == dialog.exec()) {
					return;
				}
				else {
					this->done(QDialog::Rejected);
				}
			});
		}
	}

	void UpgradeHelperWidget::InitUI() {
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		setObjectName("UpgradeHelperWidget");
		setAttribute(Qt::WA_StyledBackground);
		setFixedSize(320, 380);

		QVBoxLayout* main_vbox_layout = new QVBoxLayout(this);
		main_vbox_layout->setSpacing(0);
		main_vbox_layout->setContentsMargins(0, 0, 0, 0);
		main_vbox_layout->setAlignment(Qt::AlignTop);
		main_vbox_layout->addStretch(1);
		{
			QHBoxLayout* hbox_layout = new QHBoxLayout();
			hbox_layout->setSpacing(0);
			hbox_layout->setContentsMargins(0, 0, 0, 0);
			hbox_layout->setAlignment(Qt::AlignLeft);

			remote_version_lab_ = new QLabel(this);
			remote_version_lab_->setFixedSize(180, 20);
			QString style = R"(
				QLabel {font-size: 15px; font-family: Microsoft YaHei; color: #000000; line-height: 20px; font-weight: 500}
			)";
			remote_version_lab_->setStyleSheet(style);
			remote_version_lab_->setAlignment(Qt::AlignLeft); 

			hbox_layout->addSpacing(20);
			hbox_layout->addWidget(remote_version_lab_);
			hbox_layout->addStretch(1);
			main_vbox_layout->addSpacing(10);
			main_vbox_layout->addLayout(hbox_layout);
		}

		{
			QHBoxLayout* msg_hlayout = new QHBoxLayout();
			msg_hlayout->setContentsMargins(0, 0, 0, 0);
			msg_hlayout->setSpacing(0);
			msg_hlayout->setAlignment(Qt::AlignLeft);

			text_edit_ = new QTextEdit();
			text_edit_->setFixedSize(280, 100);
			text_edit_->setPlaceholderText(QStringLiteral(""));
			QFont font("Microsoft YaHei");
			font.setPixelSize(13);
			text_edit_->setFont(font);

			text_edit_->setReadOnly(true); 
			text_edit_->setFrameStyle(QFrame::NoFrame); 
			text_edit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 
			text_edit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 
			text_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			QString edit_style = R"(
				QTextEdit {
					background: transparent;
					font-size: 13px;
					font-family: Microsoft YaHei;
					color: #8d8e8f;
					padding: 0px;
					border: 0px;
					font-weight: 400;
				}
			)";
			text_edit_->setStyleSheet(edit_style);

			msg_hlayout->addSpacing(18);
			msg_hlayout->addWidget(text_edit_);
			msg_hlayout->addSpacing(20);

			main_vbox_layout->addSpacing(12);
			main_vbox_layout->addLayout(msg_hlayout);
		}


		stack_widget_ = new QStackedWidget(this);
		stack_widget_->setFixedSize(320, 56);
		{
			notify_update_widget_ = new QWidget(stack_widget_);
			stack_widget_->addWidget(notify_update_widget_);
			notify_update_widget_->setFixedSize(320, 56);
			QHBoxLayout* hlayout = new QHBoxLayout(notify_update_widget_);
			hlayout->setContentsMargins(0, 0, 0, 0);
			hlayout->setSpacing(0);
			hlayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			confirm_btn_ = new GDButton(notify_update_widget_);
			GDButton::BorderInfo btn_border_info;
			GDButton::TextInfo btn_text_info;
			GDButton::IconInfo btn_icon_info;
			GDButton::BackgroundInfo btn_bk_info;
			btn_bk_info.m_background_color_normal = QColor(0x1A, 0x76, 0xF6);
			btn_bk_info.m_background_color_hover = QColor(0x1D, 0x79, 0xF9);
			btn_bk_info.m_background_color_press = QColor(0x1F, 0x7F, 0xFF);
			btn_bk_info.m_background_color_disable = QColor(0xcc, 0xcc, 0xcc);
			btn_text_info.m_blod = false;
			btn_text_info.m_font_color_normal = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_hover = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_press = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_disable = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_text = tcTr("id_upgrade_download_now");
			btn_text_info.m_font_size = 15;
			btn_text_info.m_padding_left = 36;
			btn_text_info.m_padding_top = 28;
			btn_border_info.m_border_radius = 4;
			btn_border_info.m_border_width = 0;
			btn_icon_info.m_have_icon = false;
			confirm_btn_->Init(QSize(132, 44), btn_text_info, btn_bk_info, btn_icon_info,
				btn_border_info);
			confirm_btn_->setEnabled(true);

			btn_text_info.m_text = tcTr("id_upgrade_update_later");
			cancel_btn_ = new GDButton(notify_update_widget_);

			btn_bk_info.m_background_color_normal = QColor(0xeb, 0xeb, 0xeb);
			btn_bk_info.m_background_color_hover = QColor(0xe9, 0xe9, 0xe9);
			btn_bk_info.m_background_color_press = QColor(0xe7, 0xe7, 0xe7);
			btn_bk_info.m_background_color_disable = QColor(0xcc, 0xcc, 0xcc);
			btn_text_info.m_font_color_normal = QColor(0x84, 0x84, 0x84);
			btn_text_info.m_font_color_hover = QColor(0x84, 0x84, 0x84);
			btn_text_info.m_font_color_press = QColor(0x84, 0x84, 0x84);
			btn_text_info.m_font_color_disable = QColor(0x84, 0x84, 0x84);

			cancel_btn_->Init(QSize(132, 44), btn_text_info, btn_bk_info, btn_icon_info,
				btn_border_info);
			cancel_btn_->setEnabled(true);

			hlayout->addStretch(1);
			hlayout->addWidget(cancel_btn_);
			hlayout->addSpacing(16);
			hlayout->addWidget(confirm_btn_);
			hlayout->addStretch(1);
		}

		{
			forced_update_widget_ = new QWidget(stack_widget_);
			stack_widget_->addWidget(forced_update_widget_);
			forced_update_widget_->setFixedSize(320, 56);
			QHBoxLayout* hlayout = new QHBoxLayout(forced_update_widget_);
			hlayout->setContentsMargins(0, 0, 0, 0);
			hlayout->setSpacing(0);
			hlayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			forced_confirm_btn_ = new GDButton(forced_update_widget_);
			GDButton::BorderInfo btn_border_info;
			GDButton::TextInfo btn_text_info;
			GDButton::IconInfo btn_icon_info;
			GDButton::BackgroundInfo btn_bk_info;
			btn_bk_info.m_background_color_normal = QColor(0x1A, 0x76, 0xF6);
			btn_bk_info.m_background_color_hover = QColor(0x1D, 0x79, 0xF9);
			btn_bk_info.m_background_color_press = QColor(0x1F, 0x7F, 0xFF);
			btn_bk_info.m_background_color_disable = QColor(0xcc, 0xcc, 0xcc);
			btn_text_info.m_blod = false;
			btn_text_info.m_font_color_normal = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_hover = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_press = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_disable = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_text = tcTr("id_upgrade_download_now");;
			btn_text_info.m_font_size = 15;
			btn_text_info.m_padding_left = 36;
			btn_text_info.m_padding_top = 28;
			btn_border_info.m_border_radius = 4;
			btn_border_info.m_border_width = 0;
			btn_icon_info.m_have_icon = false;
			forced_confirm_btn_->Init(QSize(132, 44), btn_text_info, btn_bk_info, btn_icon_info,
				btn_border_info);
			forced_confirm_btn_->setEnabled(true);

			btn_text_info.m_text = tcTr("id_upgrade_exit_app");
			exit_app_btn_ = new GDButton(notify_update_widget_);

			btn_bk_info.m_background_color_normal = QColor(0xeb, 0xeb, 0xeb);
			btn_bk_info.m_background_color_hover = QColor(0xe9, 0xe9, 0xe9);
			btn_bk_info.m_background_color_press = QColor(0xe7, 0xe7, 0xe7);
			btn_bk_info.m_background_color_disable = QColor(0xcc, 0xcc, 0xcc);
			btn_text_info.m_font_color_normal = QColor(0x84, 0x84, 0x84);
			btn_text_info.m_font_color_hover = QColor(0x84, 0x84, 0x84);
			btn_text_info.m_font_color_press = QColor(0x84, 0x84, 0x84);
			btn_text_info.m_font_color_disable = QColor(0x84, 0x84, 0x84);

			exit_app_btn_->Init(QSize(132, 44), btn_text_info, btn_bk_info, btn_icon_info,
				btn_border_info);
			exit_app_btn_->setEnabled(true);

			hlayout->addStretch(1);
			hlayout->addWidget(exit_app_btn_);
			hlayout->addSpacing(16);
			hlayout->addWidget(forced_confirm_btn_);
			hlayout->addStretch(1);
		}

		{
			download_widget_ = new QWidget(stack_widget_);
			stack_widget_->addWidget(download_widget_);
			download_widget_->setFixedSize(320, 56);
			QVBoxLayout* main_vlayout = new QVBoxLayout(download_widget_);
			main_vlayout->setSpacing(0);
			main_vlayout->setContentsMargins(0, 0, 0, 0);
			main_vlayout->setAlignment(Qt::AlignTop);
			QHBoxLayout* hlayout = new QHBoxLayout();
			hlayout->setContentsMargins(0, 0, 0, 0);
			hlayout->setSpacing(0);
			hlayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			
			progress_bar_ = new GDCustomProgressBar(download_widget_);
			progress_bar_->setFixedWidth(280);
			progress_bar_->setError(false);
			progress_bar_->setValue(0);
			hlayout->addStretch(1);
			hlayout->addWidget(progress_bar_);
			hlayout->addStretch(1);
			main_vlayout->addLayout(hlayout);

			QHBoxLayout* hint_layout = new QHBoxLayout();
			hint_layout->setContentsMargins(0, 0, 0, 0);
			hint_layout->setSpacing(0);
			hint_layout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			download_hint_lab_ = new QLabel(download_widget_);
			download_hint_lab_->setText(tcTr("id_upgrade_downloading"));
			QString style = R"(
				QLabel {font-size: 12px; font-family: Microsoft YaHei; color: #8f8f8f; line-height: 20px; font-weight: 400}
			)";
			download_hint_lab_->setStyleSheet(style);
			download_hint_lab_->adjustSize();
			hint_layout->addWidget(download_hint_lab_);

			retry_btn_ = new QPushButton(download_widget_);
			retry_btn_->setText(tcTr("id_upgrade_retry"));
			retry_btn_->setFixedSize(40, 16);
			retry_btn_->setStyleSheet(R"(
				QPushButton {border: none; font-family:Microsoft YaHei;font-size:12px; color:#4c72ff; text-decoration: underline; background-color:#ffffff;}
			")");
			retry_btn_->setCursor(QCursor(Qt::PointingHandCursor));

			exit_btn_ = new QPushButton(download_widget_);
			exit_btn_->setText(tcTr("id_upgrade_exit"));
			exit_btn_->setFixedSize(90, 16);
			exit_btn_->setStyleSheet(R"(
				QPushButton {border: none; font-family:Microsoft YaHei;font-size:12px; color:#4c72ff; text-decoration: underline; background-color:#ffffff;}
			")");
			exit_btn_->setCursor(QCursor(Qt::PointingHandCursor));
			hint_layout->addWidget(retry_btn_);
			hint_layout->addWidget(exit_btn_);

			retry_btn_->hide();
			exit_btn_->hide();

			main_vlayout->addSpacing(8);
			main_vlayout->addLayout(hint_layout);
		}

		{
			install_widget_ = new QWidget(stack_widget_);
			stack_widget_->addWidget(install_widget_);
			install_widget_->setFixedSize(320, 56);
			QHBoxLayout* hlayout = new QHBoxLayout(install_widget_);
			hlayout->setContentsMargins(0, 0, 0, 0);
			hlayout->setSpacing(0);
			hlayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			install_confirm_btn_ = new GDButton(install_widget_);
			GDButton::BorderInfo btn_border_info;
			GDButton::TextInfo btn_text_info;
			GDButton::IconInfo btn_icon_info;
			GDButton::BackgroundInfo btn_bk_info;
			btn_bk_info.m_background_color_normal = QColor(0x1A, 0x76, 0xF6);
			btn_bk_info.m_background_color_hover = QColor(0x1D, 0x79, 0xF9);
			btn_bk_info.m_background_color_press = QColor(0x1F, 0x7F, 0xFF);
			btn_bk_info.m_background_color_disable = QColor(0xcc, 0xcc, 0xcc);
			btn_text_info.m_blod = false;
			btn_text_info.m_font_color_normal = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_hover = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_press = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_font_color_disable = QColor(0xff, 0xff, 0xff);
			btn_text_info.m_text = tcTr("id_upgrade_install_now");
			btn_text_info.m_font_size = 15;
			btn_text_info.m_padding_left = 110;
			btn_text_info.m_padding_top = 28;
			btn_border_info.m_border_radius = 4;
			btn_border_info.m_border_width = 0;
			btn_icon_info.m_have_icon = false;
			install_confirm_btn_->Init(QSize(280, 44), btn_text_info, btn_bk_info, btn_icon_info,
				btn_border_info);

			hlayout->addStretch(1);
			hlayout->addWidget(install_confirm_btn_);
			hlayout->addStretch(1);
		}

		stack_widget_->setCurrentWidget(notify_update_widget_);
		
		main_vbox_layout->addSpacing(14);
		main_vbox_layout->addWidget(stack_widget_);
		main_vbox_layout->addSpacing(8);
	}

	void UpgradeHelperWidget::SetRemoteVersion(const QString& version) {
		QString msg = tcTr("id_upgrade_find_new_version") + ":  " + version;
		remote_version_lab_->setText(msg);
	}

	void UpgradeHelperWidget::SetRemoteUpdateDesc(const QString& desc) {
		text_edit_->setText(desc);
	}

	void UpgradeHelperWidget::InitSigChannel() {
		connect(cancel_btn_, &QPushButton::clicked, this, [this]() {
			this->done(QDialog::Rejected);
		});

		connect(exit_app_btn_, &QPushButton::clicked, this, [this]() {
			exit_app_ = true;
			this->done(QDialog::Rejected);
		});

		connect(confirm_btn_, &QPushButton::clicked, this, [this]() {
			UpdateManager::GetInstance()->Download();
			stack_widget_->setCurrentWidget(download_widget_);
		});

		connect(retry_btn_, &QPushButton::clicked, this, [this]() {
			UpdateManager::GetInstance()->Download();
			stack_widget_->setCurrentWidget(download_widget_);
		});

		connect(exit_btn_, &QPushButton::clicked, this, [this]() {
			this->done(QDialog::Rejected);
		});

		connect(install_confirm_btn_, &QPushButton::clicked, this, [this]() {
			UpdateManager::GetInstance()->OpenInstallFile();
		});

		connect(forced_confirm_btn_, &QPushButton::clicked, this, [this]() {
			UpdateManager::GetInstance()->Download();
			stack_widget_->setCurrentWidget(download_widget_);
		});

		connect(UpdateManager::GetInstance(), &UpdateManager::SigDownloadProgressValue, this, [this](int value) {
			if (stack_widget_->currentWidget() != download_widget_) {
				stack_widget_->setCurrentWidget(download_widget_);
			}
			if (!retry_btn_->isHidden() || !exit_btn_->isHidden()) {
				retry_btn_->hide();
				exit_btn_->hide();
			}
			download_hint_lab_->setText(tcTr("id_upgrade_downloading"));
            progress_bar_->setValue(value);
		});

		connect(UpdateManager::GetInstance(), &UpdateManager::SigDownloadComplete, this, [this](bool res, QString reson) {
			if (!res) {
				if (stack_widget_->currentWidget() != download_widget_) {
					stack_widget_->setCurrentWidget(download_widget_);
				}
                download_hint_lab_->setText(QStringLiteral("id_upgrade_down_error") + reson);
				retry_btn_->show();
				exit_btn_->show();
				return;
			}
			stack_widget_->setCurrentWidget(install_widget_);
		});

		connect(UpdateManager::GetInstance(), &UpdateManager::SigOpenInstallFileError, this, [this]() {
			TcDialog dialog(tcTr("id_tips"), tcTr("id_upgrade_cannot_open_file_for_exit"), this);
			dialog.exec();
			this->done(QDialog::Rejected);
		});
	}

	void UpgradeHelperWidget::mousePressEvent(QMouseEvent* event) {
		if (event->button() == Qt::LeftButton) {
			QWindow* window = windowHandle();
			if (window) {
				window->startSystemMove();
			}
		}
	}

	void UpgradeHelperWidget::ApplyWindowsShadow() {
		HWND hwnd = reinterpret_cast<HWND>(winId());

		if (!hwnd) return;

		const MARGINS shadowMargins = { 1, 1, 1, 1 };
		DwmExtendFrameIntoClientArea(hwnd, &shadowMargins);

		DWM_BLURBEHIND blurBehind = { 0 };
		blurBehind.dwFlags = DWM_BB_ENABLE;
		blurBehind.fEnable = TRUE;
		blurBehind.hRgnBlur = NULL;
		DwmEnableBlurBehindWindow(hwnd, &blurBehind);

		DWMNCRENDERINGPOLICY policy = DWMNCRP_ENABLED;
		DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY,
			&policy, sizeof(policy));

		BOOL compositionEnabled = TRUE;
		DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED,
			&compositionEnabled, sizeof(compositionEnabled));
	}

	//UpdateChecker
	std::string GetUpgradeRootAddr() {
		std::string upgrade_host = "127.0.0.1"; //GrSettings::Instance()->GetSpvrServerHost();
		std::string upgrade_addr = "http://" + upgrade_host + ":" + kUpgradeServerPort;
		return upgrade_addr;
	}

	UpdateManager::UpdateManager(QObject* parent) : QObject(parent) {
		
	}

	void UpdateManager::CheckUpdate(bool need_notify, bool from_user_clicked) {
		std::string root_addr = GetUpgradeRootAddr();

		LOGI("root_addr: {}", root_addr);

		QUrl url(QString::fromStdString(root_addr + kUpgradeQueryPath));
		QUrlQuery url_query;
		url_query.addQueryItem("page", "1");
		url_query.addQueryItem("page_size", "1");
		url_query.addQueryItem("sort_time", "-1");
		url.setQuery(url_query);

		QNetworkRequest request(url);
		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::AnyProtocol);
		request.setSslConfiguration(config);
		request.setHeader(QNetworkRequest::UserAgentHeader, "GammaRay");
		
		QPointer<QNetworkReply> reply = manager_.get(request);

		connect(reply, &QNetworkReply::finished, this, [thiz = QPointer(this), reply, need_notify, from_user_clicked]() {
			qDebug() << "OnCheckUpdateReplyFinished";
			LOGI("OnCheckUpdateReplyFinished");
			thiz->OnCheckUpdateReplyFinished(reply, need_notify, from_user_clicked);

			
		});
	}

	void UpdateManager::OnCheckUpdateReplyFinished(QPointer<QNetworkReply> reply, bool need_notify, bool from_user_clicked) {
		if (reply->error() != QNetworkReply::NoError) {
			emit SigGetUpdateConfigError("Network error: " + reply->errorString());
			LOGE("Network error: {}", reply->errorString().toStdString());
			reply->deleteLater();
			if (from_user_clicked) {
				emit SigUpdateHint(tcTr("id_upgrade_check_error") + tcTr("id_upgrade_network_error"));
			}
			return;
		}

		QByteArray data = reply->readAll();
		reply->deleteLater();

		QJsonParseError err;
		QJsonDocument doc = QJsonDocument::fromJson(data, &err);

		if (err.error != QJsonParseError::NoError) {
			emit SigGetUpdateConfigError("JSON parse error: " + err.errorString());
			LOGE("JSON parse error: {}", err.errorString().toStdString());
			if (from_user_clicked) {
				emit SigUpdateHint(tcTr("id_upgrade_check_error") + tcTr("id_upgrade_data_format_error") + QString("(%1)").arg(QString::number(1)));
			}
			return;
		}

		if (!doc.isObject()) {
			emit SigGetUpdateConfigError("Invalid JSON format");
			LOGE("Invalid JSON format");
			if (from_user_clicked) {
				emit SigUpdateHint(tcTr("id_upgrade_check_error") + tcTr("id_upgrade_data_format_error") + QString("(%1)").arg(QString::number(2)));
			}
			return;
		}

		QJsonObject obj = doc.object();
		// --- Safe extract fields with default values ---
		const int resp_code = obj.value("code").toInt();
		if (resp_code != kUpgradeApiOkValue) {
			QString msg = "check upgrade error, code: " + QString::number(resp_code) + "; msg: " + obj.value("message").toString();
			emit SigGetUpdateConfigError(msg);
			if (from_user_clicked) {
				emit SigUpdateHint(msg);
			}
			return;
		}

		QString version;
		QString down_path;
		QString file_md5;
		QString file_name;
		QString desc;
		bool forced = false;

		if (obj.contains("data") && obj.value("data").isArray()) {
			QJsonArray data_array = obj.value("data").toArray();
			for (const QJsonValue& value : data_array) {
				if (value.isObject()) {
					QJsonObject obj = value.toObject();
					desc = obj.value("desc").toString("");
					version = obj.value("version").toString("");
					down_path = obj.value("down_addr").toString("");
					file_md5 = obj.value("file_md5").toString("");
					file_name = obj.value("file_name").toString("");
					forced = obj.value("forced").toBool(false);
					file_size_ = obj.value("file_size").toInt();
					break;
				}
			}
		}

		// --- Check required fields ---
		if (version.isEmpty() || down_path.isEmpty() || file_md5.isEmpty()) {
			emit SigGetUpdateConfigError("Invalid json: missing required fields");
			LOGE("Invalid json: missing required fields");
			if (from_user_clicked) {
				emit SigUpdateHint(tcTr("id_upgrade_check_error") + tcTr("id_upgrade_data_format_error") + QString("(%1)").arg(QString::number(3)));
			}
			return;
		}

		auto desc_list = desc.split("###");
		QString update_desc;
		for (auto info : desc_list) {
			update_desc = update_desc + info + "\n";
		}

		// Build a map and emit
		QVariantMap result;
		result["version"] = version;
		result["down_path"] = down_path;
		result["down_file_md5"] = file_md5;
		result["desc"] = update_desc;
		result["forced"] = forced;
		qDebug() << "UpdateChecker::onReplyFinished: " << result;

		download_url_ = QString::fromStdString(GetUpgradeRootAddr() + kUpgradeDownloadPath);
		remote_file_md5_ = file_md5;
		file_name_ = file_name;

		if (get_remote_update_version_callback_func_) {
			get_remote_update_version_callback_func_(version);
		}

		ParseUpdateConfig(result, need_notify, from_user_clicked);
	}

	// version1 == version2 return 0;  version1 > version2 return 1; version1 < version2 return -1;
	int UpdateManager::CompareVersion(const QString& version1, const QString& version2)
	{
		QStringList parts1 = version1.split('.');
		QStringList parts2 = version2.split('.');
		int numParts = qMax(parts1.size(), parts2.size());
		for (int i = 0; i < numParts; ++i) {
			int part1 = (i < parts1.size()) ? parts1[i].toInt() : 0;
			int part2 = (i < parts2.size()) ? parts2[i].toInt() : 0;

			if (part1 < part2) {
				return -1;
			}
			else if (part1 > part2) {
				return 1;
			}
		}
		return 0;
	}

	void UpdateManager::ParseUpdateConfig(const QVariantMap& data, bool need_notify, bool from_user_clicked) {
		int res = CompareVersion(data["version"].toString(), PROJECT_VERSION);
		if (res <= 0) {
			if (from_user_clicked) {
				emit SigUpdateHint(tcTr("id_upgrade_latest"));
			}
			return;
		}
		if (need_notify) {
			emit SigFindUpdate(data); 
		}
	}

	void UpdateManager::Download() {
		if (download_url_.isEmpty()) {
			return;
		}
		
		QString dir_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
		QDir dir{ dir_path };
        if (!dir.exists()) {
			if (!dir.mkpath(".")) {
				qWarning() << "Failed to create directories for file: " << dir_path;
				LOGE("Failed to create directories for file:", dir_path.toStdString());
				emit SigDownloadComplete(false, tcTr("id_upgrade_failed_create_dir"));
				return;
			}
		}
		QString save_path = dir_path + "/" + file_name_;
		save_path_ = save_path;

		QPointer<QFile> file_ptr = new QFile(save_path);
		if (!file_ptr->open(QIODevice::WriteOnly)) {
			qDebug() << "open " << "" << "error";
			LOGE("Failed to open file: {}", save_path.toStdString());
			emit SigDownloadComplete(false, tcTr("id_upgrade_failed_open_file"));
			return;
		}
		QUrl url(download_url_);
		QUrlQuery url_query;
		url_query.addQueryItem("down_file", file_name_);
		url.setQuery(url_query);
		QNetworkRequest request(url);

		QSslConfiguration config;
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		config.setProtocol(QSsl::AnyProtocol);
		request.setSslConfiguration(config);
		QPointer<QNetworkReply> reply = manager_.get(request);
		
		//add timer for timeout
		QPointer<QTimer> timer = new QTimer(this);
		timer->setSingleShot(true);
		std::shared_ptr<gd::MD5> file_md5_ptr = std::make_shared<gd::MD5>();
		connect(timer, &QTimer::timeout, this, [reply, timer, this]() {
			disconnect(reply, nullptr, nullptr, nullptr);
			reply->abort();
			reply->close();
			reply->deleteLater();
			timer->deleteLater();
			emit SigDownloadComplete(false, tcTr("id_upgrade_time_out"));
		});

		connect(reply, &QNetworkReply::finished, this, [timer, file_ptr, reply, file_md5_ptr, this]() {
			if (timer->isActive())
			{
				timer->stop();
			}
			auto part_content = reply->readAll();
			file_ptr->write(part_content);
			file_ptr->close();
			reply->deleteLater();
			timer->deleteLater();
			file_md5_ptr->update(part_content.data(), part_content.size());
			const QString file_md5_value = QString::fromStdString(file_md5_ptr->toString());
			bool res = file_md5_value == this->remote_file_md5_;
			emit SigDownloadComplete(res, res ? QStringLiteral("") : tcTr("id_upgrade_file_corrupted"));
		});

		connect(reply, &QIODevice::readyRead, this, [file_ptr, reply, file_md5_ptr]() {
			auto part_content = reply->readAll();
			file_ptr->write(part_content);
			file_md5_ptr->update(part_content.data(), part_content.size());
		});

		connect(reply, &QNetworkReply::downloadProgress, this, [=](qint64 bytesReceived, qint64 bytesTotal) {
			auto size = bytesTotal > 0 ? bytesTotal : file_size_;
			if (size > 0) {
				emit SigDownloadProgressValue(100 * (bytesReceived * 1.0 / size));
			}
		});

		const int kTimeout = 180;
		timer->start(std::chrono::seconds(kTimeout));
	}

	void UpdateManager::OpenInstallFile() {
		QString work_dir = QFileInfo(save_path_).absolutePath();
		bool res = QProcess::startDetached("cmd.exe", {"/c", "start", save_path_}, work_dir);
		if (!res) {
			emit SigOpenInstallFileError();
		}
	}
}