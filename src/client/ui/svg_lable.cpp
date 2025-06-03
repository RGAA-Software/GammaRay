#include "svg_lable.h"

namespace tc {

	void SvgLable::paintEvent(QPaintEvent* event) {
		QPainter painter(this);
		svg_renderer_.load(svg_path_);
		svg_renderer_.render(&painter, QRectF(0, 0, this->width(), this->height()));
	}

}