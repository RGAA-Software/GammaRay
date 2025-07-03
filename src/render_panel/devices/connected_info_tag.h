#pragma once
#include <qwidget.h>
#include <qpainter.h>
#include <qevent.h>


namespace tc {

	class ConnectedInfoTag : public QWidget {
		Q_OBJECT
	public:
		ConnectedInfoTag(QWidget* parent = nullptr);
		void paintEvent(QPaintEvent* event) override;
		bool GetExpanded() const;
		void SetExpanded(bool expanded);

		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
	public:
		bool generate_movement_ = false;
	private:
		bool expanded_ = true;
		QPoint m_dragStartPos;  // 记录鼠标按下时的位置
		bool m_dragging = false; // 是否正在拖动
	};

}