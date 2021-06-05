#pragma once

#include <QWidget>
#include "ui_ConfigurationScanWidget.h"

class ConfigurationScanWidget : public QWidget
{
	Q_OBJECT

public:
	ConfigurationScanWidget(QWidget *parent = Q_NULLPTR);
	~ConfigurationScanWidget();

protected:
	void showEvent(QShowEvent* event) override;
	void hideEvent(QHideEvent* event) override;

private:
	Ui::ConfigurationScanWidget ui;
};
