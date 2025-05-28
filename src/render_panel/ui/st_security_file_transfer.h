//
// Created by RGAA on 28/05/2025.
//

#ifndef GAMMARAY_ST_SECURITY_FILE_TRANSFER_H
#define GAMMARAY_ST_SECURITY_FILE_TRANSFER_H

#include "tab_base.h"
#include <memory>

namespace tc
{

    class GrApplication;

    class StSecurityFileTransfer : public TabBase {
    public:
        StSecurityFileTransfer(const std::shared_ptr<GrApplication>& app, QWidget *parent);

    };

}
#endif //GAMMARAY_ST_SECURITY_VISITOR_H
