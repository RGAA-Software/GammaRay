//
// Created by RGAA on 2024/4/8.
//

#ifndef TC_SERVER_STEAM_QR_GENERATOR_H
#define TC_SERVER_STEAM_QR_GENERATOR_H

#include <QPixmap>
#include <QString>

namespace tc
{

    class QrGenerator {
    public:

        static QPixmap GenQRPixmap(const QString& message, int qr_size);

    };

}

#endif //TC_SERVER_STEAM_QR_GENERATOR_H
