#include "ConfigurationScanWidget.h"
#include "settings/ScanSettings.h"

ConfigurationScanWidget::ConfigurationScanWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

ConfigurationScanWidget::~ConfigurationScanWidget()
{
}

void ConfigurationScanWidget::showEvent(QShowEvent* event)
{
	ui.EnableMemImage->setChecked(SCAN_SETTING.GetMemImageEnable() ? Qt::Checked : Qt::Unchecked);
	ui.EnableMemPrivate->setChecked(SCAN_SETTING.GetMemPrivateEnable() ? Qt::Checked : Qt::Unchecked);
	ui.EnableMemMapped->setChecked(SCAN_SETTING.GetMemMappedEnable() ? Qt::Checked : Qt::Unchecked);

	ui.EnablePageGuard->setChecked(SCAN_SETTING.GetPageGuardEnable() ? Qt::Checked : Qt::Unchecked);
	ui.EnablePageWriteCombine->setChecked(SCAN_SETTING.GetPageWriteCombineEnable() ? Qt::Checked : Qt::Unchecked);

	ui.IncludeByte->setChecked(SCAN_SETTING.GetAllTypeIncludeByte() ? Qt::Checked : Qt::Unchecked);
	ui.Include2Byte->setChecked(SCAN_SETTING.GetAllTypeInclude2Byte() ? Qt::Checked : Qt::Unchecked);
	ui.Include4Byte->setChecked(SCAN_SETTING.GetAllTypeInclude4Byte() ? Qt::Checked : Qt::Unchecked);
	ui.Include8Byte->setChecked(SCAN_SETTING.GetAllTypeInclude8Byte() ? Qt::Checked : Qt::Unchecked);
	ui.IncludeFloat->setChecked(SCAN_SETTING.GetAllTypeIncludeFloat() ? Qt::Checked : Qt::Unchecked);
	ui.IncludeDouble->setChecked(SCAN_SETTING.GetAllTypeIncludeDouble() ? Qt::Checked : Qt::Unchecked);
}

void ConfigurationScanWidget::hideEvent(QHideEvent* event)
{

}
