#include "local_file_util.h"
#include <qstandardpaths.h>
#include <qdebug.h>
#include <qstorageinfo.h>
#include "tc_common_new/file_util.h"
#include "tc_label.h" // 翻译相关
#include "file_detail_info.h"
#include "file_log_manager.h"
#include "file_const_def.h"


namespace tc {

QString LocalFileUtil::current_user_dirs_;

YKTaskWorker LocalFileUtil::async_task_worker_;

LocalFileUtil::LocalFileUtil() : BaseFileUtil() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        LocalFileUtil::current_user_dirs_ = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        });
}

void LocalFileUtil::GetThisPCFilesImpl() {
    current_file_container_.Clear();
    current_file_container_.home_ = true;
    // 获取系统中的磁盘列表
    QList<QStorageInfo> drives = QStorageInfo::mountedVolumes();

    // 遍历磁盘列表
    for (const QStorageInfo& drive : drives) {
        qDebug() << "Root path:" << drive.rootPath(); // C:/
        FileDetailInfo file_detaile_info;
        file_detaile_info.local_file_ = true;
        file_detaile_info.file_name_ = drive.rootPath();
        file_detaile_info.file_path_ = drive.rootPath();
        file_detaile_info.file_type_ = EFileType::kDisk;
        current_file_container_.AddFileInfo(file_detaile_info);
    }

    // 获取桌面路径
    FileDetailInfo desktop_info;
    desktop_info.file_path_ = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    desktop_info.file_name_ = /*QStringLiteral("桌面");*/  tcTr("id_file_trans_desktop");
    desktop_info.file_type_ = EFileType::kDesktopFolder;
    desktop_info.local_file_ = true;
    current_file_container_.AddFileInfo(desktop_info);
    qDebug() << "Desktop Path: " << desktop_info.file_path_;

    // 获取我的文档路径
    FileDetailInfo my_document_info;
    my_document_info.file_path_ = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    my_document_info.file_name_ = /*QStringLiteral("我的文档")*/ tcTr("id_file_trans_my_document");
    my_document_info.file_type_ = EFileType::kFolder;
    my_document_info.local_file_ = true;
    current_file_container_.AddFileInfo(my_document_info);
    qDebug() << "Documents Path: " << my_document_info.file_path_;

    // 获取我的音乐路径
    FileDetailInfo my_music_info;
    my_music_info.file_path_ = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    my_music_info.file_name_ = /*QStringLiteral("我的音乐")*/ tcTr("id_file_trans_my_music");
    my_music_info.file_type_ = EFileType::kFolder;
    my_music_info.local_file_ = true;
    current_file_container_.AddFileInfo(my_music_info);
    qDebug() << "Music Path: " << my_music_info.file_path_;

    // 获取我的图片路径
    FileDetailInfo my_picture_info;
    my_picture_info.file_path_ = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    my_picture_info.file_name_ = /*QStringLiteral("我的图片")*/ tcTr("id_file_trans_my_picture");
    my_picture_info.file_type_ = EFileType::kFolder;
    my_picture_info.local_file_ = true;
    current_file_container_.AddFileInfo(my_picture_info);
    qDebug() << "Pictures Path: " << my_picture_info.file_path_;

    // 获取我的视频路径
    FileDetailInfo my_video_info;
    my_video_info.file_path_ = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    my_video_info.file_name_ = /*QStringLiteral("我的视频")*/  tcTr("id_file_trans_my_video");
    my_video_info.file_type_ = EFileType::kFolder;
    my_video_info.local_file_ = true;
    current_file_container_.AddFileInfo(my_video_info);
    qDebug() << "Videos Path: " << my_video_info.file_path_;
}

void LocalFileUtil::GetThisPCFiles() {
    async_task_worker_.AsyncTask(
        [=]() {
            GetThisPCFilesImpl();
        },
        [=]() {
            emit SigGetFiles(current_file_container_);
        }
    );
}

void LocalFileUtil::GetFilesImpl(const QString& path) {
    get_files_list_ = false;
    QDir dir{ path };
    if (!dir.exists()) {
        return;
    }
    current_file_container_.Clear();
    current_file_container_.home_ = false;
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo : fileInfoList) {

        FileDetailInfo info;

        info.file_path_ = fileInfo.absoluteFilePath();

        info.file_name_ = fileInfo.fileName();

        info.file_size_ = fileInfo.size();
        info.data_changed_time_ = fileInfo.metadataChangeTime().toMSecsSinceEpoch();
        info.date_changed_ = fileInfo.metadataChangeTime().toString("yyyy-MM-dd hh:mm:ss");

        std::string file_name = info.file_name_.toStdString();
        std::string file_suffix = FileUtil::GetFileSuffix(file_name);
        info.suffix_ = QString::fromStdString(file_suffix);

        if (fileInfo.isDir()) {
            // 如果是目录，则递归查找子目录
            info.file_type_ = EFileType::kFolder;
        }
        else {
            // 如果是文件，则输出文件信息
            info.file_type_ = EFileType::kFile;
        }
        current_file_container_.AddFileInfo(info);
    }
    get_files_list_ = true;
}

void LocalFileUtil::GetFiles(const QString& path) {

    if (tc::kRealRootPath == path) {
        GetThisPCFiles();
        return;
    }

    async_task_worker_.AsyncTask(
        [=]() {
            GetFilesImpl(path);
        },
        [=]() {
            if (get_files_list_) {
                emit SigGetFiles(current_file_container_);
                return;
            }
            //FileLogManager::Instance()->AppendLog(path + QStringLiteral("不是有效目录"));
            FileLogManager::Instance()->AppendLog(path + tcTr("id_file_trans_log_not_a_valid_dir"));
        }
    );
}

void LocalFileUtil::BatchCreateFoldersImpl(std::vector<QString> folders) {
    for (auto path : folders) {
        QDir dir;
        if (!dir.exists(path)) {
            if (!dir.mkpath(path)) {
                //FileLogManager::Instance()->AppendLog(QStringLiteral("在本地创建") + path + QStringLiteral("文件夹失败"));
                FileLogManager::Instance()->AppendLog(
                    tcTr("id_file_trans_log_create_locally") + path + tcTr("id_file_trans_log_folder_failed"));
            }
        }
    }
}

// 等后面在判断返回值啥的
void LocalFileUtil::BatchCreateFolders(std::vector<QString> folders) {
    async_task_worker_.AsyncTask(
        [=]() {
            BatchCreateFoldersImpl(folders);
        },
        [=]() {

        }
    );
}

void LocalFileUtil::RemoveImpl(std::vector<QString> paths) {
    undel_paths_list_.clear();
    for (auto path : paths) {
        QFileInfo info{ path };
        if (info.isDir()) {
            QDir dir{ path };
            if (!dir.exists()) {
                break;
            }
            if (dir.removeRecursively()) {
            }
            else {
                undel_paths_list_ << path;
            }
        }
        else if (info.isFile()) {
            QFile file{ path };
            if (!file.exists()) {
                break;
            }
            if (file.remove()) {
            }
            else {
                undel_paths_list_ << path;
            }
        }
        else {

        }
    }
}

void LocalFileUtil::Remove(std::vector<QString> paths) {
    async_task_worker_.AsyncTask(
        [=]() {
            RemoveImpl(paths);
        },
        [=]() {
            if (undel_paths_list_.empty()) {
                emit SigRemove();
                return;
            }
            //FileLogManager::Instance()->AppendLog(QStringLiteral("以下本地文件夹或文件夹移除失败:"));
            FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_remove_failed") + QString(":"));
            for (auto& path : undel_paths_list_) {
                FileLogManager::Instance()->AppendLog(path);
            }
        }
    );
}

void LocalFileUtil::ReNameImpl(const QString& old_path, const QString& new_name) {
    QFileInfo  file_info{ old_path };
    QString new_file_path = file_info.path() + "/" + new_name;
    rename_res_ = false;
    if (QFile::rename(old_path, new_file_path)) {
        rename_res_ = true;
    }
    else {
        rename_res_ = false;
    }
}

void LocalFileUtil::ReName(const QString& old_path, const QString& new_name) {
    async_task_worker_.AsyncTask(
        [=]() {
            ReNameImpl(old_path, new_name);
        },
        [=]() {
            if (rename_res_) {
                emit SigReName();
                return;
            }
            //FileLogManager::Instance()->AppendLog(QStringLiteral("在本地") + old_path + QStringLiteral(",重命名为:") + new_name + QStringLiteral(" 失败.请确保文件或文件夹没有被占用或者该路径存在"));
            FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_locally") + old_path + QStringLiteral(",") + tcTr("id_file_trans_log_rename_to") +
                QStringLiteral(":") + new_name + tcTr("id_file_trans_log_failed_rename_reason"));
        }
    );
}

void LocalFileUtil::CreateNewFolderImpl(const QString& parent_path) {
    QDir dir{ parent_path };
    create_new_folder_path_ = "";
    create_new_folder_res_ = false;
    if (!dir.exists()) {
        return;
    }
    int temp_count = 1;
    QString prefix = QStringLiteral("新建文件夹");
    do {
        QString suffix;
        if (1 == temp_count) {
            suffix = "";
        }
        else {
            suffix = "(" + QString::number(temp_count) + ")";
        }

        QString new_folder_name = prefix + suffix;
        create_new_folder_path_ = parent_path + "/" + new_folder_name;
        QDir target_folder{ create_new_folder_path_ };
        if (target_folder.exists()) {
            ++temp_count;
            continue;
        }
        create_new_folder_res_ = dir.mkdir(new_folder_name);
        break;
    } while (true);
}

void LocalFileUtil::CreateNewFolder(const QString& parent_path) {
    async_task_worker_.AsyncTask(
        [=]() {
            CreateNewFolderImpl(parent_path);
        },
        [=]() {
            if (create_new_folder_res_) {
                emit SigCreateNewFolder(create_new_folder_path_);
                return;
            }
            //FileLogManager::Instance()->AppendLog(QStringLiteral("在本地 ") + parent_path + QStringLiteral(" 目录下新建文件夹失败"));
            FileLogManager::Instance()->AppendLog(tcTr("id_file_trans_log_locally") + parent_path + tcTr("id_file_trans_log_failed_create_new_folder_in_the_dir"));
        }
    );
}

void LocalFileUtil::Exists(const QString& path) {
    async_task_worker_.AsyncTask(
        [=]() {
            QFile file{ path };
            emit SigExists(file.exists());
        },
        [=]() {

        }
    );
}

}