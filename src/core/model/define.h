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

enum EScanMethod {
	EScanMethod_Exact,        // ��ȷ��ֵ
	EScanMethod_Greater,      // �����ϴε�ɨ��ֵ
	EScanMethod_Less,         // С���ϴε�ɨ��ֵ
	EScanMethod_Between,      // ����[a, b]֮��
	EScanMethod_Unknown,      // δ֪����ֵ

	// ��������ٴ�ɨ��
	EScanMethod_MoreValue,    // ���ӵ���ֵ, ���ϴ�ɨ������
	EScanMethod_AddValue,     // ������N����ֵ, ���ϴ�ɨ���N����
	EScanMethod_Smaller,      // ���ٵ���ֵ
	EScanMethod_CutValue,     // ������N����ֵ
	EScanMethod_Changed,      // �䶯����ֵ
	EScanMethod_Equal,        // û�䶯����ֵ
	EScanMethod_FirstDiff,    // �Ա��״�ɨ��
};