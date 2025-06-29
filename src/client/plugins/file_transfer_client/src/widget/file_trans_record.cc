#include "file_trans_record.h"
#include <iostream>
#include <qapplication.h>
#include <qfileinfo.h>
#include <qfileiconprovider.h>
#include <qtableview.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstandardpaths.h>
#include <qfileiconprovider.h>
#include <qstorageinfo.h>
#include <qtableview.h>
#include <qheaderview.h>
#include <qpushbutton.h>
#include <qtablewidget.h>
#include <qscrollbar.h>
#include <qprogressbar.h>
#include <qsizepolicy.h>
#include <qstylepainter.h>
#include <qsvgrenderer.h>
#include <qsvggenerator.h>
#include <qpainter.h>
#include <qmenu.h>
#include <qwidgetaction.h>
#include <qprocess.h>
#include "tc_common_new/string_util.h"
#include "file_table_view_style.h"
#include "file_operation_btn.h"
#include "file_transmit_single_task_manager.h"
#include "tc_label.h"


#include <qpushbutton.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qsvgrenderer.h>


namespace tc {

static QString s_style_sheet = "QProgressBar {"
    "background-color: #E0E0E0;" // 设置背景色
    "border-radius:4px;"
    "color: #000000;"
    "text-align: center;"
    "}"
    "QProgressBar::chunk {"
    "background-color: #2979ff;" // 设置进度块的颜色
    "border-radius: 0px;"
    "}";

static QString s_btn_style_sheet = "QPushButton{border: none;left: 0px;top: 0px;background-color: rgba(255, 255, 255, 0);\
image: url(:/resource/go_back_normal.svg);}";

static QString s_btn_style = R"(
QPushButton{border:0px; border-radius:4px; background-color:#3C3C3C; image:url(:/resource/go_back_normal.svg); padding: 7px;}
QPushButton::hover{border:0px; border-radius:4px; background-color:#454545; image:url(:/resource/go_back_normal.svg); padding: 7px;} 
)";

#define DL_RECORD_TABLE_SCHEDULE_COLUMN 1
#define DL_RECORD_TABLE_DELETE_COLUMN  7
#define DL_RECORD_TABBLE_TASK_STATE_ROLE 1

FileTransRecordTableViewBtnDelegate::FileTransRecordTableViewBtnDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
    QColor color(0xed, 0xf0, 0xf6, 255);
    hover_item_background_color_ = QBrush(color);

    del_icon_ = QIcon{ ":/resource/delete.svg" };
    del_icon_hover_ = QIcon{ ":/resource/delete.svg" };
}

void FileTransRecordTableViewBtnDelegate::OnHoverIndexChanged(const QModelIndex& index)
{
    hover_row_ = index.row();
    mouse_in_flag_ = true;
}

void FileTransRecordTableViewBtnDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem newOption(option);
    initStyleOption(&newOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus)) {
        newOption.state = newOption.state ^ QStyle::State_HasFocus;
    }
    if (index.row() == hover_row_ && mouse_in_flag_) {
        painter->fillRect(option.rect, hover_item_background_color_);
    }
    int temp_state_int = index.model()->data(index, Qt::UserRole + DL_RECORD_TABBLE_TASK_STATE_ROLE).toInt();
    EFileTransmitTaskState current_task_state = static_cast<EFileTransmitTaskState>(temp_state_int);
    if (index.column() == DL_RECORD_TABLE_SCHEDULE_COLUMN) {
        // 当前任务是传输中才会显示进度
        if (EFileTransmitTaskState::KTransmitting == current_task_state) {
            //QStyledItemDelegate::paint(painter, newOption, index); 这样会显示原生的进度数字
            QProgressBar renderer;
            int progressPercentage = index.model()->data(index, Qt::DisplayRole).toInt();
            auto temp_rect = option.rect.adjusted(8, 6, -8, -8);
            renderer.resize(temp_rect.size());
            renderer.setMinimum(0);
            renderer.setMaximum(100);
            renderer.setValue(progressPercentage);
            renderer.setStyleSheet(s_style_sheet);
            renderer.setFormat("%p%");
            painter->save();
            auto point = option.rect.topLeft();
            point.setY(point.y() + 8);
            painter->translate(point);
            renderer.render(painter);
            painter->restore();
        }
        else {
            QStyledItemDelegate::paint(painter, newOption, index);
        }
    }
    else if (index.column() == DL_RECORD_TABLE_DELETE_COLUMN) {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        painter->fillRect(opt.rect, opt.backgroundBrush);
        auto& op_rect = option.rect;
        auto top_left = op_rect.topLeft();
        top_left.setX(top_left.x() + 8);
        top_left.setY(top_left.y() + 4);
        auto new_rect = QRect(top_left, QSize(16, 16));
        QStyledItemDelegate::paint(painter, newOption, index);
        if (btn_state_ == ButtonState::kMouseOver && index.row() == row_) {
            del_icon_hover_.paint(painter, new_rect, Qt::AlignCenter);
        }
        else {
            del_icon_.paint(painter, new_rect, Qt::AlignCenter);
        }
    }
    else {
        QStyledItemDelegate::paint(painter, newOption, index);
    }
}

// 响应鼠标事件，更新数据
bool FileTransRecordTableViewBtnDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    btn_state_ = ButtonState::kUnKnow;
    row_ = -1;
    bool bRepaint = false;
    QRect decorationRect = option.rect;
    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    mouse_point_ = mouseEvent->pos();
    if (event->type() == QEvent::MouseButtonPress && decorationRect.contains(mouseEvent->pos()))
    {
        if (index.column() == DL_RECORD_TABLE_SCHEDULE_COLUMN)
        {
            bool data = model->data(index).toBool();
            model->setData(index, !data);
            bRepaint = true;
        }
    }
    if (index.column() == DL_RECORD_TABLE_DELETE_COLUMN) {
        // 还原鼠标样式
        //QApplication::restoreOverrideCursor();
        do {
            QStyleOptionButton button;
            auto op_rect = option.rect;
            auto top_left = op_rect.topLeft();
            top_left.setX(top_left.x() + 8);
            top_left.setY(top_left.y() + 2);
            auto new_rect = QRect(top_left, QSize(18, 18));
            button.rect = new_rect;
            // 鼠标位于按钮之上
            if (!button.rect.contains(mouse_point_)) {
                break;
            }
            bRepaint = true;
            switch (event->type())
            {
                // 鼠标滑过
            case QEvent::MouseMove:
            {
                // 设置鼠标样式为手型
                //QApplication::setOverrideCursor(Qt::PointingHandCursor);
                btn_state_ = ButtonState::kMouseOver;
                row_ = index.row();
                break;
            }
            // 鼠标按下
            case QEvent::MouseButtonPress:
            {
                btn_state_ = ButtonState::kMousePress;
                emit deleteData(index);
                break;
            }
            // 鼠标释放
            case QEvent::MouseButtonRelease:
            {
            }
            default:
                break;
            }
        } while (false);
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}


int FileTransRecordContainer::Size() {
    return files_trans_record_info_.size();
}

void FileTransRecordContainer::Calculate() {
    int completed_size = 0;
    int failed_size = 0;
    for (const auto& [task_id, detail_info] : files_trans_record_info_) {
        if (EFileTransmitTaskState::kSuccess == detail_info.state_) {
            ++completed_size;
        }
        else if (EFileTransmitTaskState::kError == detail_info.state_) {
            ++failed_size;
        }
    }
    completed_size_ = completed_size;
    failed_size_ = failed_size;
}

int FileTransRecordContainer::CompletedSize() {
    return completed_size_;
}

int FileTransRecordContainer::FailedSize() {
    return failed_size_;
}

FileTransRecordTableModel::FileTransRecordTableModel() {}

QVariant FileTransRecordTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

int FileTransRecordTableModel::columnCount(const QModelIndex& parent) const
{
    return m_headers.size();
}

int FileTransRecordTableModel::rowCount(const QModelIndex& parent) const
{
    return FileTransRecordContainer::Instance()->Size();
}

QVariant FileTransRecordTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    auto row = index.row();
    int col = index.column();
    if (row >= FileTransRecordContainer::Instance()->Size()) {
        return QVariant();
    }
    auto iter = FileTransRecordContainer::Instance()->files_trans_record_info_.rbegin();
    std::advance(iter, row);
    if (iter == FileTransRecordContainer::Instance()->files_trans_record_info_.rend()) {
        return QVariant();
    }
    auto record = iter->second;
    switch (role)
    {
    case Qt::TextAlignmentRole:
    {
#if 0 //这是对齐方式
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
    case Qt::DecorationRole:
    {
        if (col == (int)FileTransRecordTableColumnType::kColumnName) {
            QFileInfo fileInfo(record.file_name_);
            QFileIconProvider iconProvider;
            return iconProvider.icon(fileInfo);
        }
        break;
    }
    case Qt::DisplayRole:
    {
        switch (col)
        {
        case (int)FileTransRecordTableColumnType::kColumnName:
            return record.file_name_;
        case (int)FileTransRecordTableColumnType::kColumnSchedule:
            if (EFileTransmitTaskState::KTransmitting == record.state_) {
                return record.schedule_;
            }
            else if (EFileTransmitTaskState::kSuccess == record.state_) {
                //return QStringLiteral("成功");
                return tcTr("id_file_trans_success");
            }
            else if (EFileTransmitTaskState::kWait == record.state_) {
                //return QStringLiteral("等待中");
                return tcTr("id_file_trans_waiting");
            }
            else if (EFileTransmitTaskState::kError == record.state_) {
                // 将具体原因返回回去
                if (EFileTransmitTaskErrorCause::kDelete == record.cause_) {
                    //return QStringLiteral("删除");
                    return tcTr("id_file_trans_del");
                }
                else if (EFileTransmitTaskErrorCause::kSkip == record.cause_) {
                    //return QStringLiteral("跳过");
                    return tcTr("id_file_trans_skip");
                }
                else if (EFileTransmitTaskErrorCause::kCancel == record.cause_) {
                    //return QStringLiteral("取消");
                    return tcTr("id_file_trans_cancel");
                }
                else if (EFileTransmitTaskErrorCause::kTimeOut == record.cause_) {
                    //return QStringLiteral("超时");
                    return tcTr("id_file_trans_time_out");
                }
                else if (EFileTransmitTaskErrorCause::kTCcaException == record.cause_) {
                    //return QStringLiteral("流路异常");
                    return tcTr("id_file_trans_failed_cause_stream_exception");
                }
                else if (EFileTransmitTaskErrorCause::kPacketLoss == record.cause_) {
                    //return QStringLiteral("网络错误");
                    return tcTr("id_file_trans_net_error");//丢包
                }
#if 0
                else if (EFileTransmitTaskErrorCause::kRemoteFileFailedOpen == record.cause_) {
                    return QStringLiteral("读取远端文件异常");
                }
                else if (EFileTransmitTaskErrorCause::kVerifyError == record.cause_) {
                    return QStringLiteral("校验失败");
                }
                else if (EFileTransmitTaskErrorCause::kFileFailedWrite == record.cause_) {
                    return QStringLiteral("写文件异常");
                }
                else if (EFileTransmitTaskErrorCause::kDirFailedCreate == record.cause_) {
                    return QStringLiteral("文件夹创建失败");
                }
                else if (EFileTransmitTaskErrorCause::kFileNotExists == record.cause_) {
                    return QStringLiteral("文件不存在");
                }
                else if (EFileTransmitTaskErrorCause::kFileFailedOpen == record.cause_) {
                    return QStringLiteral("打开文件异常");
                }
                else if (EFileTransmitTaskErrorCause::kFileFailedRead == record.cause_) {
                    return QStringLiteral("读取文件异常");
                }
#endif
                else {
                    //return QStringLiteral("失败");
                    return tcTr("id_file_trans_log_failed");
                }
            }
            else if (EFileTransmitTaskState::kVerifying == record.state_) {
                //return QStringLiteral("校验中");
                return tcTr("id_file_trans_in_verification");
            }
            return record.schedule_;
        case (int)FileTransRecordTableColumnType::kColumnSize: {
            return QString::fromStdString(tc::StringUtil::FormatSize(record.file_size_));
        }
        case (int)FileTransRecordTableColumnType::kColumnSpeed: {
            return record.speed_;
        }
        case (int)FileTransRecordTableColumnType::kColumnOriginPath: {
            return record.origin_path_;
        }
        case (int)FileTransRecordTableColumnType::kColumnTargetPath: {
            return record.target_path_;
        }
        case (int)FileTransRecordTableColumnType::kColumnDirection: {

            if (EFileTransmitTaskType::kDownload == record.type_) {
                //return QStringLiteral("下载");
                return tcTr("id_file_trans_down");
            }
            else if (EFileTransmitTaskType::kUpload == record.type_) {
                //return QStringLiteral("上传");
                return tcTr("id_file_trans_upload");
            }
            else {
                //return QStringLiteral("未知");
                return tcTr("id_file_trans_failed_cause_unknow");
            }
        }
        default:
            break;
        }
        break;
    }
    case Qt::UserRole + DL_RECORD_TABBLE_TASK_STATE_ROLE: {
        if (col == (int)FileTransRecordTableColumnType::kColumnSchedule) {
            return (int)record.state_;
        }
        return QVariant();
        break;
    }
    default:
        break;
    }
    return QVariant();
}


void FileTransRecordTableModel::Updatefilecontainer() {
    refresh();
}

FileTransRecordTableView::FileTransRecordTableView(QWidget* parent) : QTableView(parent) {
}

void FileTransRecordTableView::mouseMoveEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    emit SigHoverIndexChanged(index);
    QTableView::mouseMoveEvent(event);
}

void FileTransRecordTableView::leaveEvent(QEvent* event) {
    emit SigLeaveEvent();
    QTableView::leaveEvent(event);
}

void FileTransRecordTableView::contextMenuEvent(QContextMenuEvent* event) {
    auto pos = event->pos();
    auto context_index = indexAt(pos);
    // 这里的坐标 是 减去 表头高度的
    //std::cout << "event->pos() x = " << event->pos().x() << " y = " << event->pos().y() << std::endl;
    emit SigContextClicked(context_index, event->globalPos());
}

FileTransRecord::FileTransRecord(QWidget* parent) : QWidget(parent) {
    Init();
}

FileTransRecord::~FileTransRecord() {

}

void FileTransRecord::Init() {
    setAttribute(Qt::WA_StyledBackground);
    main_vbox_layout_ = new QVBoxLayout();
    main_vbox_layout_->setContentsMargins(6, 6, 6, 6);
    setLayout(main_vbox_layout_);
    setStyleSheet(QString("QWidget {background-color: #F5F5F5; border-radius: %1px;}").arg(6));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    /*FileTransRecordTableModel::Instance()->m_headers
        << QStringLiteral("名称")
        << QStringLiteral("进度")
        << QStringLiteral("大小")
        << QStringLiteral("速度")
        << QStringLiteral("发送路径")
        << QStringLiteral("接受路径")
        << QStringLiteral("方向")
        << QStringLiteral("删除");*/


    FileTransRecordTableModel::Instance()->m_headers
        << tcTr("id_file_trans_name")
        << tcTr("id_file_trans_progress")
        << tcTr("id_file_trans_size")
        << tcTr("id_file_trans_speed")
        << tcTr("id_file_trans_send_path")
        << tcTr("id_file_trans_recv_path")
        << tcTr("id_file_trans_direction")
        << tcTr("id_file_trans_del");

    file_trans_record_table_view_ = new FileTransRecordTableView(this);
    main_vbox_layout_->addWidget(file_trans_record_table_view_);
    file_trans_record_table_view_->setAttribute(Qt::WA_StyledBackground);
    file_trans_record_table_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    file_trans_record_table_view_->setObjectName("file_trans_record_table_view_");
    file_trans_record_table_view_->verticalHeader()->hide();
    file_trans_record_table_view_->verticalHeader()->setDefaultSectionSize(25); //设置高度
    file_trans_record_table_view_->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
    file_trans_record_table_view_->horizontalHeader()->setFixedHeight(25);
    file_trans_record_table_view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    file_trans_record_table_view_->verticalScrollBar()->setStyleSheet(QString::fromStdString(kScrollBarStyle));
    file_trans_record_table_view_->setShowGrid(false);
    file_trans_record_table_view_->setAlternatingRowColors(true); // 奇数 偶数 颜色差异
    file_trans_record_table_view_->horizontalHeader()->setHighlightSections(false);
    //file_trans_record_table_view_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft); 无效  要在tablemodel里面做
    file_trans_record_table_view_->setSelectionBehavior(QAbstractItemView::SelectRows); //设置选择行为时每次选择一行
    //file_trans_record_table_view_->setStyleSheet("selection-background-color:lightgrey;"); //设置选中背景色
    file_trans_record_table_view_->setStyleSheet(QString::fromStdString(KFileTableStyle));
    QFont font = file_trans_record_table_view_->horizontalHeader()->font();
    //font.setBold(true); //设置表头字体加粗
    file_trans_record_table_view_->horizontalHeader()->setFont(font);
    file_trans_record_table_view_->setModel(FileTransRecordTableModel::Instance());
    file_trans_record_table_view_->horizontalHeader()->resizeSection(0, 260); //设置表头第一列的宽度
    file_trans_record_table_view_->horizontalHeader()->resizeSection(1, 120);
    file_trans_record_table_view_->horizontalHeader()->resizeSection(2, 70);
    file_trans_record_table_view_->horizontalHeader()->resizeSection(3, 70);
    file_trans_record_table_view_->horizontalHeader()->resizeSection(4, 280);
    file_trans_record_table_view_->horizontalHeader()->resizeSection(5, 280);
    file_trans_record_table_view_->horizontalHeader()->resizeSection(6, 80);
    file_trans_record_table_view_->horizontalHeader()->resizeSection(7, 30);
    file_trans_record_table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    file_trans_record_table_view_->horizontalHeader()->setSectionsMovable(QHeaderView::ResizeToContents);
    file_trans_record_table_view_->setDragDropMode(QAbstractItemView::NoDragDrop);
    auto item_delegate = new FileTransRecordTableViewBtnDelegate(this);
    file_trans_record_table_view_->setItemDelegate(item_delegate);
    connect(item_delegate, &FileTransRecordTableViewBtnDelegate::deleteData, this, [=](QModelIndex index) {
        std::cout << "del index row = " << index.row() << std::endl;
        auto row = index.row();
        if (row >= FileTransRecordContainer::Instance()->Size()) {
            return;
        }
        auto iter = FileTransRecordContainer::Instance()->files_trans_record_info_.rbegin();
        std::advance(iter, row);
        if (iter == FileTransRecordContainer::Instance()->files_trans_record_info_.rend()) {
            return;
        }
        auto task_id = iter->first;
        auto record = iter->second;
        FileTransmitSingleTaskManager::Instance()->DeleteFileTransmitSingleTask(task_id);
        FileTransRecordContainer::Instance()->files_trans_record_info_.erase(task_id);
        file_trans_record_table_view_->update();
    });
    connect(file_trans_record_table_view_, &FileTransRecordTableView::SigHoverIndexChanged, item_delegate, &FileTransRecordTableViewBtnDelegate::OnHoverIndexChanged);
    connect(file_trans_record_table_view_, &FileTransRecordTableView::SigLeaveEvent, this, [=]() {
        item_delegate->mouse_in_flag_ = false;
        file_trans_record_table_view_->update(); // 需要立即刷新，不然hover效果会一直在
        });
    file_trans_record_table_view_->setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    file_trans_record_table_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    adjustSize();
    InitContextMenu();
}

void FileTransRecord::InitContextMenu() {

    QString btn_css = "QPushButton{text-align: left; padding-left: %1px;  border:none; background-color: #ffffff; font-family:Microsoft YaHei; font-size:%2px; color:#000000;}\
					QPushButton:hover{border:none; background-color: #F5F5F5; font-family:Microsoft YaHei; color:#000000;}";
    QString menu_css = R"(QMenu { background-color: #f9f9fd; border-radius: %1px;
		border: 1px solid #c0c0c0;	
		padding: %2px;
		box-shadow: 4px 4px 8px rgba(0, 0, 0, 0.4); 
	})";
    btn_css = btn_css.arg(2).arg(12);
    menu_css = menu_css.arg(4).arg(6);
    QIcon* open_dir_icon = new QIcon(":/resource/new_folder.svg");
 
    context_menu_ = new QMenu();
    context_menu_->setStyleSheet(menu_css);
    context_menu_->setFixedSize(114, 38);

    open_file_explorer_btn_= new QPushButton(this);
    open_file_explorer_btn_->setStyleSheet(btn_css);
    open_file_explorer_btn_->setFixedSize(106, 30);
    open_file_explorer_btn_->setIcon(*open_dir_icon);
    //open_file_explorer_btn_->setText(QStringLiteral("打开所在目录"));
    open_file_explorer_btn_->setText(tcTr("id_file_trans_open_dir_are_located"));
    QWidgetAction* file_explorer_action = new QWidgetAction(context_menu_);
    file_explorer_action->setDefaultWidget(open_file_explorer_btn_);
    context_menu_->addAction(file_explorer_action);

    connect(file_trans_record_table_view_, &FileTransRecordTableView::SigContextClicked, this, [=](QModelIndex index, QPoint pos) {
        OnContextMenuEvent(index, pos);
    });

    connect(open_file_explorer_btn_, &QPushButton::clicked, this, [=]() {
        context_menu_->hide();
        if (to_be_opened_file_path_.isEmpty()) {
            return;
        }
        // Windows下使用explorer的/select参数
        QString nativePath = QDir::toNativeSeparators(to_be_opened_file_path_);
        QProcess::startDetached("explorer", QStringList() << "/select," << nativePath);
    });
}

void FileTransRecord::OnContextMenuEvent(QModelIndex index, QPoint pos) {
    if (!index.isValid()) {
        return;
    }
    auto row = index.row();
    int col = index.column();
    if (row >= FileTransRecordContainer::Instance()->Size()) {
        return;
    }
    auto iter = FileTransRecordContainer::Instance()->files_trans_record_info_.rbegin();
    std::advance(iter, row);
    if (iter == FileTransRecordContainer::Instance()->files_trans_record_info_.rend()) {
        return;
    }
    auto record = iter->second;

    if (EFileTransmitTaskType::kDownload == record.type_) {
        if (static_cast<int>(FileTransRecordTableColumnType::kColumnTargetPath) == col) {
            to_be_opened_file_path_ = record.target_path_;
            context_menu_->exec(pos);
        }
    }
    else if (EFileTransmitTaskType::kUpload == record.type_) {
        if (static_cast<int>(FileTransRecordTableColumnType::kColumnOriginPath) == col) {
            to_be_opened_file_path_ = record.origin_path_;
            context_menu_->exec(pos);
        }
    }    
}

void FileTransRecord::ClearCompletedTasksRecord() {
    std::erase_if(FileTransRecordContainer::Instance()->files_trans_record_info_, [](const auto& item) {
        return EFileTransmitTaskState::kSuccess == item.second.state_;
    });
    file_trans_record_table_view_->update();
    update();
}

std::tuple<int, int, int> FileTransRecord::GetStatisticsInfo() {
    FileTransRecordContainer::Instance()->Calculate();
    return { FileTransRecordContainer::Instance()->Size(), FileTransRecordContainer::Instance()->CompletedSize(), FileTransRecordContainer::Instance()->FailedSize()};
}

}