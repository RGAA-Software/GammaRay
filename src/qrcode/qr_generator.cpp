//
// Created by RGAA on 2024/4/8.
//

#include "qr_generator.h"
#include "qrcodegen.hpp"

using namespace qrcodegen;

namespace tc
{

    QPixmap QrGenerator::GenQRPixmap(const QString &message, int qr_size) {
        std::vector<QrSegment> segs = QrSegment::makeSegments(message.toStdString().c_str());
        // QrCode qr1 = QrCode::encodeSegments(segs, QrCode::Ecc::QUARTILE, 15, 15, 2, false);
        QrCode qr1 = QrCode::encodeSegments(segs, QrCode::Ecc::LOW, 10, 10, 1, false);
        QImage QrCode_Image=QImage(qr1.getSize(),qr1.getSize(),QImage::Format_RGB888);
        QrCode_Image.fill(Qt::transparent);
        for (int y = 0; y < qr1.getSize(); y++) {
            for (int x = 0; x < qr1.getSize(); x++) {
                if(qr1.getModule(x, y)) {
                    QrCode_Image.setPixel(x, y, qRgb(0, 0, 0));
                } else {
                    QrCode_Image.setPixel(x, y, qRgb(255, 255, 255));
                }
            }
        }
        QrCode_Image=QrCode_Image.scaled(qr_size, qr_size, Qt::KeepAspectRatio);
        return QPixmap::fromImage(QrCode_Image);
    }

}
