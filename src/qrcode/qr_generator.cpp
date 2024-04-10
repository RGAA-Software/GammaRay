//
// Created by hy on 2024/4/8.
//

#include "qr_generator.h"
#include "qrcodegen.hpp"

using namespace qrcodegen;

namespace tc
{

    QPixmap QrGenerator::GenQRPixmap(const QString &message, int qr_size) {
        // Manual operation
        std::vector<QrSegment> segs =
                QrSegment::makeSegments(message.toStdString().c_str());
        QrCode qr1 = QrCode::encodeSegments(
                segs, QrCode::Ecc::QUARTILE, 15, 15, 2, false);

        QImage QrCode_Image=QImage(qr1.getSize(),qr1.getSize(),QImage::Format_RGB888);
        QrCode_Image.fill(Qt::transparent);
        for (int y = 0; y < qr1.getSize(); y++) {
            for (int x = 0; x < qr1.getSize(); x++) {
                //getModule(x, y)会返回指定位置的颜色 false为白色，true为黑色
                if(qr1.getModule(x, y))
                    QrCode_Image.setPixel(x,y,qRgb(0,0,0));
                else
                    QrCode_Image.setPixel(x,y,qRgb(255,255,255));
            }
        }

        //图像大小转换为适当的大小
        QrCode_Image=QrCode_Image.scaled(qr_size, qr_size, Qt::KeepAspectRatio);
        //转换为QPixmap在Label中显示
        return QPixmap::fromImage(QrCode_Image);
    }

}
