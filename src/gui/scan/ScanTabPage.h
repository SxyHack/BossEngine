#pragma once

#include <QWidget>
#include <QButtonGroup>
#include <QTimer>
#include <QMenu>

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
	void ShowUiStateOpenProcess();
	void ShowUiStateNext(); // ��ʾ�״�ɨ���Ľ���״̬
	void ShowUiStateInitial();

protected:
	void SetupScanValueType();
	void SetupScanMethod();
	void SetupModuleTableHead();
	void SetupFoundTableHead();
	void SetupFoundTableMenu();

	void AppendFoundPointer(BEPointer* pValue);

	//
	// ת����16����
	// 
	void ChangeToBase16(); 
	//
	// ת����10����
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

	void OnMenuFoundTable(const QPoint& pos);
	void OnActionFoundTableCopyAddress(bool checked);


	void OnComboBoxScanMethodChanged(int);
	void OnComboBoxScanValueTypeChanged(int);

	void OnMemorySearchStarted();
	void OnMemorySearchDone(qint64);
	void OnFoundPointer(BEPointer*, qint64);

	void OnTimeProcScanning();
	void OnTimeProcUpdateResults();

signals:
	void ES_MemoryScanStarted();
	void ES_MemoryScanning(qint64 scanned, qint64 total);
	void ES_MemoryScanDone(qint64 count);

	void ES_MemoryFoundValue(qint64 count);

private:
	Ui::ScanTabPage ui;
	bool         _IsBase;           // �Ƿ��һ��ɨ��
	QButtonGroup _BaseGroup;
	qint32       _UniqueID;
	BEWorkspace  _Workspace;
	QString      _InputMask;
	QTimer       _ScanningTimer;    // ɨ����ȸ���
	QTimer       _UpdateTimer;      // ɨ����ʵʱ���
	QMenu        _MenuFoundTable;   
};
