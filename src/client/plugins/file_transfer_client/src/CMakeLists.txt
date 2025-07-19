project(file_transfer)

set(CMAKE_PREFIX_PATH ${QT_ROOT}/lib/cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/widget)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)
include_directories(${GR_PROJECT_PATH}/deps/tc_qt_widget)

set(FILE_TRANSFER_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/widget/base_file_util.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_const_def.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_detail_info.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_list_widget.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_log_manager.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_operation_btn.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_overview_widget.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_popup_widgets.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_send_btn.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_show_widget.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_table_model.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_table_view.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_transmit_log_widget.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_transmit_single_task.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_transmit_single_task_manager.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_transmit_task.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_transmit_task_manager.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_trans_record.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_trans_widget.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/local_file_util.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/remote_file_util.cc
	${CMAKE_CURRENT_SOURCE_DIR}/widget/file_trans_log_switch_button.cc
	${CMAKE_CURRENT_SOURCE_DIR}/core/file_sdk_interface.cc
	${CMAKE_CURRENT_SOURCE_DIR}/core/file_trans_interface.cc
	${CMAKE_CURRENT_SOURCE_DIR}/core/file_msg_answer_cbk.cc
	${CMAKE_CURRENT_SOURCE_DIR}/core/file_transmit_sdk.cc
	PARENT_SCOPE
)