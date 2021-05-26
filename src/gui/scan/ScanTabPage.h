#pragma once

#include <QWidget>
#include <QButtonGroup>
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

protected:
	void SetupScanValueType();
	void SetupScanMethod();

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

	void OnComboBoxScanMethodChanged(int);
	void OnComboBoxScanValueTypeChanged(int);

private:
	Ui::ScanTabPage ui;
	QButtonGroup _BaseGroup;
	qint32       _UniqueID;
	BEWorkspace  _Workspace;
	QString      _InputMask;
};
