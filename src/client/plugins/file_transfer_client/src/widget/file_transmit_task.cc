#include "file_transmit_task.h"
#include <qdir.h>
#include <qfileinfo.h>
#include <qdebug.h>
#include <vector>
#include <queue>

#include "tc_common_new/string_util.h"
#include "file_transmit_single_task.h"
#include "file_transmit_single_task_manager.h"
#include "remote_file_util.h"
#include "local_file_util.h"
#include "file_const_def.h"


namespace tc {

YKTaskWorker FileTransmitTask::async_download_task_woker_;

YKTaskWorker FileTransmitTask::async_upload_task_woker_;

std::shared_ptr<FileTransmitTask> FileTransmitTask::CreateFileTransmitTask(FileContainer&& file_container) {
    return std::make_shared<FileTransmitTask>(std::forward<FileContainer>(file_container));
}

FileTransmitTask::FileTransmitTask(FileContainer&& file_container) {
    remote_file_util_ = std::make_shared<RemoteFileUtil>();
    local_file_util_ = std::make_shared<LocalFileUtil>();
    file_container_ = file_container;
    connect(remote_file_util_.get(), &BaseFileUtil::SigGetFiles2, this, [=](FileContainer file_container) {
        async_download_task_woker_.AsyncTask(
            [=]() {
                std::vector<QString> dirs_vec;
                for (auto file_info : file_container.files_detail_info_) {
                    if (EFileType::kFolder == file_info.file_type_ || EFileType::kDesktopFolder == file_info.file_type_) {
                        dirs_vec.push_back(file_info.file_path_);
                    }
                    else if (EFileType::kFile == file_info.file_type_) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        QMetaObject::invokeMethod(this, [=]() {
                            AddFileTransmitSingleTask(file_info.file_path_, file_info.file_size_, file_info.date_changed_);
                            }, Qt::QueuedConnection);
                    }
                }
                std::vector<QString> local_dirs_path; // 要在本地创建的目录
                for (auto dir : dirs_vec) {
                    auto new_dir = BuildOppositePath(dir);
                    std::cout << "DownloadTaskRun new_dir = " << new_dir.toStdString() << std::endl;
                    local_dirs_path.push_back(new_dir);
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    //创建下载任务时候,已经通过递归获取所有文件和文件夹，所以这里没必要再逐层获取了
                    //remote_file_util_->GetFiles2(dir); // 逐层获取
                }
                local_file_util_->BatchCreateFolders(local_dirs_path);
            },
            []() {}
        );
    });
}

FileTransmitTask::~FileTransmitTask() {

}

void FileTransmitTask::TraverseDirectory(const QString& path, std::vector<QString>& folders, std::vector<QString>& files) {
    QDir directory(path);
    // 遍历当前目录下的所有项
    QStringList items = directory.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoSymLinks, QDir::DirsFirst);
    foreach(QString item, items) {
        QString itemPath = directory.filePath(item);

        QFileInfo fileInfo(itemPath);

        if (fileInfo.isDir()) {
            // 如果是文件夹，则递归遍历，并将路径存储到文件夹向量中
            folders.push_back(fileInfo.absoluteFilePath());
            TraverseDirectory(itemPath, folders, files);
        }
        else {
            // 如果是文件，则将路径存储到文件向量中
            files.push_back(fileInfo.absoluteFilePath());
        }
    }
}

void FileTransmitTask::Run() {
    if (EFileTransmitTaskType::kUpload == task_type_) {
        UploadTaskRun();
    }
    else if (EFileTransmitTaskType::kDownload == task_type_) {
        DownloadTaskRun();
    }
}

QString FileTransmitTask::BuildOppositePath(QString current_path) {
    if (EFileTransmitTaskType::kUpload == task_type_) {
        // to do 目前是windows客户端场景，如果linux需要再测试考虑
        if (tc::kRealRootPath == current_dir_path_) {
            current_path = current_path.replace(LocalFileUtil::current_user_dirs_, "");
        }
        else {
            current_path = current_path.replace(current_dir_path_, "");
        }
    }
    else if (EFileTransmitTaskType::kDownload == task_type_) {
        // to do 目前是windows服务端场景，如果linux需要再测试考虑
        if (tc::kRealRootPath == current_dir_path_) {
            current_path = current_path.replace(RemoteFileUtil::current_user_dirs_, "");
        }
        else {
            current_path = current_path.replace(current_dir_path_, "");
        }
    }
    auto res = target_dir_path_ + "/" + current_path;
    res = res.replace("////", "/");
    res = res.replace("///", "/");
    res = res.replace("//", "/");
    return res;
}

void FileTransmitTask::AddFileTransmitSingleTask(const QString& current_file_path, uint64_t remote_file_size, const QString& remote_file_date_str) {
    auto single_task = std::make_shared<FileTransmitSingleTask>();
    single_task->parent_task_id_ = task_id_;
    single_task->task_type_ = task_type_;
    auto new_file_path = BuildOppositePath(current_file_path);
    single_task->current_file_path_ = current_file_path;
    single_task->target_file_path_ = new_file_path;
    if (EFileTransmitTaskType::kDownload == single_task->task_type_) {
        single_task->file_size_ = remote_file_size;
        single_task->remote_file_size_str_ = QString::fromStdString(tc::StringUtil::FormatSize(remote_file_size));
        single_task->remote_file_date_str_ = remote_file_date_str;
    }
    else {
        QFileInfo info{ current_file_path };
        single_task->file_size_ = info.size();
    }
    FileTransmitSingleTaskManager::Instance()->AddFileTransmitSingleTask(single_task);
}

void FileTransmitTask::DownloadTaskRun() {

    std::cout << "async_download_task_woker_  task count = " << async_download_task_woker_.TasksCount() << std::endl;

    async_download_task_woker_.AsyncTask(
        [=]() {
            std::vector<QString> dirs_vec;
            for (auto file_info : file_container_.files_detail_info_) {
                if (EFileType::kFolder == file_info.file_type_ || EFileType::kDesktopFolder == file_info.file_type_) {
                    dirs_vec.push_back(file_info.file_path_);
                }
                else if (EFileType::kFile == file_info.file_type_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    QMetaObject::invokeMethod(this, [=]() {
                        AddFileTransmitSingleTask(file_info.file_path_, file_info.file_size_, file_info.date_changed_);
                        }, Qt::QueuedConnection);
                }
            }
            std::vector<QString> local_dirs_path; // 要在本地创建的目录
            for (auto dir : dirs_vec) {
                auto new_dir = BuildOppositePath(dir);
                std::cout << "DownloadTaskRun new_dir = " << new_dir.toStdString() << std::endl;
                local_dirs_path.push_back(new_dir);
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                // 递归获取文件,因为传文件过程中容易造成网络堵塞,如果不一次性递归获取文件, 可能导致后续的文件获取不到
                remote_file_util_->RecursiveGetFiles(dir);
            }
            local_file_util_->BatchCreateFolders(local_dirs_path);
        },
        []() {}
    );
}

void FileTransmitTask::UploadTaskRun() {

    async_upload_task_woker_.AsyncTask(
        [=]() {
            for (auto file_info : file_container_.files_detail_info_) {
                // 如果是文件夹
                if (EFileType::kFolder == file_info.file_type_ || EFileType::kDesktopFolder == file_info.file_type_) {
                    std::vector<QString> dirs_vec;
                    std::vector<QString> files_vec;
                    dirs_vec.push_back(file_info.file_path_);
                    TraverseDirectory(file_info.file_path_, dirs_vec, files_vec);
                    std::vector<QString> remote_dirs_path;
                    for (auto dir : dirs_vec) {
                        auto new_dir = BuildOppositePath(dir);
                        std::cout << "UploadTaskRun new_dir = " << new_dir.toStdString() << std::endl;
                        remote_dirs_path.push_back(new_dir);
                    }
                    remote_file_util_->BatchCreateFolders(remote_dirs_path);
                    for (auto file_path : files_vec) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        QMetaObject::invokeMethod(this, [=]() {
                            AddFileTransmitSingleTask(file_path);
                            }, Qt::QueuedConnection);
                    }
                }
                // 如果是文件
                else if (EFileType::kFile == file_info.file_type_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    QMetaObject::invokeMethod(this, [=]() {
                        AddFileTransmitSingleTask(file_info.file_path_);
                        }, Qt::QueuedConnection);
                }
            }
        },
        []() {}
    );
}


}