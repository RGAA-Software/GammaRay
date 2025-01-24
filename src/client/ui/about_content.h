//
// Created by RGAA on 2024/5/21.
//

#ifndef GAMMARAYPC_ABOUT_CONTENT_H
#define GAMMARAYPC_ABOUT_CONTENT_H

#include "app_content.h"

#include <memory>

namespace tc
{

    class ClientContext;
    class Settings;

    class AboutContent : public AppContent {
    public:
        AboutContent(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);

    };

}

#endif //GAMMARAYPC_ABOUT_CONTENT_H
