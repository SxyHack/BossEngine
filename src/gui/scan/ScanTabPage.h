#pragma once

#include <QWidget>
#include <QButtonGroup>
#include <QTimer>

#include "core/model/BEWorkspace.h"

#include "ui_ScanTabPage.h"

class ScanTabPage : public QWidget
{
	Q_OBJECT

public:
	ScanTabPage(QWidget *parent = Q_NULLPTR);
	~ScanTabPage();

	void SetUniqueID(qint32 uid);
	qint32 UniqueID();

	void ShowModules();

protected:
	void SetupScanValueType();
	void SetupScanMethod();
	void SetupModuleTableHead();

	//
	// 转换到16进制
	// 
	void ChangeToBase16(); 
	//
	// 转换到10进制
	//
	void ChangeToBase10();
	void ChangeToBase2();

protected:
	void showEvent(QShowEvent* e) override;
	void hideEvent(QHideEvent* e) override;

private slots:
	void OnButtonScan();
	void OnButtonScanReset();
	void OnButtonScanUndo();
	void OnButtonBaseChanged(QAbstractButton*);
	void OnButtonModuleAllSelect(bool);

	void OnComboBoxScanMethodChanged(int);
	void OnComboBoxScanValueTypeChanged(int);

	void OnMemorySearchStarted();
	void OnMemorySearchDone();

	void OnTimeScanRefresh();

signals:
	void ES_MemoryScanStarted();
	void ES_MemoryScanning(qint64 scanned, qint64 total);
	void ES_MemoryScanDone();

private:
	Ui::ScanTabPage ui;
	QButtonGroup _BaseGroup;
	qint32       _UniqueID;
	BEWorkspace  _Workspace;
	QString      _InputMask;
	QTimer       _ScanRefreshTimer;

};
