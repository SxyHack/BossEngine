#include "ConfigurationDialog.h"
#include "ConfigurationScanWidget.h"

ConfigurationDialog::ConfigurationDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	SetupMenu();
	SetupContants();

	connect(ui.TreeMenu, &QTreeWidget::itemClicked, this, &ConfigurationDialog::OnItemClickScan);
}

ConfigurationDialog::~ConfigurationDialog()
{
}

void ConfigurationDialog::SetupMenu()
{
	auto itemScan = new QTreeWidgetItem(QStringList("扫描"));
	itemScan->setData(0, Qt::UserRole, 0);
	ui.TreeMenu->addTopLevelItem(itemScan);
}

void ConfigurationDialog::SetupContants()
{
	auto cfgScanWidget = new ConfigurationScanWidget(this);
	ui.TabContents->addTab(cfgScanWidget, tr("扫描配置"));

	ui.TabContents->setCurrentIndex(0);
}

void ConfigurationDialog::OnItemClickScan(QTreeWidgetItem* item, int column)
{

}
