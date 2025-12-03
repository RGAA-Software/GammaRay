#include "gd_icon_button.h"
#include <qdebug.h>

GDIconButton::GDIconButton(QWidget* parent) : QPushButton(parent) {
	setMouseTracking(true);
	setFocusPolicy(Qt::NoFocus);
	setAttribute(Qt::WA_TranslucentBackground, true);
}

void GDIconButton::Init(QSize size, int padding, int radius, const QString& normal_img_path, const QString& hover_img_path, const QString& click_img_path) {
	setFixedSize(size);
	padding_ = padding;
	radius_ = radius;

	m_normal_img_path_1 = normal_img_path;
	m_hover_img_path_1 = hover_img_path;
	m_press_img_path_1 = click_img_path;

	m_normal_pixmap_1 = QPixmap(m_normal_img_path_1);
	m_hover_pixmap_1 = QPixmap(m_hover_img_path_1);
	m_press_img_pixmap_1 = QPixmap(m_press_img_path_1);
}


void GDIconButton::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	if (m_user_bg) {
		painter.save();
		BackgroundInfo background_info = m_background_info_1;
		if (show_back_info_2_) {
			background_info = m_background_info_2;
		}
		painter.setPen(Qt::NoPen);
		if (isChecked()) {
			painter.setBrush(QBrush(background_info.m_background_color_checked));
		}
		else {
			if (m_pressed) {
				painter.setBrush(QBrush(background_info.m_background_color_press));
			}
			else {
				if (m_cursor_in) {
					painter.setBrush(QBrush(background_info.m_background_color_hover));
				}
				else {
					painter.setBrush(QBrush(background_info.m_background_color_normal));
				}
			}
		}
		painter.drawRoundedRect(this->rect(), radius_, radius_);
		painter.restore();
	}

	if (m_cursor_in) {
		if (m_pressed) {
			if (m_show_pixmap_1) {
				if (m_use_svg) {
					m_svg_renderer.load(m_press_img_path_1);
					m_svg_renderer.render(&painter, QRectF(padding_, padding_, this->width() - padding_ * 2, this->height() - padding_ * 2));
				}
				else {
					painter.drawPixmap(0, 0, m_press_img_pixmap_1);
				}
			}
			else {
				if (m_use_svg) {
					m_svg_renderer.load(m_press_img_path_2);
					m_svg_renderer.render(&painter, QRectF(padding_, padding_, this->width() - padding_ * 2, this->height() - padding_ * 2));
				}
				else {
					painter.drawPixmap(0, 0, m_press_img_pixmap_2);
				}
			}
		}
		else {
			if (m_show_pixmap_1) {
				if (m_use_svg) {
					m_svg_renderer.load(m_hover_img_path_1);
					m_svg_renderer.render(&painter, QRectF(padding_, padding_, this->width() - padding_ * 2, this->height() - padding_ * 2));
				}
				else {
					painter.drawPixmap(0, 0, m_hover_pixmap_1);
				}
				
			}
			else {
				if (m_use_svg) {
					m_svg_renderer.load(m_hover_img_path_2);
					m_svg_renderer.render(&painter, QRectF(padding_, padding_, this->width() - padding_ * 2, this->height() - padding_ * 2));
				}
				else {
					painter.drawPixmap(0, 0, m_hover_pixmap_2);
				}
			}
		}
	}
	else {
		if (m_show_pixmap_1) {
			if (m_use_svg) {
				m_svg_renderer.load(m_normal_img_path_1);
				m_svg_renderer.render(&painter, QRectF(padding_, padding_, this->width() - padding_ * 2, this->height() - padding_ * 2));
			}
			else {
				painter.drawPixmap(0, 0, m_normal_pixmap_1);
			}
			
		}
		else {
			if (m_use_svg) {
				m_svg_renderer.load(m_normal_img_path_2);
				m_svg_renderer.render(&painter, QRectF(padding_, padding_, this->width() - padding_ * 2, this->height() - padding_ * 2));
			}
			else {
				painter.drawPixmap(0, 0, m_normal_pixmap_2);
			}
		}
	}
}

void GDIconButton::set_pixmap_2(const QString& normal_img_path, const QString& hover_img_path, const QString& click_img_path) {
	m_normal_img_path_2 = normal_img_path;
	m_hover_img_path_2 = hover_img_path;
	m_press_img_path_2 = click_img_path;

	m_normal_pixmap_2 = QPixmap(m_normal_img_path_2);
	m_hover_pixmap_2 = QPixmap(m_hover_img_path_2);
	m_press_img_pixmap_2 = QPixmap(m_press_img_path_2);
}

void GDIconButton::show_pixmap_1() {
	m_show_pixmap_1 = true;
	m_show_pixmap_2 = false;
}

void GDIconButton::show_pixmap_2() {
	m_show_pixmap_2 = true;
	m_show_pixmap_1 = false;
}

void GDIconButton::ShowBackgroundInfo1() {
	show_back_info_1_ = true;
	show_back_info_2_ = false;
}

void GDIconButton::ShowBackgroundInfo2() {
	show_back_info_1_ = false;
	show_back_info_2_ = true;
}

void GDIconButton::mousePressEvent(QMouseEvent* event) {
	event->accept();
	m_pressed = true;
	repaint();
	QPushButton::mousePressEvent(event);
}

void GDIconButton::mouseReleaseEvent(QMouseEvent* event) {
	event->accept();
	m_pressed = false;
	repaint();
	QPushButton::mouseReleaseEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void GDIconButton::enterEvent(QEnterEvent* event)
#else
void GDIconButton::enterEvent(QEvent* event)
#endif
{
	m_cursor_in = true;
	repaint();
	QPushButton::enterEvent(event);
}

void GDIconButton::leaveEvent(QEvent* event) {
	m_cursor_in = false;
	repaint();
	QPushButton::leaveEvent(event);
}

void GDIconButton::SetUseSvg(bool use_svg) {
	m_use_svg = use_svg;
}

void GDIconButton::SetBackgroundInfo(const BackgroundInfo& bg_info) {
	m_background_info_1 = bg_info;
	m_user_bg = true;
}

void GDIconButton::SetBackgroundInfo2(const BackgroundInfo& bg_info) {
	m_background_info_2 = bg_info;
	m_user_bg = true;
}

void GDIconButton::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Space) {
		event->accept(); // In this way, the event will not be passed to the parent class, and ignore will continue to pass it
		return;
	}
	QPushButton::keyPressEvent(event);
}
