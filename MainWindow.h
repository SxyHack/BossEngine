#pragma once

#include <QtWidgets/QMainWindow>
#include <QProgressBar>
#include "ui_MainWindow.h"
#include "WinExtras.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);

protected:
    void SetupToolBar();
    void SetupStatusBar();
    void AddScanPage();

private slots:
    void onActionOpenProcess();
    void OnMemoryScanning(qint64 scanned, qint64 total);
    void OnMemoryScanStarted();
    void OnMemoryScanDone();

private:
    Ui::MainWindowClass ui;
    QProgressBar* _ScanProgress;
    WinExtras     _Windows;
};
