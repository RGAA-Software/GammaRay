//
// Created by RGAA on 6/08/2025.
//

#ifndef GAMMARAYPREMIUM_RENDER_PROCESS_H
#define GAMMARAYPREMIUM_RENDER_PROCESS_H

#include <vector>
#include <string>
#include <memory>

namespace tc
{
    class ProcessInfo;

    using RenderProcessId = uint32_t;

    class RenderProcess {
    public:
        // = ws port
        RenderProcessId id_ = 0;
        std::shared_ptr<ProcessInfo> process_info_ = nullptr;
        std::string app_path_{};
        std::string app_args_{};
        std::vector<std::string> origin_args_;
        std::string work_dir_{};
    };

}

#endif //GAMMARAYPREMIUM_RENDER_PROCESS_H
