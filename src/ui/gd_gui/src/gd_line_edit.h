#pragma once

#include <qpushbutton.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <QWidget>
#include <QLineEdit>
#include <qstring.h>
#include <qsize.h>
#include <qevent.h>
#include <qpainter.h>
#include <qcolor.h>

class QHBoxLayout;
class GDIconButton;

class GDLineEdit : public QLineEdit {
	Q_OBJECT
public:
	struct BorderInfo {
		int m_border_radius = 0;
		int m_border_width = 0;
		QColor m_normal_color;
		QColor m_hover_color;
		QColor m_focus_color;
	};

	struct IconInfo {
		QSize m_left_size;
		int m_padding_left = 0;
		QString m_left_icon_normal;
		QString m_left_icon_hover;
		QString m_left_icon_press;
		bool m_have_left_icon = false;

		QSize m_right_size;
		int m_padding_right = 0;
		QString m_right_icon_normal;
		QString m_right_icon_hover;
		QString m_right_icon_press;
		bool m_have_right_icon = false;
	};

	struct TextInfo {
		int m_font_size = 0;
		QString m_font_color;
		int m_padding_left = 0;
		int m_padding_right = 0;
	};

	GDLineEdit(QWidget* parent = nullptr);
	~GDLineEdit() = default;

	void Init(const QSize& size, const QString& placeholderText, BorderInfo border_info, IconInfo icon_info, TextInfo text_info, const QColor& brackground_color);

	virtual void paintEvent(QPaintEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	virtual void enterEvent(QEnterEvent* event) override;
#else
	virtual void enterEvent(QEvent* event) override;
#endif

	virtual void focusInEvent(QFocusEvent* event) override;
	virtual void focusOutEvent(QFocusEvent* event) override;


	void HideLeftIcon();

	void HideRightIcon();

    void ShowLeftIcon();

	void ShowRightIcon();

	GDIconButton* m_btn_left = nullptr;
	GDIconButton* m_btn_right = nullptr;
signals:
    void SigClickedLeftIcon();
    void SigClickedRightIcon();
private:
	QHBoxLayout* m_hbox_main_layout = nullptr;
	

	BorderInfo m_border_info;
	IconInfo m_icon_info;
	TextInfo m_text_info;

	bool m_cursor_in = false;

	bool m_focus_in = false;

	QColor m_brackground_color;
};
