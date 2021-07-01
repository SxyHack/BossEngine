#pragma once

#include <QDialog>
#include "ui_BEMemoryEditDialog.h"

class BEMemoryEditDialog : public QDialog
{
	Q_OBJECT

public:
	BEMemoryEditDialog(QWidget *parent = Q_NULLPTR);
	~BEMemoryEditDialog();

private:
	Ui::BEMemoryEditDialog ui;
};
