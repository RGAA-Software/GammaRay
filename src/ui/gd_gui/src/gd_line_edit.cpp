#include "gd_line_edit.h"

#include <qpen.h>
#include <qbrush.h>
#include <qboxlayout.h>
#include "GD_icon_button.h"

GDLineEdit::GDLineEdit(QWidget* parent) : QLineEdit(parent) {

}

void GDLineEdit::Init(const QSize& size, const QString& placeholderText, BorderInfo border_info, IconInfo icon_info, TextInfo text_info, const QColor& brackground_color) {
	m_border_info = border_info;
	m_icon_info = icon_info;
	m_text_info = text_info;
	m_brackground_color = brackground_color;

	setAttribute(Qt::WA_StyledBackground);
	setMouseTracking(true);

	if (size.width() > 0) {
		setFixedWidth(size.width());
	}

	if (size.height() > 0) {
		setFixedHeight(size.height());
	}

	m_hbox_main_layout = new QHBoxLayout();
	m_hbox_main_layout->setSpacing(0);
	m_hbox_main_layout->setAlignment(Qt::AlignLeft);
	m_hbox_main_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_hbox_main_layout);

	if (icon_info.m_have_left_icon) {
		m_btn_left = new GDIconButton(this);
		m_btn_left->Init(icon_info.m_left_size, 0, 0, icon_info.m_left_icon_normal, icon_info.m_left_icon_hover, icon_info.m_left_icon_press);
		m_hbox_main_layout->addSpacing(icon_info.m_padding_left);
		m_hbox_main_layout->addWidget(m_btn_left);
	}

	if (icon_info.m_have_right_icon) {
		m_btn_right = new GDIconButton(this);
		m_btn_right->Init(icon_info.m_right_size, 0, 0, icon_info.m_right_icon_normal, icon_info.m_right_icon_hover, icon_info.m_right_icon_press);
		m_hbox_main_layout->addWidget(m_btn_right, 1, Qt::AlignRight);
		m_hbox_main_layout->addSpacing(icon_info.m_padding_right);

		connect(m_btn_right, &QPushButton::clicked, this, [=, this]() {
			emit SigClickedRightIcon();
		});
	}
	
	QPalette palette;
	palette.setColor(QPalette::PlaceholderText, QColor(0xaf, 0xb3, 0xbc));
	setPlaceholderText(placeholderText);
	
	QString css_str = QString("QLineEdit {border-width:0;border-style:outset; background-color: #00000000; \
	font-family: Microsoft YaHei; font-size: %1px; padding-left: %2px; color: %3; padding-right: %4px;}").arg(m_text_info.m_font_size).
		arg(m_text_info.m_padding_left).arg(m_text_info.m_font_color).arg(m_text_info.m_padding_right);

	setStyleSheet(css_str);
}

void GDLineEdit::HideLeftIcon() {
	if (m_btn_left) {
		m_btn_left->hide();
	}
}

void GDLineEdit::HideRightIcon() {
	if (m_btn_right) {
		m_btn_right->hide();
	}
}

void GDLineEdit::ShowLeftIcon() {
	if (m_btn_left) {
		m_btn_left->show();
	}
}

void GDLineEdit::ShowRightIcon() {
	if (m_btn_right) {
		m_btn_right->show();
	}
}

void GDLineEdit::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	QPen pen;
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(m_border_info.m_border_width);
	if (m_focus_in) {
		pen.setColor(m_border_info.m_focus_color);
	}
	else {
		if (m_cursor_in) {
			pen.setColor(m_border_info.m_hover_color);
		}
		else {
			pen.setColor(m_border_info.m_normal_color);
		}
	}
	painter.setPen(pen);
	painter.setBrush(QBrush(m_brackground_color));
	painter.drawRoundedRect(this->rect(), m_border_info.m_border_radius, m_border_info.m_border_radius);
	QLineEdit::paintEvent(event);
}

void GDLineEdit::leaveEvent(QEvent* event) {
	QLineEdit::leaveEvent(event);
	m_cursor_in = false;
	repaint();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void GDLineEdit::enterEvent(QEnterEvent* event)
#else
void GDLineEdit::enterEvent(QEvent* event)
#endif
{
	QLineEdit::enterEvent(event);
	m_cursor_in = true;
	repaint();
}

void GDLineEdit::focusInEvent(QFocusEvent* event) {
	QLineEdit::focusInEvent(event);
	m_focus_in = true;
	repaint();
}

void GDLineEdit::focusOutEvent(QFocusEvent* event) {
	QLineEdit::focusOutEvent(event);
	m_focus_in = false;
	repaint();
}

