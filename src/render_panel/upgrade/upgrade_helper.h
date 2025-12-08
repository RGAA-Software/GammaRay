#pragma once
#include <qwidget.h>
#include <qevent.h>
#include <qstackedwidget.h>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <qpointer.h>
#include <qdialog.h>

class QLabel;
class QTextEdit;
class QPushButton;
class GDButton;
class GDCustomProgressBar;

namespace tc {

	using GetRemoteUpdateVersionCallbackFuncType = std::function<void(const QString&)>;

	class UpgradeHelperWidget : public QDialog { 
        Q_OBJECT
	public:
		UpgradeHelperWidget(QWidget* parent = nullptr);
		~UpgradeHelperWidget();
		void mousePressEvent(QMouseEvent* event) override;
		void paintEvent(QPaintEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;
		void closeEvent(QCloseEvent* event) override;

		void SetRemoteVersion(const QString& version);
		void SetRemoteUpdateDesc(const QString& desc);
		void SetForced(bool f);
		bool exit_app_ = false;
		bool need_exit_ = false;
	private:
		void InitUI();
		void InitSigChannel();
		void ApplyWindowsShadow();
	
	private:
		QLabel* remote_version_lab_ = nullptr;
        QTextEdit* text_edit_ = nullptr;

		QStackedWidget* stack_widget_ = nullptr;

		QWidget* notify_update_widget_ = nullptr;
		GDButton* cancel_btn_ = nullptr;
		GDButton* confirm_btn_ = nullptr;

		QWidget* forced_update_widget_ = nullptr;
		GDButton* exit_app_btn_ = nullptr;
        GDButton* forced_confirm_btn_ = nullptr;

		QWidget* download_widget_ = nullptr;
		GDCustomProgressBar* progress_bar_ = nullptr;
		QLabel* download_hint_lab_ = nullptr;
		QPushButton* retry_btn_ = nullptr;
		QPushButton* exit_btn_ = nullptr;

		QWidget* install_widget_ = nullptr;
		GDButton* install_confirm_btn_ = nullptr;

		bool forced_ = false;
	};

	class UpdateManager : public QObject
	{
		Q_OBJECT
	public:
		static UpdateManager* GetInstance() {
			static UpdateManager self;
			return &self;
		}

		explicit UpdateManager(QObject* parent = nullptr);

		void CheckUpdate(bool need_notify = true, bool from_user_clicked = false);

		void Download();

		void OpenInstallFile();

		GetRemoteUpdateVersionCallbackFuncType get_remote_update_version_callback_func_ = nullptr;
	signals:
		void SigGetUpdateConfigError(const QString& error);
		void SigFindUpdate(const QVariantMap& data);
		void SigDownloadProgressValue(int progress);
		void SigDownloadComplete(bool res, QString reson);
		void SigOpenInstallFileError();
		void SigUpdateHint(QString info);
	private:
		void OnCheckUpdateReplyFinished(QPointer<QNetworkReply> reply, bool need_notify = true, bool from_user_clicked = false);
		void ParseUpdateConfig(const QVariantMap& data, bool need_notify = true, bool from_user_clicked = false);
		int CompareVersion(const QString& version1, const QString& version2);
	private:
		QNetworkAccessManager manager_;
		QString download_url_;
		QString remote_file_md5_;
		QString file_name_;
		QString save_path_;
		uint64_t file_size_ = 0;
	};
	
}