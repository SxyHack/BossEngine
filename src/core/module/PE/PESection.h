#pragma once

#include <QObject>
#include <QList>
#include "global.h"

class PESection : public QObject
{
	Q_OBJECT

public:
	PESection();
	PESection(const PIMAGE_SECTION_HEADER pHeader, quint64 imageBase);
	PESection(const PESection& src);
	~PESection();

public:
	PIMAGE_SECTION_HEADER Header;
	QString Name;
	quint64 RVA;  // ��������ַ
	quint64 FOA;  // �ļ�ƫ�Ƶ�ַ
	quint64 ImageAddress;  // ���뵽�ڴ��ĵ�ַ, ImageBase + RVA;

	//�����Ա��һ���������У�����������л�������һ����Ա�������ô��������ǾͲ������ˣ�
	//��Ҫ����VirtualSize�ĺ��塣�����Աָ���˸ý���װ���ڴ����ܴ�С�����ֽ�Ϊ��λ��
	//�����ֵ����SizeOfRawData��ֵ����ô����Ĳ��ֽ���0x00��䡣
	//�����Աֻ�Կ�ִ���ļ���Ч�������obj�ļ��˳�Ա��ֵΪ0
	quint64 ImageSize;
	
	// ָ����ý�����Ӳ���ļ��еĵ�ַ�������ֵ�Ǵ��ļ�ͷ��ʼ�����ƫ������Ҳ����˵�����ַ��һ���ļ�ƫ�Ƶ�ַ(FOA)��
	// ����ֵ������FileAlignment�������������������ֽ�����δ��ʼ�������ݣ��򽫴˳�Ա����Ϊ��
	quint64 RawAddress;
	
	// ָ���˸ý�����Ӳ���ϳ�ʼ�����ݵĴ�С�����ֽ�Ϊ��λ��
	// ����ֵ������FileAlignment�������������С��Misc.VirtualSize��
	// ��ô�ò��ֵ����ಿ�ֽ���0x00��䡣����ò��ֽ�����δ��ʼ�������ݣ���ô���ֵ����Ϊ��
	quint64 RawSize; 
};


// typedef 
typedef QList<PESection>  LIST_PESECTION;