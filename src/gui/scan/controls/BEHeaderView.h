#pragma once

#include <QHeaderView>

//
// �Զ���QTableWidget��ͷ, �ڵ�һ������ʾȫѡ�ؼ�
//
class BEHeaderView : public QHeaderView
{
	Q_OBJECT

public:
	BEHeaderView(QWidget* parent = nullptr);
	~BEHeaderView();

protected:
	void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
	void mouseReleaseEvent(QMouseEvent* e) override;

signals:
	void sgStateChanged(bool checked);

protected:
	bool _EnableSelectAll;
	bool _IsSelected;
};
