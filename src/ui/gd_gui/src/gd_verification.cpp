#include "gd_verification.h"
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QRandomGenerator>

// 替换 qsrand(seed)
static void initializeRandomSeed(unsigned int seed) {
    QRandomGenerator::global()->seed(seed);
}

// 替换 qrand()
static int generateRandomNumber() {
    return QRandomGenerator::global()->bounded(INT_MAX); // 生成 0 到 INT_MAX 的随机数
}

// 替换 qrand() % range
static int generateRandomNumberInRange(int range) {
    return QRandomGenerator::global()->bounded(range); // 生成 0 到 range-1 的随机数
}

GDVerificationCode::GDVerificationCode(bool draw_dot, int code_num, QWidget* parent)
    : draw_dot_(draw_dot), code_num_(code_num), QWidget(parent)
{
    colors_ = new QColor[code_num_]; // 改用 QColor
    initializeRandomSeed(QTime::currentTime().second() * 1000 + QTime::currentTime().msec());
    verification_code_ = GetVerificationCodeByRand();
    UpdateColors();
}

GDVerificationCode::~GDVerificationCode()
{
    delete[] colors_;
}

QString GDVerificationCode::GetVerificationCodeByRand()
{
    initializeRandomSeed(QTime::currentTime().second() * 1000 + QTime::currentTime().msec());
    QString destCode;
    for (int i = 0; i < code_num_; i++) {
        int flag = generateRandomNumberInRange(2);
        if (flag == 0) {
            int c = '0' + generateRandomNumberInRange(10);
            destCode += static_cast<QChar>(c);
        }
        else {
            int c = (generateRandomNumberInRange(2)) ? 'a' : 'A';
            destCode += static_cast<QChar>(c + generateRandomNumberInRange(26));
        }
    }
    return destCode;
}

void GDVerificationCode::UpdateColors()
{
    for (int i = 0; i < code_num_; ++i) {
        int red = 50 + generateRandomNumberInRange(206);   // 确保颜色值在范围 [50, 255]
        int green = 50 + generateRandomNumberInRange(206);
        int blue = 50 + generateRandomNumberInRange(206);
        colors_[i] = QColor(red, green, blue); // 使用 QColor 生成颜色
    }
}

void GDVerificationCode::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    // 填充验证码绘制矩形
    painter.fillRect(0, 0, width(), height(), QColor(255, 250, 240));
    painter.setFont(QFont("Comic Sans MS", height() / 2));

    // 动态计算每个字符的绘制位置
    int charWidth = width() / code_num_;
    int charHeight = height();

    for (int i = 0; i < code_num_; i++) {
        painter.setPen(colors_[i]); // 使用优化后的颜色
        QRect charRect(i * charWidth, 0, charWidth, charHeight);
        painter.drawText(charRect, Qt::AlignCenter, QString(verification_code_[i]));
    }

    // 绘制噪点
    if (draw_dot_) {
        paintDot(&painter);
    }
}

void GDVerificationCode::paintDot(QPainter* painter)
{
    if (painter == nullptr)
        return;

    int widgetWidth = width();
    int widgetHeight = height();

    for (int i = 0; i < 150; i++) {
        // 为噪点生成更深的颜色
        int red = generateRandomNumberInRange(150);   // 噪点颜色范围 [0, 150]
        int green = generateRandomNumberInRange(150);
        int blue = generateRandomNumberInRange(150);
        QColor dotColor(red, green, blue);
        dotColor.setAlpha(200); // 增加不透明度，确保颜色更加清晰

        painter->setPen(dotColor);
        int x = generateRandomNumberInRange(widgetWidth);
        int y = generateRandomNumberInRange(widgetHeight);
        painter->drawPoint(x, y);
    }
}

void GDVerificationCode::mousePressEvent(QMouseEvent* event)
{
    verification_code_ = GetVerificationCodeByRand();
    UpdateColors();
    update();
    QWidget::mousePressEvent(event);
}