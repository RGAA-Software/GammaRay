//
// Created by hy on 8/07/2024.
//

#include "file_transfer.h"
#include "gr_settings.h"
#include "gr_context.h"
#include "tc_common_new/log.h"

namespace tc
{

    std::shared_ptr<FileTransfer> FileTransfer::Make(const std::shared_ptr<GrContext>& ctx) {
        return std::make_shared<FileTransfer>(ctx);
    }

    FileTransfer::FileTransfer(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
    }

    void FileTransfer::Start() {
        server_ = std::make_shared<asio2::ws_server>();
        server_->bind_accept([&](std::shared_ptr<asio2::ws_session>& session_ptr) {
               if (!asio2::get_last_error()) {
               } else {
                   LOGE("error occurred when calling the accept function : {} -> {}", asio2::get_last_error_val(), asio2::get_last_error_msg().data());
               }
           }).bind_recv([&](auto & session_ptr, std::string_view data) {
                LOGI("File transfer data size: {}", data.size());
            }).bind_connect([](auto & session_ptr) {

            }).bind_disconnect([](auto & session_ptr) {
               asio2::ignore_unused(session_ptr);

            }).bind_upgrade([](auto & session_ptr) {
            LOGI("File transfer upgrade");
           }).bind_start([&]()
         {
            LOGI("File transfer start");
         }).bind_stop([&]()
          {
              LOGI("File transfer stop");
          });

        server_->start("0.0.0.0", settings_->file_transfer_port_, settings_->file_transfer_path_);
    }

    void FileTransfer::Exit() {

    }

}
