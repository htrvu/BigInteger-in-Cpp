#include"BigInt.h"

//--------------------------------------------------------------------

/* Đọc dữ liệu */

/* Đọc chuỗi số nhị phân, xem đó là chuỗi số nhị phân dương */
BigInt InputPosBinary(char* s) {
	int len = strlen(s);

	int startT = 0;
	while (s[startT] == '0' && startT < len - 1) startT++;

	BigInt res;
	res.sign = 1;
	res.nBit = len - startT;
	if (res.nBit >= 128) res.rBit = res.nBit + 1;
	res.nBlock = (res.nBit + 15) >> 4;
	res.data = (BLOCK*)calloc(res.nBlock, sizeof(BLOCK));

	for (int i = startT; i < len; i++) {
		if (s[len - 1 - i + startT] == '1') {
			SetBit1(res, i - startT);
		}
	}

	return res;
}

/* Đọc chuỗi số nhị phân */
BigInt InputBinary(char* s) {
	int len = strlen(s);
	BigInt res = InputPosBinary(s);

	if (res.nBit >= 128 && s[0] == '1') {
		for (int i = res.nBit; i < res.nBlock * 16; i++) {
			SetBit1(res, i);
		}
		res.rBit = res.nBit;

		BigInt temp = TwoComplement(res);
		FixNBlock(&temp, res.nBlock);
		temp.sign = -1;

		Free(&res);
		res = temp;
	}

	return res;
}

/* Đưa chuỗi số thập phân dương */
BigInt InputPosDecimal(char* s) {
	int len = strlen(s);
	if (len == 1 && s[0] == '0') {
		BigInt res = Create(0);
		return res;
	}

	reverse(s, s + len);

	BigInt res;
	res.sign = 1;
	res.nBlock = 0;
	res.nBit = 0;
	res.data = NULL;
	uint64_t carry = 0;

	while (len > 1 || s[0] != '0') {
		carry = 0;
		for (int i = len - 1; i >= 0; i--) {
			uint64_t num = int(s[i] - '0') + carry * 10;
			s[i] = char((num >> 16) + '0');
			carry = num & (BASE - 1);
		}

		res.data = (BLOCK*)realloc(res.data, (res.nBlock + 1) * sizeof(BLOCK));
		res.data[res.nBlock] = carry;
		res.nBlock += 1;
		res.nBit += 16;

		while (len > 1 && s[len - 1] == '0') {
			len--;
		}
	}

	for (int i = 15; i >= 0; i--) {
		if ((1 << i) & carry) break;
		res.nBit--;
	}

	if (res.nBit >= 128) res.rBit = res.nBit + 1;

	return res;
}

/* Đọc chuỗi số thập phân */
BigInt InputDecimal(char* s) {
	int sign = 1, i = 0, len = strlen(s);
	while (s[i] == '-' || s[i] == '+') {
		if (s[i] == '-') sign *= -1;
		i++;
	}

	while (s[i] == '0' && i < len - 1) i++;

	BigInt res = InputPosDecimal(s + i);
	if (IsZero(res)) res.sign = 1;
	else res.sign = sign;

	return res;
}

/* Đọc theo hệ cơ số base */
BigInt Input(char* s, BYTE base) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt res;
	if (base == 10) res = InputDecimal(s);
	else res = InputBinary(s);

	endT = clock();
	endM = HeapUsed();

	printf("Read BigInt: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}
//--------------------------------------------------------------------

/* Xuất dữ liệu */

/* Ghi dưới dạng nhị phân */
void PrintBinary(BigInt a, FILE* f) {
	char* binary = ToBase2(a);
	fprintf(f, "%s\n", binary);
	free(binary);
}

/* Ghi dưới dạng thập phân */
void PrintDecimal(BigInt a, FILE* f) {
	char* decimal = ToBase10(a);
	fprintf(f, "%s\n", decimal);
	free(decimal);
}

/* Ghi theo hệ cơ số base */
void Print(BigInt a, BYTE base, FILE* f) {
	if (base == 10) PrintDecimal(a, f);
	else PrintBinary(a, f);
}

//--------------------------------------------------------------------

/* Chuyển hệ cơ số */

/* Chuyển BigInt qua số nhị phân */
char* ToBase2(BigInt a) {
	char* res = NULL;
	int len = 0;
	bool copy = true;
	int last = 0;

	BigInt _a;
	if (a.sign == -1) {
		_a = TwoComplement(a);
		last = _a.rBit - 1;
	}
	else {
		_a = a;
		last = _a.nBit - 1;
		copy = false;
	}

	for (int i = last; i >= 0; i--) {
		push_back(res, GetBit(_a, i) + 48, len);
	}

	if (copy) Free(&_a);

	return res;
}

char* ToBase10(BigInt a) {
	BigInt _a;
	BigIntCpy(&_a, a);

	if (IsZero(_a)) {
		char* res = (char*)malloc(2);
		res[0] = '0';
		res[1] = '\0';
		return res;
	}

	char* res = (char*)malloc(1);
	res[0] = '\0';
	int nLen = 0;

	int len = _a.nBlock;
	while (len > 1 || _a.data[0] != 0) {
		uint64_t carry = 0;
		for (int i = len - 1; i >= 0; i--) {
			uint64_t sum = (uint64_t)_a.data[i] + (carry << 16);
			_a.data[i] = (BLOCK)(sum / 10);
			carry = sum % 10;
		}

		push_back(res, char(carry + '0'), nLen);

		while (len > 1 && _a.data[len - 1] == 0) {
			len--;
		}
	}

	if (_a.sign == -1) {
		push_back(res, '-', nLen);
	}

	Free(&_a);

	reverse(res, res + nLen);
	return res;
}

/* Chuyển số thập phân qua số nhị phân */
char* DecToBin(char* s) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt a = InputDecimal(s);
	char* res = ToBase2(a);

	endT = clock();
	endM = HeapUsed();

	printf("Function DecToBin: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

/* Chuyển số nhị phân qua số thập phân */
char* BinToDec(char* s) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt a = InputBinary(s);
	char* res = ToString(a);
	Free(&a);

	endT = clock();
	endM = HeapUsed();

	printf("Function BinToDec: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

/* Chuyển BigInt sang base 32 (sử dụng bảng chữ cái base32hex) */
char* ToBase32(BigInt a) {
	char* res = NULL;
	int len = 0;

	char base32hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G' ,'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V' };

	bool copy = true;
	BigInt _a = ChangeBitArray(a, copy);

	BLOCK pad = 8 - (_a.rBit % 40 + 4) / 5;

	BLOCK temp;
	for (int i = _a.rBit - 1; i >= 0; i -= 5) {
		temp = 0;
		for (int j = 0; j <= 4 && i - j >= 0; j++) {
			BLOCK bit = i - j < _a.nBit ? GetBit(_a, i - j) : 0;
			temp |= bit << (4 - j);
		}
		push_back(res, base32hex[temp], len);
	}

	for (; pad > 0; pad--) {
		push_back(res, '=', len);
	}

	if (copy) Free(&_a);

	return res;
}

/* Chuyển BigInt sang base 58 */
char* ToBase58(BigInt a) {
	if (a.sign == -1) return NULL;

	char* res = NULL;
	int nLen = 0;

	if (IsZero(a)) {
		res = (char*)malloc(2);
		res[0] = '1';
		res[1] = 0;
		return res;
	}

	char base58[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' , 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

	BigInt _a;
	BigIntCpy(&_a, a);

	int len = _a.nBlock;
	while (len > 1 || _a.data[0] != 0) {
		uint64_t carry = 0;
		for (int i = len - 1; i >= 0; i--) {
			uint64_t sum = (int)_a.data[i] + (carry << 16);
			_a.data[i] = (BLOCK)(sum / 58);
			carry = sum % 58;
		}

		push_back(res, base58[carry], nLen);

		while (len > 1 && _a.data[len - 1] == 0) {
			len--;
		}
	}

	reverse(res, res + nLen);
	return res;
}

/* Chuyển BigInt sang base 64 (sử dụng bảng chữ cái Base64 chuẩn) */
char* ToBase64(BigInt a) {
	char* res = NULL;
	int len = 0;

	char base64[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

	bool copy = true;
	BigInt _a = ChangeBitArray(a, copy);

	BLOCK pad = 4 - (_a.rBit % 24 + 5) / 6;

	BLOCK temp;
	for (int i = _a.rBit - 1; i >= 0; i -= 6) {
		temp = 0;
		for (int j = 0; j <= 5 && i - j >= 0; j++) {
			BLOCK bit = i - j < _a.nBit ? GetBit(_a, i - j) : 0;
			temp |= bit << (5 - j);
		}
		push_back(res, base64[temp], len);
	}

	for (; pad > 0; pad--) {
		push_back(res, '=', len);
	}

	if (copy) Free(&_a);

	return res;
}

/* Chuyển BigInt sang base N (2, 10, 32, 58, 64) */
char* ToBaseN(BigInt a, BYTE n) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	char* res;
	BYTE base;
	switch (n) {
	case 2:
		res = ToBase2(a);
		base = 2;
		break;
	case 10:
		res = ToBase10(a);
		base = 10;
		break;
	case 32:
		res = ToBase32(a);
		base = 32;
		break;
	case 58:
		res = ToBase58(a);
		base = 58;
		break;
	case 64:
		res = ToBase64(a);
		base = 64;
		break;
	default:
		res = NULL;
		break;
	}

	endT = clock();
	endM = HeapUsed();

	printf("Function ToBase%d: took %gs and %d bytes\n", base, double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

//--------------------------------------------------------------------

/* Các thao tác phụ */

/* Toán tử gán */
void BigIntCpy(BigInt* des, BigInt src) {
	des->sign = src.sign;
	des->rBit = src.rBit;
	des->nBit = src.nBit;
	des->nBlock = src.nBlock;
	des->data = (BLOCK*)malloc(des->nBlock * sizeof(BLOCK));
	memcpy(des->data, src.data, des->nBlock * sizeof(BLOCK));
}

/* Đưa dãy bit đang lưu về đúng biểu diễn nhị phân của số BigInt theo dạng bù 2*/
BigInt ChangeBitArray(BigInt a, bool& copy) {
	BigInt _a;
	if (a.sign == -1) {
		_a = TwoComplement(a);
	}
	else {
		_a = a;
		copy = false;
	}
	return _a;
}

/* Gán bit thứ i bằng 1 */
void SetBit1(BigInt a, int i) {
	a.data[i >> 4] |= ((BLOCK)1 << (i & 15));
}

/* Lấy ra bit thứ i */
BYTE GetBit(BigInt a, int i) {
	return a.data[i >> 4] >> (i & 15) & 1;
}

/* Khởi tạo BigInt bằng một số dương, không quá hơn 16 bit */
BigInt Create(BLOCK n) {
	BigInt res;
	res.sign = 1;
	if (n == 0) res.nBit = 1;
	else {
		res.nBit = 0;
		BLOCK m = n;
		while (m > 0) {
			m /= 2;
			res.nBit++;
		}
	}
	res.nBlock = 1;
	res.data = (BLOCK*)malloc(1 * sizeof(BLOCK));
	res.data[0] = n;
	return res;
}

/* Giải phỏng bộ nhớ */
void Free(BigInt* a) {
	if (a->data) {
		free(a->data);
		a->data = NULL;
	}
}

/* Bỏ những block thừa ở đầu (có giá trị bằng 0) */
void DelExcessBlock(BigInt* a) {
	int i = a->nBlock - 1;
	while (i > 0 && a->data[i] == 0) i--;
	if (a->nBlock - 1 != i) {
		a->nBlock = i + 1;
		if (a->nBit > a->nBlock * 16) {
			a->nBit = a->nBlock * 16;
		}
		a->data = (BLOCK*)realloc(a->data, a->nBlock * sizeof(BLOCK));
	}

	while (a->nBit - 1 > 0 && GetBit(*a, a->nBit - 1) == 0) {
		a->nBit--;
	}

	if (a->sign == 1) {
		a->rBit = max(a->nBit + 1, 128);
	}
}

/* Thay đổi số block */
void FixNBlock(BigInt* a, int nBlockNew) {
	if (nBlockNew != a->nBlock) {
		a->data = (BLOCK*)realloc(a->data, nBlockNew * sizeof(BLOCK));
		if (nBlockNew > a->nBlock) {
			memset(a->data + a->nBlock, 0, (nBlockNew - a->nBlock) * sizeof(BLOCK));
		}
		a->nBlock = nBlockNew;
	}
}

/* Đổi dãy bit đang lưu trong data thành biểu diễn bù 2 của nó */
BigInt TwoComplement(BigInt a) {
	BigInt one = Create(1);
	BigInt _a;
	BigIntCpy(&_a, a);
	int sign = a.sign;

	if (sign == -1) {
		FixNBlock(&_a, (_a.rBit + 15) >> 4);
		_a.nBit = _a.rBit;
	}

	ConvertBit(_a);
	_a.sign = 1;
	BigInt res = _a + one;

	if (sign == -1) {
		FixNBlock(&res, _a.nBlock);
		res.nBit = _a.nBit;
	}
	else {
		DelExcessBlock(&res);
	}

	res.rBit = a.rBit;


	Free(&one);
	Free(&_a);

	return res;
}

/* Đảo bit */
void ConvertBit(BigInt& a) {
	for (int i = 0; i < a.nBlock; i++) {
		a.data[i] = ~a.data[i];
	}
}

/* Kiểm tra BigInt có tất cả block bằng 0 */
bool IsZero(BigInt a) {
	for (int i = 0; i < a.nBlock; i++) {
		if (a.data[i] != (BLOCK)0) return false;
	}
	return true;
}

// Random 1 số BigInt < n
BigInt Randomize(BigInt n) {
	BigInt res;
	res.sign = n.sign;
	res.rBit = n.rBit;
	res.nBlock = n.nBlock;
	res.nBit = res.nBlock * 16;
	res.data = (BLOCK*)malloc(res.nBlock * sizeof(BLOCK));

	for (int i = 0; i < res.nBlock; i++) {
		res.data[i] = rand() & (BASE - 1);
	}

	DelExcessBlock(&res);

	res %= n;
	return res;
}

// Số mũ k lớn nhất thỏa mãn 2^k | n
int CountExponent2(BigInt n) {
	int ans = 0;
	for (int i = 0; i < n.nBlock; i++) {
		if (n.data[i] == 0) ans += 16;
		else {
			for (int j = 0; j < 16; j++) {
				if ((1 << j) & n.data[i]) break;
				ans++;
			}
			break;
		}
	}
	return ans;
}

/* Kiểm tra a, b nguyên tố cùng nhau */
bool IsCoprime(BigInt a, BigInt b) {
	BigInt _a, _b;
	BigIntCpy(&_a, a);
	BigIntCpy(&_b, b);

	BigInt mod_temp = _a % _b;
	while (!(IsZero(mod_temp))) {
		Free(&_a);
		BigIntCpy(&_a, _b);

		Free(&_b);
		BigIntCpy(&_b, mod_temp);

		Free(&mod_temp);
		mod_temp = _a % _b;
	}

	Free(&_a);
	Free(&mod_temp);

	bool res = _b.nBlock == 1 && _b.data[0] == 1;
	Free(&_b);

	return res;
}

/* Bước kiểm tra hợp số trong thuật toán Miller Rabin */
bool IsComposite_MR(BigInt a, BigInt m, int cnt, BigInt n, BigInt nSub1, BigInt One) {
	BigInt temp = PowMod(a, m, n);

	if (temp == One || temp == nSub1) return false;
	for (int i = 1; i < cnt; i++) {
		temp *= temp;
		temp %= n;
		if (temp == nSub1) return false;
	}

	Free(&temp);
	return true;
}

/* Thuật toán Miller Rabin */
bool MillerRabin(BigInt n, int nTest) {
	BigInt nSub1;
	BigIntCpy(&nSub1, n);
	BigInt one = Create(1);
	nSub1 -= one;

	int cnt = CountExponent2(nSub1);
	BigInt m = nSub1 >> cnt;

	for (int test = 0; test < nTest; test++) {
		BigInt a;
		a.data = NULL;

		do {
			Free(&a);
			a = Randomize(n);
		} while (IsZero(a));

		if (!IsCoprime(a, n)) return false;

		if (IsComposite_MR(a, m, cnt, n, nSub1, one)) {
			return false;
		}

		Free(&a);
	}

	Free(&one);
	Free(&nSub1);
	Free(&m);
	return true;
}

//--------------------------------------------------------------------

/* Toán tử trên bit */

/* Toán tử SHIFT LEFT */
BigInt operator << (BigInt a, int n) {
	BigInt res;
	BigIntCpy(&res, a);

	int nBlockNew = res.nBlock;
	if ((res.nBit & 15) == 0 || (res.nBit & 15) + n > 16) {
		nBlockNew += ((n + 15) >> 4);
	}
	res.nBit += n;
	res.rBit += n;

	if (nBlockNew != res.nBlock) {
		FixNBlock(&res, nBlockNew);
	}

	int idx = n >> 4;
	for (int i = res.nBlock - 1; i >= idx; i--) {
		res.data[i] = res.data[i - idx] << (n & 15);
		if (i - idx - 1 >= 0) {
			res.data[i] |= (res.data[i - idx - 1] >> (16 - (n & 15)));
		}
	}
	for (int i = idx - 1; i >= 0; i--) {
		res.data[i] = 0;
	}

	DelExcessBlock(&res);
	return res;
}

BigInt& BigInt::operator <<= (int n) {
	BigInt temp = (*this) << n;
	Free(this);
	*this = temp;
	return *this;
}

/* Toán tử SHIFT RIGHT */
BigInt operator >> (BigInt a, int n) {
	BigInt res;
	BigIntCpy(&res, a);

	res.nBit = max(res.nBit - n, 1);
	if (res.sign == 1) {
		res.rBit = max(128, res.rBit - n);
	}

	int idx = n >> 4;
	for (int i = 0; i <= res.nBlock - 1 - idx; i++) {
		res.data[i] = res.data[i + idx] >> (n & 15);
		if (i + idx + 1 < res.nBlock) {
			res.data[i] |= res.data[i + idx + 1] << (16 - (n & 15));
		}
	}
	for (int i = res.nBlock - idx; i < res.nBlock; i++) {
		if (i >= 0) res.data[i] = 0;
	}

	if (a.sign == -1) {
		int cnt = CountExponent2(a);
		if (n > cnt && !IsZero(res)) {
			BigInt one = Create(1);
			res -= one;
			Free(&one);
		}
	}

	if (IsZero(res)) res.sign = 1;

	DelExcessBlock(&res);
	return res;
}

BigInt& BigInt::operator >>= (int n) {
	BigInt temp = (*this) >> n;
	Free(this);
	*this = temp;
	return *this;
}

/* Toán tử AND */
BigInt operator & (BigInt a, BigInt b) {
	bool copyA = true, copyB = true;
	BigInt _a = ChangeBitArray(a, copyA);
	BigInt _b = ChangeBitArray(b, copyB);

	BLOCK bitA = a.sign == 1 ? 0 : (BASE - 1);
	BLOCK bitB = b.sign == 1 ? 0 : (BASE - 1);

	BigInt res;
	res.rBit = max(a.rBit, b.rBit);
	res.nBlock = max(_a.nBlock, _b.nBlock);
	res.nBit = max(_a.nBit, _b.nBit);
	res.data = (BLOCK*)malloc(res.nBlock * sizeof(BLOCK));

	for (int i = 0; i < res.nBlock; i++) {
		if (i < _a.nBlock && i < _b.nBlock) {
			res.data[i] = _a.data[i] & _b.data[i];
		}
		else if (i < _a.nBlock) {
			res.data[i] = _a.data[i] & bitB;
		}
		else res.data[i] = bitA & _b.data[i];
	}

	if (bitA * bitB == 0) res.sign = 1;
	else {
		res.sign = 1;
		BigInt temp = TwoComplement(res);
		temp.sign = -1;
		Free(&res);
		res = temp;
	}

	if (copyA) Free(&_a);
	if (copyB) Free(&_b);

	DelExcessBlock(&res);
	return res;
}

BigInt& BigInt::operator &= (BigInt a) {
	BigInt temp = (*this) & a;
	Free(this);
	*this = temp;
	return *this;
}

/* Toán tử OR */
BigInt	operator | (BigInt a, BigInt b) {
	bool copyA = true, copyB = true;
	BigInt _a = ChangeBitArray(a, copyA);
	BigInt _b = ChangeBitArray(b, copyB);

	BLOCK bitA = a.sign == 1 ? 0 : (BASE - 1);
	BLOCK bitB = b.sign == 1 ? 0 : (BASE - 1);

	BigInt res;
	res.rBit = max(a.rBit, b.rBit);
	res.nBlock = max(_a.nBlock, _b.nBlock);
	res.nBit = max(_a.nBit, _b.nBit);
	res.data = (BLOCK*)malloc(res.nBlock * sizeof(BLOCK));

	for (int i = 0; i < res.nBlock; i++) {
		if (i < _a.nBlock && i < _b.nBlock)
			res.data[i] = _a.data[i] | _b.data[i];
		else if (i < _a.nBlock) {
			res.data[i] = _a.data[i] | bitB;
		}
		else {
			res.data[i] = bitA | _b.data[i];
		}
	}

	if (bitA == 0 && bitB == 0) res.sign = 1;
	else {
		res.sign = 1;
		BigInt temp = TwoComplement(res);
		temp.sign = -1;
		Free(&res);
		res = temp;
	}

	if (copyA) Free(&_a);
	if (copyB) Free(&_b);

	DelExcessBlock(&res);
	return res;
}

BigInt& BigInt::operator |= (BigInt a) {
	BigInt temp = (*this) | a;
	Free(this);
	*this = temp;
	return *this;
}

/* Toán tử XOR */
BigInt operator ^ (BigInt a, BigInt b) {
	bool copyA = true, copyB = true;
	BigInt _a = ChangeBitArray(a, copyA);
	BigInt _b = ChangeBitArray(b, copyA);

	BLOCK bitA = a.sign == 1 ? 0 : (BASE - 1);
	BLOCK bitB = b.sign == 1 ? 0 : (BASE - 1);

	BigInt res;
	res.rBit = max(a.rBit, b.rBit);
	res.nBlock = max(_a.nBlock, _b.nBlock);
	res.nBit = max(_a.nBit, _b.nBit);
	res.data = (BLOCK*)malloc(res.nBlock * sizeof(BLOCK));

	for (int i = 0; i < res.nBlock; i++) {
		if (i < _a.nBlock && i < _b.nBlock)
			res.data[i] = _a.data[i] ^ _b.data[i];
		else if (i < _a.nBlock) {
			res.data[i] = _a.data[i] ^ bitB;
		}
		else {
			res.data[i] = bitA ^ _b.data[i];
		}
	}

	if (bitA == bitB) res.sign = 1;
	else {
		res.sign = 1;
		BigInt temp = TwoComplement(res);
		temp.sign = -1;
		Free(&res);
		res = temp;
	}

	DelExcessBlock(&res);
	return res;
}

BigInt& BigInt::operator ^= (BigInt a) {
	BigInt temp = (*this) ^ a;
	Free(this);
	*this = temp;
	return *this;
}

/* Toán tử NOT */
BigInt operator ~ (BigInt a) {
	BigInt one = Create(1);
	BigInt res;

	if (a.sign == 1) {
		res = AddSameSign(a, one);
		res.sign = -1;
	}
	else {
		a.sign = 1;
		res = SubSameSign(a, one);
		res.sign = 1;
	}

	Free(&one);

	return res;
}

BigInt Not(BigInt a) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt res = ~a;

	endT = clock();
	endM = HeapUsed();

	printf("Operator ~: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}


/* Kết quả phép toán dịch trái, dịch phải */
BigInt Shift(BigInt a, int n, char direction) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt res;
	if (direction == '>') res = a >> n;
	else res = a << n;

	endT = clock();
	endM = HeapUsed();

	printf("Operator %c%c: took %gs and %d bytes\n", direction, direction, double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

//--------------------------------------------------------------------

/* Toán tử cộng, trừ, nhân, modulo */

/* Toán tử cộng */
BigInt operator + (BigInt a, BigInt b) {
	if (a.sign == b.sign) return AddSameSign(a, b);

	if (a.sign == -1) {
		a.sign = 1;
		return SubSameSign(b, a);
	}
	else {
		return SubSameSign(a, b);
	}
}

BigInt& BigInt::operator += (BigInt a) {
	BigInt temp = *this + a;
	Free(this);
	*this = temp;
	return *this;
}

/* Toán tử trừ */
BigInt operator - (BigInt a, BigInt b) {
	if (a.sign == b.sign) {
		return SubSameSign(a, b);
	}

	b.sign *= -1;
	return AddSameSign(a, b);
}

BigInt& BigInt::operator -= (BigInt a) {
	BigInt temp = *this - a;
	Free(this);
	*this = temp;
	return *this;
}

/* Toán tử nhân */
BigInt operator * (BigInt a, BigInt b) {
	int signA = a.sign, signB = b.sign;
	a.sign = b.sign = 1;

	BigInt res = PosMultiply(a, b);
	if (signA == signB) res.sign = 1;
	else res.sign = -1;

	return res;
}

BigInt& BigInt::operator *= (BigInt a) {
	BigInt temp = *this * a;
	Free(this);
	*this = temp;
	return *this;
}

/* Toán tử modulo */
BigInt operator % (BigInt a, BigInt b) {
	int sign = a.sign;
	
	a.sign = b.sign = 1;
	BigInt res = PosModulo(a, b);
	res.sign = sign;

	return res;
}

BigInt& BigInt::operator %= (BigInt a) {
	BigInt temp = *this % a;
	Free(this);
	*this = temp;
	return *this;
}

/* Kết quả phép tính giữa a và b: +, -, *, %, &, |, ^ */
BigInt Calculate(BigInt a, BigInt b, char ope) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt res;
	char c;
	switch (ope) {
	case '+':
		res = a + b;
		c = '+';
		break;
	case '-':
		res = a - b;
		c = '-';
		break;
	case '*':
		res = a * b;
		c = '*';
		break;
	case '%':
		res = a % b;
		c = '%';
		break;
	case '&':
		res = a & b;
		c = '&';
		break;
	case '|':
		res = a | b;
		c = '|';
		break;
	case '^':
		res = a ^ b;
		c = '^';
		break;
	}

	endT = clock();
	endM = HeapUsed();

	printf("Operator %c: took %gs and %d bytes\n", c, double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

//--------------------------------------------------------------

/* Các hàm hỗ trợ tính toán */

/* Cộng hai số cùng dấu */
BigInt AddSameSign(BigInt a, BigInt b) {
	BigInt res;
	res.sign = a.sign;
	res.rBit = max(a.rBit, b.rBit);
	res.nBit = max(a.nBit, b.nBit) + 1;
	res.nBlock = max(a.nBlock, b.nBlock) + 1;
	res.data = (BLOCK*)calloc(res.nBlock, sizeof(BLOCK));

	BLOCK carry = 0, bitA, bitB;
	uint32_t sum = 0;
	for (int i = 0; i < res.nBlock - 1; i++) {
		bitA = i < a.nBlock ? a.data[i] : 0;
		bitB = i < b.nBlock ? b.data[i] : 0;
		sum = (uint32_t)bitA + bitB + carry;
		res.data[i] = sum & (BASE - 1);
		carry = sum >> 16;
	}
	res.data[res.nBlock - 1] = carry;

	DelExcessBlock(&res);
	return res;
}

/* Trừ hai số cùng dấu */
BigInt SubSameSign(BigInt a, BigInt b) {
	if (a < b) {
		BigInt res = SubSameSign(b, a);
		res.sign = -1;
		return res;
	}

	BigInt res;
	res.sign = 1;
	res.rBit = max(a.rBit, b.rBit);
	res.nBit = max(a.nBit, b.nBit);
	res.nBlock = max(a.nBlock, b.nBlock);
	res.data = (BLOCK*)calloc(res.nBlock, sizeof(BLOCK));

	int64_t sum = 0;
	BLOCK bitA, bitB, carry = 0;
	for (int i = 0; i < res.nBlock; i++) {
		bitA = i < a.nBlock ? a.data[i] : 0;
		bitB = i < b.nBlock ? b.data[i] : 0;
		sum = (int64_t)bitA - bitB - carry;
		carry = 0;
		if (sum < 0) {
			res.data[i] = BLOCK(sum + BASE);
			carry = 1;
		}
		else res.data[i] = (BLOCK)sum;
	}

	DelExcessBlock(&res);
	return res;
}

/* Nhân hai số dương */
BigInt PosMultiply(BigInt a, BigInt b) {
	BigInt res;

	if (IsZero(a) || IsZero(b)) {
		res = Create(0);
		return res;
	}

	res.sign = 1;
	res.nBit = a.nBit + b.nBit + 1;
	res.nBlock = a.nBlock + b.nBlock + 1;
	res.data = (BLOCK*)calloc(res.nBlock, sizeof(BLOCK));

	uint32_t bitA, bitB;
	for (int j = 0; j < b.nBlock; j++) {
		uint64_t sum = 0, carry = 0;
		bitB = b.data[j];
		for (int i = 0; i < a.nBlock || carry; i++) {
			bitA = i < a.nBlock ? a.data[i] : 0;
			sum = (uint64_t)res.data[i + j] + bitB * bitA + carry;
			res.data[i + j] = (BLOCK)sum & (BASE - 1);
			carry = sum >> 16;
		}
	}

	DelExcessBlock(&res);
	return res;
}

/* Modulo hai số dương */
BigInt PosModulo(BigInt a, BigInt b) {
	BigInt res;
	if (a < b) {
		BigIntCpy(&res, a);
		return res;
	}

	BigInt _a, _b;
	BigIntCpy(&_a, a);
	BigIntCpy(&_b, b);

	_b <<= (_a.nBit - _b.nBit);

	while (_a >= b) {
		if (_a >= _b) _a -= _b;
		else _b >>= 1;
	}

	BigIntCpy(&res, _a);
	Free(&_a);
	Free(&_b);

	DelExcessBlock(&res);
	return res;
}

//--------------------------------------------------------------------

/* Toán tử so sánh */

bool operator < (BigInt a, BigInt b) {
	if (a.sign != b.sign) {
		return a.sign < b.sign;
	}
	else if (a.nBlock != b.nBlock) {
		return (a.sign == 1 && a.nBlock < b.nBlock) || (a.sign == -1 && a.nBlock > b.nBlock);
	}
	else {
		for (int i = 0; i < a.nBlock; i++) {
			if (a.data[a.nBlock - 1 - i] != b.data[b.nBlock - 1 - i]) {
				if (a.sign == 1) {
					return (a.data[a.nBlock - 1 - i] < b.data[b.nBlock - 1 - i]);
				}
				return a.data[a.nBlock - 1 - i] > b.data[b.nBlock - 1 - i];
			}
		}
	}

	return false;
}

bool operator <= (BigInt a, BigInt b) {
	return !(a > b);
}

bool operator == (BigInt a, BigInt b) {
	if (a.sign != b.sign || a.nBlock != b.nBlock) {
		return false;
	}

	for (int i = 0; i < a.nBlock; i++) {
		if (a.data[a.nBlock - 1 - i] != b.data[b.nBlock - 1 - i]) {
			return false;
		}
	}

	return true;
}

bool operator != (BigInt a, BigInt b) {
	return !(a == b);
}

bool operator > (BigInt a, BigInt b) {
	return !(a < b) && !(a == b);
}

bool operator >= (BigInt a, BigInt b) {
	return !(a < b);
}

//--------------------------------------------------------------------

/* Các hàm áp dụng với BigInt */

/* Max giữa 2 số BigInt */
BigInt Max(BigInt a, BigInt b) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt res;
	if (a >= b) BigIntCpy(&res, a);
	else BigIntCpy(&res, b);

	endT = clock();
	endM = HeapUsed();

	printf("Function Max: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

/* Min giữa 2 số BigInt */
BigInt Min(BigInt a, BigInt b) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt res;
	if (a <= b) BigIntCpy(&res, a);
	else BigIntCpy(&res, b);

	endT = clock();
	endM = HeapUsed();

	printf("Function Min: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

/* Giá trị tuyệt đối */
BigInt Abs(BigInt a) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt res;
	BigIntCpy(&res, a);
	res.sign = 1;

	endT = clock();
	endM = HeapUsed();

	printf("Function Abs: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

/* Số lượng chữ số của BigInt */
int Digits(BigInt a) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	int res = 0;
	if (IsZero(a)) res = 1;
	else {
		BigInt _a;
		BigIntCpy(&_a, a);

		int len = _a.nBlock;
		while (len > 1 || _a.data[0] != 0) {
			int carry = 0;
			for (int i = len - 1; i >= 0; i--) {
				int sum = (int)_a.data[i] + (carry << 16);
				_a.data[i] = (BLOCK)(sum / 10);
				carry = sum % 10;
			}

			res++;

			while (len > 1 && _a.data[len - 1] == 0) {
				len--;
			}
		}

		Free(&_a);
	}

	endT = clock();
	endM = HeapUsed();

	printf("Function Digits: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

/* Tính lũy thừa a^b */
BigInt Pow(BigInt a, BigInt b) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	BigInt _a, _b;
	BigIntCpy(&_a, a);
	BigIntCpy(&_b, b);

	BigInt res = Create(1);

	while (!IsZero(_b)) {
		if (_b.data[0] & 1) res *= _a;
		_b >>= 1;
		_a *= _a;
	}

	Free(&_a);
	Free(&_b);
	DelExcessBlock(&res);

	endT = clock();
	endM = HeapUsed();

	printf("Function Pow: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

/* Tính (a^b) % mod*/
BigInt PowMod(BigInt a, BigInt b, BigInt mod) {
	BigInt _a, _b;
	BigIntCpy(&_a, a);
	BigIntCpy(&_b, b);

	BigInt res = Create(1);

	while (!IsZero(_b)) {
		if (_b.data[0] & 1) {
			res *= _a;
			res %= mod;
		}
		_b >>= 1;
		_a *= _a;
		_a %= mod;
	}

	Free(&_a);
	Free(&_b);
	DelExcessBlock(&res);
	return res;
}

/* Kiểm tra nguyên tố */
bool IsPrime(BigInt a) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	if (a.sign < 0) return false;
	if (a.nBlock == 1 && a.data[0] < 4) {
		return a.data[0] == 2 || a.data[0] == 3;
	}
	if ((a.data[0] & 1) == 0) return false;
	bool res = MillerRabin(a);

	endT = clock();
	endM = HeapUsed();

	printf("Function IsPrime: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

/* Chuyển BigInt sang chuỗi */
char* ToString(BigInt a) {
	clock_t startT, endT;
	long long startM, endM;

	startM = HeapUsed();
	startT = clock();

	char* res = ToBase10(a);

	endT = clock();
	endM = HeapUsed();

	printf("Function ToSting: took %gs and %d bytes\n", double(endT - startT) / SEC, max(endM - startM - 36, 0));

	return res;
}

//--------------------------------------------------------------------

/* Các hàm tiện ích */
long long max(long long a, long long b) {
	return a > b ? a : b;
}

void push_back(char*& s, char c, int& len) {
	s = (char*)realloc(s, len + 2);
	s[len] = c;
	s[len + 1] = 0;
	len++;
}

void push_back(char**& s, char* t, int& len) {
	s = (char**)realloc(s, (len + 1) * sizeof(char*));
	s[len] = t;
	len++;
}

void reverse(char* first, char* last) {
	last--;
	while (first < last) {
		char temp = *first;
		*first = *last;
		*last = temp;
		first++;
		last--;
	}
}

/* Tính bộ nhớ đã cấp phát trong phân vùng Heap */
long long HeapUsed() {
	_HEAPINFO info = { 0, 0, 0 };
	long long used = 0;
	int rc;

	while ((rc = _heapwalk(&info)) == _HEAPOK) {
		if (info._useflag == _USEDENTRY) {
			used += info._size;
		}
	}
	if (rc != _HEAPEND && rc != _HEAPEMPTY) {
		used = (used ? -used : -1);
	}

	return used;
}
//--------------------------------------------------------------------