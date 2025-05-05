// This file is a part of RCKangaroo software
// (c) 2024, RetiredCoder (RC)
// https://github.com/RetiredC

#pragma once

#include <string>
#include "defs.h"
#include "utils.h"

class EcInt
{
public:
	EcInt();

	void Assign(EcInt& val);
	void Set(u64 val);
	void SetZero();
	bool SetHexStr(const char* str);
	void GetHexStr(char* str);
	u16 GetU16(int index);

	bool Add(EcInt& val); //returns true if carry
	bool Sub(EcInt& val); //returns true if carry
	void Neg();
	void Neg256();
	void ShiftRight(int nbits);
	void ShiftLeft(int nbits);
	bool IsLessThanU(EcInt& val);
	bool IsLessThanI(EcInt& val);
	bool IsEqual(EcInt& val);
	bool IsZero();
	bool IsEven();

	void Mul_u64(EcInt& val, u64 multiplier);
	void Mul_i64(EcInt& val, i64 multiplier);

	void AddModP(EcInt& val);
	void SubModP(EcInt& val);
	void NegModP();
	void NegModN();
	void MulModP(EcInt& val);
	void InvModP();
	void SqrtModP();

	void RndBits(int nbits);
	void RndMax(EcInt& max);

	u64 data[4 + 1];
};

class EcPoint
{
public:
	bool IsEqual(EcPoint& pnt);
	void LoadFromBuffer64(u8* buffer);
	void SaveToBuffer64(u8* buffer);
	bool SetHexStr(const char* str);
	EcInt x;
	EcInt y;
};

class Ec
{
public:
	static EcPoint AddPoints(EcPoint& pnt1, EcPoint& pnt2);
	static EcPoint SubtractPoints(EcPoint& pnt1, EcPoint& pnt2);
	static EcPoint DoublePoint(EcPoint& pnt);
	static EcPoint MultiplyG(EcInt& k);
	static EcPoint MultiplyP(EcPoint& pnt, EcInt& k);
	static EcPoint DivPointBy2(EcPoint& pnt);
	static std::string GetPublicKeyHex(EcPoint& pnt);
	static EcPoint ParsePublicKeyHex(std::string pubkey);
	//static EcPoint MultiplyG_Fast(EcInt& k);
	static EcInt CalcY(EcInt& x, bool is_even);
	static bool IsValidPoint(EcPoint& pnt);
};

void InitEc();
//void DeInitEc();
void SetRndSeed(u64 seed);
