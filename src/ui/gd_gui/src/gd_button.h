#pragma once

#include <qpushbutton.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <QWidget>
#include <qpixmap.h>
#include <qpainter.h>
#include <QtSvg/QSvgRenderer>

class GDButton : public QPushButton {
	Q_OBJECT
public:
	GDButton(QWidget* parent = nullptr);
	~GDButton() = default;

	struct BorderInfo {
		int m_border_radius = 0;
		int m_border_width = 0;
		QColor m_color_normal;
		QColor m_color_hover;
		QColor m_color_press;
		QColor m_color_disable;
	};

	struct TextInfo {
		int m_font_size;
		int m_padding_left = 0;
		int m_padding_top = 0;
		bool m_blod = false;
		QColor m_font_color_normal;
		QColor m_font_color_hover;
		QColor m_font_color_press;
		QColor m_font_color_disable;
		QString m_text;
	};

	struct BackgroundInfo {
		QColor m_background_color_normal;
		QColor m_background_color_hover;
		QColor m_background_color_press;
		QColor m_background_color_disable;
	};

	struct IconInfo {
		bool m_have_icon = false;
		int m_padding_left = 0;
		int m_padding_top = 0;
		QSize m_icon_size;
		QString m_icon_normal;
		QString m_icon_hover;
		QString m_icon_press;
		QString m_icon_disable;
	};

	void Init(const QSize& size, TextInfo text_info, BackgroundInfo background_info, IconInfo icon_info, BorderInfo border_info);

	virtual void paintEvent(QPaintEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	virtual void enterEvent(QEnterEvent* event) override;
#else
	virtual void enterEvent(QEvent* event) override;
#endif
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
private:
	QPixmap m_pixmap_normal;
	QPixmap m_pixmap_hover;
	QPixmap m_pixmap_press;
	QPixmap m_pixmap_disable;

	TextInfo m_text_info;
	BackgroundInfo m_background_info;
	IconInfo m_icon_info;
	BorderInfo m_border_info;

	bool m_cursor_in = false;
	bool m_pressed = false;

private:
	QSvgRenderer m_svg_renderer;
	bool m_use_svg = false;
};
