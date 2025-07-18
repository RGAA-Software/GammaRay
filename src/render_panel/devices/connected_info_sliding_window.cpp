#include "connected_info_sliding_window.h"
#include <qtimer.h>
#include <qapplication.h>
#include "no_margin_layout.h"
#include "render_panel/gr_context.h"
#include "tc_qt_widget/widget_helper.h"
#include "connected_info_panel.h"
#include "connected_info_tag.h"
#include "tc_common_new/log.h"
#include "tc_render_panel_message.pb.h"
#include "tc_common_new/client_id_extractor.h"

namespace tc {

	ConnectedInfoSlidingWindow::ConnectedInfoSlidingWindow(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : QWidget(parent), ctx_(ctx) {
		InitView();
	}

	void ConnectedInfoSlidingWindow::InitView() {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool  | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground);
		setFixedSize(528, 200);
		main_hbox_layout_ = new NoMarginHLayout();
		setLayout(main_hbox_layout_);

		main_hbox_layout_->addStretch(1);
        {
            auto layout = new NoMarginVLayout();
            tag_ = new ConnectedInfoTag();
            layout->addSpacing(5);
            layout->addWidget(tag_);
            main_hbox_layout_->addLayout(layout);
            layout->addStretch();
        }
        panel_ = new ConnectedInfoPanel(ctx_);
        main_hbox_layout_->addWidget(panel_);
		tag_->installEventFilter(this);
	}

	void ConnectedInfoSlidingWindow::paintEvent(QPaintEvent* event) {
		QWidget::paintEvent(event);
	}

	bool ConnectedInfoSlidingWindow::eventFilter(QObject* obj, QEvent* event) {
		if (obj == tag_) {
			if (event->type() == QEvent::MouseButtonRelease && !tag_->generate_movement_) {
				bool current_state = tag_->GetExpanded();
				if (current_state) {
					panel_->hide();
				}
				else {
					panel_->show();
				}
				tag_->SetExpanded(!current_state);
				return true; // 事件已处理，不再传递
			}
		}
		return QWidget::eventFilter(obj, event);
	}

	void ConnectedInfoSlidingWindow::UpdateInfo(const std::shared_ptr<tcrp::RpConnectedClientInfo>& info) {
		panel_->UpdateInfo(info);
	}

	void ConnectedInfoSlidingWindow::Expand() {
		tag_->SetExpanded(true);
		panel_->show();
	}

	std::string ConnectedInfoSlidingWindow::GetStreamId() const {
		return panel_->GetStreamId();
	}
}

