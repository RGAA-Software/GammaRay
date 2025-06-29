#pragma once
#include <qstring.h>

#define FILE_ORGANIZATION "gammaray_cloud_gammaray_file"
#define FILE_APPLICATION  "gammaray_cloud_gammaray_file"

extern QString g_remote_file_permission_path;

namespace tc {

const QString kRealRootPath = "/";

enum class FileAffiliationType {
	kLocal,
	kRemote
};

}
