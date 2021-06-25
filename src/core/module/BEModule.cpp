#include "BEModule.h"
#include "Process.h"
#include "WinExtras.h"

#include <QFileInfo>

BEModule::BEModule()
	: QObject(nullptr)
	, EntryPoint(0)
	, ModBase(0)
	, ModSize(0)
	, FileSize(0)
{
}

BEModule::~BEModule()
{
}

BEModule* BEModule::Create(Process* pProc, const MODULEENTRY32& tlh32Entry)
{
	auto pModule = new BEModule;
	pModule->FilePath = QString::fromWCharArray(tlh32Entry.szExePath);
	pModule->FileName = QString::fromWCharArray(tlh32Entry.szModule);
	pModule->ModBase = quint64(tlh32Entry.modBaseAddr);
	pModule->ModSize = tlh32Entry.modBaseSize;

	qDebug("Load: %s", pModule->FilePath.toUtf8().data());

	QFileInfo fiMod(pModule->FilePath);
	pModule->FileExt = fiMod.suffix();

	//auto qsSystemDir = WinExtras::GetSystemDir();
	//pModule->Party = pModule->FilePath.contains(qsSystemDir, Qt::CaseInsensitive)
	//	? MODULE_PARTY::SYSTEM
	//	: MODULE_PARTY::USER;

	//QFile qfModule(pModule->FilePath);
	//if (qfModule.open(QIODevice::ReadOnly))
	//{
	//	pModule->FileSize = qfModule.size();
	//	pModule->MappingVA = (quint64)qfModule.map(0, pModule->FileSize);
	//}
	//else
	//{
	//	pModule->FileSize = 0;
	//	pModule->MappingVA = 0;
	//}

	// Get the PE headers
	//if (!NT_SUCCESS(pModule->InitImageNtHeader()))
	//{
	//	auto name = fiMod.baseName();
	//	auto ext = fiMod.suffix();
	//	qCritical("Module %s%s: invalid PE file!", name.toUtf8().data(), ext.toUtf8().data());
	//	qfModule.close();
	//	return nullptr;
	//}

	//// Enumerate all PE sections
	//pModule->ListPESection();
	//pModule->ListExports();
	//pModule->ListImports();
	//pModule->ListTLSCallbacks();
	//pModule->ReadDebugDir();

	//qfModule.close();

	return pModule;
}
