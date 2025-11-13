//
// Created by RGAA on 19/11/2024.
//

#ifndef GAMMARAY_GR_SKIN_CONTEXT_H
#define GAMMARAY_GR_SKIN_CONTEXT_H

#include <QObject>
#include <functional>
#include <memory>
#include <string>

namespace tc
{

    class SkinContext : public QObject {
    public:
        explicit SkinContext(const std::string& plugin_name);
        ~SkinContext() override = default;

        void OnDestroy();

    private:
    };

}

#endif //GAMMARAY_GR_PLUGIN_CONTEXT_H
