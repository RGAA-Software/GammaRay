#pragma once

#include <qlabel.h>
#include <qpainter.h>
#include <QtSvg/QSvgRenderer>

namespace tc {

	class SvgLable : public QLabel {
	public:
		SvgLable(QString svg_path, QWidget* parent = nullptr) : QLabel(parent), svg_path_(svg_path){}
		void paintEvent(QPaintEvent* event) override;

	private:
		QString svg_path_;
		QSvgRenderer svg_renderer_;
	};
}