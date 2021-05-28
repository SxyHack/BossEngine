#include "BEHeaderView.h"
#include "DarkStyle.h"

#include <QPainter>
#include <QMouseEvent>

BEHeaderView::BEHeaderView(QWidget* parent)
	: QHeaderView(Qt::Horizontal, parent)
	, _EnableSelectAll(true)
	, _IsSelected(false)
{
}

BEHeaderView::~BEHeaderView()
{
}

void BEHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
	// 绘制控件原始的样子
	painter->save();
	QHeaderView::paintSection(painter, rect, logicalIndex);
	painter->restore();

	QPalette palette(Qt::green, Qt::white);

	// 在第一列绘制CheckBox
	if (_EnableSelectAll && logicalIndex == 0)
	{
		QStyleOptionButton option;
		option.rect = QRect(3, 0, 21, 21);
		option.state = _IsSelected ? QStyle::State_On : QStyle::State_Off;
		option.palette = palette;

		style()->drawControl(QStyle::CE_CheckBox, &option, painter);
	}
}

void BEHeaderView::mouseReleaseEvent(QMouseEvent* e)
{
	auto index = logicalIndexAt(e->pos());
	if (index == 0)
	{
		_IsSelected = !_IsSelected;
		updateSection(index);
		emit sgStateChanged(_IsSelected);
	}

	QHeaderView::mousePressEvent(e);
}
