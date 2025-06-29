#include "file_table_model.h"
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOption>
#include <QToolTip>
#include <qpushbutton.h>
#include <qbrush.h>
#include <qfileinfo.h>
#include <qfileiconprovider.h>
#include <qsvgrenderer.h>
#include <qpixmap.h>
#include "tc_common_new/string_util.h"
// tc_label.h 与翻译相关
#include "tc_label.h"

#define COLUMN_FILE_NAME 0

namespace tc {

FileInfoTableViewBtnDelegate::FileInfoTableViewBtnDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
    QColor color(0xed, 0xf0, 0xf6, 255);
    hover_item_background_color_ = QBrush(color);
}

void FileInfoTableViewBtnDelegate::OnHoverIndexChanged(const QModelIndex& index)
{
    hover_row_ = index.row();
    mouse_in_flag_ = true;
}

void FileInfoTableViewBtnDelegate::OnMouseLeaveChanged() {
    qDebug() << "OnMouseLeaveChanged";
}

void FileInfoTableViewBtnDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem new_option(option);
    if (index.row() == hover_row_ && mouse_in_flag_) {
        painter->fillRect(option.rect, hover_item_background_color_);
    }
    QStyledItemDelegate::paint(painter, new_option, index);
}



FileInfoTableModel::FileInfoTableModel(const FileContainer& file_container)
    : file_container_(file_container)
{

}

QVariant FileInfoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        return m_headers[section];
    }
    default:
        return QVariant();
    }
    return QVariant();
}


int FileInfoTableModel::columnCount(const QModelIndex& parent) const
{
    return m_headers.size();
}

int FileInfoTableModel::rowCount(const QModelIndex& parent) const
{
    return file_container_.files_detail_info_.size();
}

QVariant FileInfoTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    int col = index.column();
    auto record = file_container_.files_detail_info_[row];

    switch (role)
    {
    case Qt::TextAlignmentRole:
    {
#if 0
        switch (col)
        {
        case COLUMN_FILEPATH:
        case COLUMN_FILENAME:
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            return int(Qt::AlignCenter | Qt::AlignVCenter);
        }
#endif
        //统一用左对齐
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    case Qt::DecorationRole: // 文件图标
    {
        if (col == (int)FileInfoTableColumnType::kColumnName) {
            QFileInfo fileInfo(record.file_name_);
            if (EFileType::kFolder == record.file_type_) {
                static auto icon = QIcon(":/resource/folder.svg");
                return icon;
            }
            else if (EFileType::kDesktopFolder == record.file_type_) {
                static auto icon = QIcon(":/resource/desktop.svg");
                return icon;
            }
            else {
                QFileIconProvider iconProvider;
                return iconProvider.icon(fileInfo);
            }
        }
        break;
    }
    case Qt::EditRole: // 这里注意
    case Qt::DisplayRole:
    {

        switch (col)
        {
        case (int)FileInfoTableColumnType::kColumnName:
            //qDebug() << "record.file_name_ = " << record.file_name_;
            return record.file_name_;
        case (int)FileInfoTableColumnType::kColumnSize:
            if (record.file_type_ != EFileType::kFile) {
                return QVariant();
            }
            return QString::fromStdString(StringUtil::FormatSize(record.file_size_)); // 转为合理的单位
        case (int)FileInfoTableColumnType::kColumnType: {
            QString type_name;
            switch (record.file_type_)
            {
            case EFileType::kDesktopFolder:
                //type_name = QStringLiteral("桌面文件夹");
                type_name = tcTr("id_file_trans_desktop_folder");
                break;
            case EFileType::kDisk:
                //type_name = QStringLiteral("磁盘");
                type_name = tcTr("id_file_trans_disk");
                break;
            case EFileType::kFile:
                //type_name = QStringLiteral("文件");
                type_name = tcTr("id_file_trans_file");
                break;
            case EFileType::kFolder:
                //type_name = QStringLiteral("文件夹");
                type_name = tcTr("id_file_trans_folder");
                break;
            default:
                break;
            }
            return type_name;
        }
        case (int)FileInfoTableColumnType::kColumnDateChanaged: {
            if (record.file_type_ != EFileType::kFile) {
                return QVariant();
            }
            return record.date_changed_;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return QVariant();
}

#if 1
// 设置表格项数据
bool FileInfoTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    auto row = index.row();
    auto col = index.column();

    switch (role)
    {
#if 0 
    case Qt::EditRole:
    {
        if (col == COLUMN_FILE_NAME)
        {
            file_list_[row].fileComment = value.toString();
            emit dataChanged(index, index);
            return true;
        }
        break;
    }
#endif
    default:
        return false;
    }
    return false;
}
#endif

void FileInfoTableModel::Updatefilecontainer(const FileContainer& file_container) {
    file_container_ = file_container;
    refresh();

}
}