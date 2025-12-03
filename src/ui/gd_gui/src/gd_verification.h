#pragma once

#include <QWidget>
#include <qcolor.h>

class GDVerificationCode : public QWidget
{
	Q_OBJECT
public:
	GDVerificationCode(bool draw_dot, int code_num, QWidget* parent = nullptr);
	~GDVerificationCode();
	QString GetVerificationCode() { 
		return verification_code_; 
	}
private:
	QString GetVerificationCodeByRand();
	void UpdateColors();
private:
	virtual void paintEvent(QPaintEvent* event);
	void paintDot(QPainter* painter);

	virtual void mousePressEvent(QMouseEvent* event) override;

private:
	QString verification_code_;
	int code_num_ = 4;
	bool draw_dot_ = true;
	QColor* colors_ = nullptr;
};