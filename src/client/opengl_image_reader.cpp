#include "opengl_image_reader.h"

#include "tc_client_sdk_new/gl/raw_image.h"

#include <QDebug>

namespace tc
{
	std::shared_ptr<RawImage> ImageReader::ReadNV12(const std::string& path, int width, int height) {
		int size = width * height * 1.5;
		std::ifstream in_file;
		in_file.open(path, std::ios::binary);

		if (!in_file.good()) {
			qDebug() << "file error : " << path.c_str();
			return nullptr;
		}

		char* buf = (char*)malloc(size);
		in_file.read(buf, size);

		return RawImage::MakeNV12(buf, size, width, height);
	}

	std::shared_ptr<RawImage> ImageReader::ReadRGBA(const std::string& path, int width, int height) {
		int size = width * height * 4;
		std::ifstream in_file;
		in_file.open(path, std::ios::binary);

		if (!in_file.good()) {
			qDebug() << "file error : " << path.c_str();
			return nullptr;
		}

		char* buf = (char*)malloc(size);
		in_file.read(buf, size);
		return RawImage::MakeRGBA(buf, size, width, height);
	}

	std::shared_ptr<RawImage> ImageReader::ReadI420(const std::string& path, int width, int height) {
		int size = width * height * 1.5;
		qDebug() << "size : " << size;
		std::ifstream in_file;
		in_file.open(path, std::ios::binary);
		if (!in_file.good()) {
			qDebug() << "file error : " << path.c_str();
			return nullptr;
		}

		char* buf = (char*)malloc(size);
		in_file.read(buf, size);
		return RawImage::MakeI420(buf, size, width, height);
	}
}