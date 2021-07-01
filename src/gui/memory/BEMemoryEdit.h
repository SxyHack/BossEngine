#pragma once

#include <QAbstractScrollArea>

class BEMemoryEdit : public QAbstractScrollArea
{
	Q_OBJECT

public:
	BEMemoryEdit(QWidget* parent);
	~BEMemoryEdit();

protected:
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:

};
