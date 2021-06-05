#pragma once

// 进制模式
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
//	EBaseCompareMethod_Equal = 0x1, // 精确查找
//	EBaseCompareMethod_MoreThan,    // 比输入数值大
//	EBaseCompareMethod_LessThan,    // 比输入数值小
//	EBaseCompareMethod_Between,     // 介于[a, b]之间
//	EBaseCompareMethod_Unknown      // 未知的数值
//};
//
//enum ENextCompareMethod
//{
//	ENextCompareMethod_Equal = 0x01, // 精确查找
//	ENextCompareMethod_Bigger,       // 比上次扫描的结果大
//	ENextCompareMethod_Smaller,      // 比上次扫描的结果小
//	ENextCompareMethod_IncreaseN,    // 比上次扫描的结果增加N
//	ENextCompareMethod_DecreaseN,    // 比上次扫描的结果减少N
//	ENextCompareMethod_Changed,      // 变动的数值
//	ENextCompareMethod_Unchanged,    // 没变动的数值
//	ENextCompareMethod_EqualBase,    // 等于基础扫描的结果(与第一次扫描对比)
//};


enum ECompareMethod {
	ECompareMethod_Exact,        // 精确查找, 基础扫描使用
	ECompareMethod_MoreThan,     // 比输入数值大
	ECompareMethod_LessThan,     // 比输入数值小
	ECompareMethod_Between,      // 介于[a, b]之间
	ECompareMethod_Unknown,      // 未知的数值

	// --------------------------下面的是再次扫描-----------------------------
	ECompareMethod_Bigger,       // 比上次扫描的结果大
	ECompareMethod_Smaller,      // 比上次扫描的结果小
	ECompareMethod_IncreaseN,    // 比上次扫描的结果增加N
	ECompareMethod_DecreaseN,    // 比上次扫描的结果减少N
	ECompareMethod_Changed,      // 变动的数值
	ECompareMethod_Unchanged,    // 没变动的数值
	ECompareMethod_EqualBase,    // 等于基础扫描的结果(与第一次扫描对比)
};


// 
// 返回EScanValueType的数据长度
// 
int GetScanValueTypeSize(EScanValueType type);
