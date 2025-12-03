#include <iostream>
#include <qapplication.h>
#include <qwidget.h>
#include "GD_button.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	QWidget* w = new QWidget();
	w->resize(800, 600);
	w->show();

	auto btn = new GDButton(w);

	app.exec();
	return 0;
}