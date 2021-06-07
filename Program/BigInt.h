/* Thư viện xử lý số nguyên lớn (số nguyên 16 bytes trở lên) */

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define BLOCK uint16_t
#define BYTE uint8_t
#define BASE 65536
#define SEC CLOCKS_PER_SEC
#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<memory.h>
#include<stdint.h>
#include<stdlib.h>
#include<time.h>

struct BigInt {
	int8_t sign;			// Lưu dấu
	int nBit;				// Số bit vừa đủ để biểu diễn GTTĐ
	int rBit = 128;			// Số bit thực sự
	int nBlock;				// Số block 2 byte vừa đủ để lưu nBit bit
	BLOCK* data = NULL;		// Lưu giá trị tuyệt đối của số biểu diễn
	/*
	Cách lưu trữ:
	data[0]: 15 14 ... 0	data[1]: 31 30 ... 16
	*/

	BigInt& operator += (BigInt a);
	BigInt& operator -= (BigInt a);
	BigInt& operator *= (BigInt a);
	BigInt& operator %= (BigInt a);
	BigInt& operator <<= (int n);
	BigInt& operator >>= (int n);
	BigInt& operator &= (BigInt a);
	BigInt& operator |= (BigInt a);
	BigInt& operator ^= (BigInt a);
};

//-----------------------------------------------------

/* Đọc dữ liệu */

BigInt InputPosBinary(char* s);

BigInt InputBinary(char* s);

BigInt InputPosDecimal(char* s);

BigInt InputDecimal(char* s);

BigInt Input(char* s, BYTE base);

//-----------------------------------------------------

/* Xuất dữ liệu */

void PrintBinary(BigInt a, FILE* f = stdout);

void PrintDecimal(BigInt a, FILE* f = stdout);

void Print(BigInt a, BYTE base, FILE* f = stdout);

//-----------------------------------------------------

/* Chuyển hệ cơ số */

char* ToBase2(BigInt a);

char* ToBase10(BigInt a);

char* DecToBin(char* s);

char* BinToDec(char* s);

char* ToBase32(BigInt a);

char* ToBase58(BigInt a);

char* ToBase64(BigInt a);

char* ToBaseN(BigInt a, BYTE base);

//-----------------------------------------------------

/* Phép toán trên bit */

BigInt operator << (BigInt a, int n);

BigInt operator >> (BigInt a, int n);

BigInt operator & (BigInt a, BigInt b);

BigInt operator | (BigInt a, BigInt b);

BigInt operator ^ (BigInt a, BigInt b);

BigInt operator ~ (BigInt a);

BigInt Not(BigInt a);

BigInt Shift(BigInt a, int n, char direction);

//-----------------------------------------------------

/* Phép toán cộng, trừ, nhân, modulo */

BigInt operator + (BigInt a, BigInt b);

BigInt operator - (BigInt a, BigInt b);

BigInt operator * (BigInt a, BigInt b);

BigInt operator % (BigInt a, BigInt b);

BigInt Calculate(BigInt a, BigInt b, char ope);

//-----------------------------------------------------

/* Toán tử so sánh */

bool operator < (BigInt a, BigInt b);

bool operator <= (BigInt a, BigInt b);

bool operator == (BigInt a, BigInt b);

bool operator > (BigInt a, BigInt b);

bool operator >= (BigInt a, BigInt b);

//-----------------------------------------------------

/* Các hàm hỗ trợ tính toán */

BigInt AddSameSign(BigInt a, BigInt b);

BigInt SubSameSign(BigInt a, BigInt b);

BigInt PosMultiply(BigInt a, BigInt b);

BigInt PosModulo(BigInt a, BigInt b);

//-----------------------------------------------------

/* Các hàm hỗ trợ BigInt */

BigInt Abs(BigInt a);

BigInt Max(BigInt a, BigInt b);

BigInt Min(BigInt a, BigInt b);

char* ToString(BigInt a);

BigInt Pow(BigInt a, BigInt b);

int Digits(BigInt a);

bool IsPrime(BigInt a);

//-----------------------------------------------------

/* Các thao tác phụ */

void SetBit1(BigInt a, int i);

BYTE GetBit(BigInt a, int i);

BigInt Create(BLOCK n);

void BigIntCpy(BigInt* des, BigInt src);

BigInt ChangeBitArray(BigInt a, bool& copy);

void Free(BigInt* a);

void FixNBlock(BigInt* a, int nBlockNew);

void DelExcessBlock(BigInt* a);

void ConvertBit(BigInt& a);

BigInt TwoComplement(BigInt a);

bool IsZero(BigInt a);

BigInt Randomize(BigInt n);

BigInt PowMod(BigInt a, BigInt b, BigInt mod);

int CountExponent2(BigInt k);

BigInt Randomize(BigInt n);

bool IsCoprime(BigInt a, BigInt b);

bool IsComposite_MR(BigInt a, BigInt b, int s, BigInt n, BigInt nSub1, BigInt One);

bool MillerRabin(BigInt n, int nTest = 5);

//-----------------------------------------------------

/* Các hàm tiện ích */

long long max(long long a, long long b);

void push_back(char*& s, char c, int& len);

void push_back(char**& s, char* t, int& len);

void reverse(char* first, char* last);

long long HeapUsed();