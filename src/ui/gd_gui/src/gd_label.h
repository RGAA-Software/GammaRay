#pragma once

#include <qlabel.h>
#include <qwidget.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qsize.h>

class GDLabel : public QLabel {
	Q_OBJECT
public:
	struct BorderInfo {
		int m_radius = 0;
		int m_width = 0;
		QColor m_color;
	};

	struct TextInfo {
		QString m_text;
		QColor m_color;
		int m_size = 0;
		int m_padding_left = 0;
		int m_padding_top = 0;
		bool m_blod = false;
	};

	GDLabel(QWidget * parent = nullptr);
	~GDLabel() = default;
	void SetText(const QString& text);
	QFont GetFont(const TextInfo& text_info);

	void Init(QSize size, const BorderInfo& border_info, const TextInfo& text_info, const QColor& background_color);
	void Init2(const BorderInfo& border_info, const TextInfo& text_info, const QColor& background_color);
	void paintEvent(QPaintEvent* event) override;

	QFont m_font;
private:
	BorderInfo m_border_info;
	TextInfo m_text_info;
	QColor m_background_color;
};