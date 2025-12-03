#include "gd_progress_bar.h"

#include <qstyle.h>

GDProgressBar::GDProgressBar(QWidget* parent) : QSlider(parent){

}

GDProgressBar::~GDProgressBar() {

}

void GDProgressBar::paintEvent(QPaintEvent* event) {
	QSlider::paintEvent(event);

}

void GDProgressBar::Init() {

}


void GDProgressBar::mouseReleaseEvent(QMouseEvent* event) {
	setValue(QStyle::sliderValueFromPosition(this->minimum(), this->maximum(), event->x(), this->width()));
	emit SigPosChanged();
}