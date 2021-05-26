#pragma once

#include <QDialog>
#include <QMap>
#include <QList>
#include "WinExtras.h"
#include "BEngine.h"

#include "ui_ProcessAttachDialog.h"

class ProcessAttachDialog : public QDialog
{
	Q_OBJECT

public:
	ProcessAttachDialog(QWidget* parent = Q_NULLPTR);
	~ProcessAttachDialog();

	DWORD GetSelectedProcessID();

protected:
	void SetupProcessCategory();
	void SetupProcessTableHead();
	void ShowProcessCategory();
	void ShowProcessTable(const QString& filter = "");

protected:
	void showEvent(QShowEvent* e) override;
	void hideEvent(QHideEvent* e) override;

private slots:
	void OnEnumProcessPrepare();
	void OnEnumProcess(qint32, PROCESSENTRY32);
	void OnEnumProcessDone(qint32);

	void OnNameFilterChanged(const QString&);
	void OnClickOpenProcess();
	void OnClickCancel();

private:
	Ui::ProcessAttachDialog ui;
	WinExtras _Windows;
	DWORD     _SelectProcessID;
};
