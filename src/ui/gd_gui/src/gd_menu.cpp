#include "gd_menu.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qmath.h>

namespace
{
    const int64_t SHADOW_WIDTH = 5;
    const int64_t RADIUS = 12;
	const QString QSS = "QMenu{background-color:transparent;margin:10px;}";
}

GDMenu::GDMenu(QWidget* parent) :QMenu(parent)
{
    init();
}

void GDMenu::init()
{
	// FramelessWindowHint and WA_TranslucentBackground will cause addAction(QAction) to fail (hover animation will fail)
    setAttribute(Qt::WA_TranslucentBackground);    
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	setAutoFillBackground(true);
	setStyleSheet(QSS);
}

void GDMenu::paintEvent(QPaintEvent* event)
{
    QMenu::paintEvent(event);
	QPainter painter(this);
	QColor m_defaultBackgroundColor = qRgb(65, 65, 65);
	QColor m_defaultBorderColor = qRgb(69, 69, 69);
	QPainterPath path;
	path.setFillRule(Qt::WindingFill);
	path.addRoundedRect(10, 10, this->width() - 20, this->height() - 20, 5, 5);

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.fillPath(path, QBrush(QColor(m_defaultBackgroundColor.red(),
		m_defaultBackgroundColor.green(),
		m_defaultBackgroundColor.blue())));

	QColor color(45, 45, 45, 50);
	for (int i = 0; i < 5; i++)
	{
		QPainterPath path;
		path.setFillRule(Qt::WindingFill);
		path.addRoundedRect(5 - i, 5 - i, this->width() - (5 - i) * 2, this->height() - (5 - i) * 2, 5, 5);
		color.setAlpha(100 - qSqrt(i) * 50);
		painter.setPen(color);
		painter.drawPath(path);
	}
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(QBrush(Qt::white));
	painter.setPen(Qt::transparent);
	QRect rect = this->rect();
	rect.setX(5);
	rect.setY(5);
	rect.setWidth(rect.width() - 5);
	rect.setHeight(rect.height() - 5);
	painter.drawRoundedRect(rect, 5, 5);
}

void GDMenu::hideEvent(QHideEvent* event) {
	emit SigHide();
	QMenu::hideEvent(event);
}
