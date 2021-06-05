#pragma once

#include <QDialog>
#include "ui_ConfigurationDialog.h"

class ConfigurationDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigurationDialog(QWidget *parent = Q_NULLPTR);
	~ConfigurationDialog();

protected:
	void SetupMenu();
	void SetupContants();

protected slots:
	void OnItemClickScan(QTreeWidgetItem* item, int column);

private:
	Ui::ConfigurationDialog ui;
};
