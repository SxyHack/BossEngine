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

enum EScanMethod {
	EScanMethod_Exact,        // 精确数值
	EScanMethod_Greater,      // 大于上次的扫描值
	EScanMethod_Less,         // 小于上次的扫描值
	EScanMethod_Between,      // 介于[a, b]之间
	EScanMethod_Unknown,      // 未知的数值

	// 下面的是再次扫描
	EScanMethod_MoreValue,    // 增加的数值, 比上次扫描大的数
	EScanMethod_AddValue,     // 增加了N的数值, 比上此扫描大N的数
	EScanMethod_Smaller,      // 减少的数值
	EScanMethod_CutValue,     // 减少了N的数值
	EScanMethod_Changed,      // 变动的数值
	EScanMethod_Equal,        // 没变动的数值
	EScanMethod_FirstDiff,    // 对比首次扫描
};