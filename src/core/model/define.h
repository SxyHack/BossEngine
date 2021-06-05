#pragma once

// ����ģʽ
enum EBaseMode {
	EBaseMode_10,
	EBaseMode_16,
	EBaseMode_2
};

enum EScanValueType {
	EScanValueType_Byte,
	EScanValueType_2_Byte,
	EScanValueType_4_Byte,
	EScanValueType_8_Byte,
	EScanValueType_Float,
	EScanValueType_Double,
	EScanValueType_String,
	EScanValueType_All
};

//enum EBaseCompareMethod
//{
//	EBaseCompareMethod_Equal = 0x1, // ��ȷ����
//	EBaseCompareMethod_MoreThan,    // ��������ֵ��
//	EBaseCompareMethod_LessThan,    // ��������ֵС
//	EBaseCompareMethod_Between,     // ����[a, b]֮��
//	EBaseCompareMethod_Unknown      // δ֪����ֵ
//};
//
//enum ENextCompareMethod
//{
//	ENextCompareMethod_Equal = 0x01, // ��ȷ����
//	ENextCompareMethod_Bigger,       // ���ϴ�ɨ��Ľ����
//	ENextCompareMethod_Smaller,      // ���ϴ�ɨ��Ľ��С
//	ENextCompareMethod_IncreaseN,    // ���ϴ�ɨ��Ľ������N
//	ENextCompareMethod_DecreaseN,    // ���ϴ�ɨ��Ľ������N
//	ENextCompareMethod_Changed,      // �䶯����ֵ
//	ENextCompareMethod_Unchanged,    // û�䶯����ֵ
//	ENextCompareMethod_EqualBase,    // ���ڻ���ɨ��Ľ��(���һ��ɨ��Ա�)
//};


enum ECompareMethod {
	ECompareMethod_Exact,        // ��ȷ����, ����ɨ��ʹ��
	ECompareMethod_MoreThan,     // ��������ֵ��
	ECompareMethod_LessThan,     // ��������ֵС
	ECompareMethod_Between,      // ����[a, b]֮��
	ECompareMethod_Unknown,      // δ֪����ֵ

	// --------------------------��������ٴ�ɨ��-----------------------------
	ECompareMethod_Bigger,       // ���ϴ�ɨ��Ľ����
	ECompareMethod_Smaller,      // ���ϴ�ɨ��Ľ��С
	ECompareMethod_IncreaseN,    // ���ϴ�ɨ��Ľ������N
	ECompareMethod_DecreaseN,    // ���ϴ�ɨ��Ľ������N
	ECompareMethod_Changed,      // �䶯����ֵ
	ECompareMethod_Unchanged,    // û�䶯����ֵ
	ECompareMethod_EqualBase,    // ���ڻ���ɨ��Ľ��(���һ��ɨ��Ա�)
};


// 
// ����EScanValueType�����ݳ���
// 
int GetScanValueTypeSize(EScanValueType type);
