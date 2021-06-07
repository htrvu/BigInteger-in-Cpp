#include"BigInt.h"
#include<iostream>
using namespace std;

/* Xử lý từng dòng trong file */
void RunLine(FILE* fIn, FILE* fOut) {
	char buffer[100];
	char* line = (char*)malloc(1);
	line[0] = 0;
	int len = 0;

	while (1) {
		fgets(buffer, 100, fIn);
		int bLen = strlen(buffer);

		bool endl = false;
		if (buffer[bLen - 1] == '\n') {
			endl = true;
			buffer[bLen - 1] = 0;
			bLen--;
		}

		line = (char*)realloc(line, len + bLen + 1);
		strcat(line, buffer);
		len += bLen;

		if (endl || feof(fIn)) break;
	}

	char** data = NULL;
	int nData = 0, start = 0;
	for (int i = 0; i <= len; i++) {
		if (i == len || line[i] == ' ') {
			if (line[i] == ' ') line[i] = 0;
			char* temp = line + start;
			push_back(data, temp, nData);
			start = i + 1;
		}
	}

	BYTE p1, p2;
	BigInt a, b, res;
	a.data = b.data = res.data = NULL;
	char* pos;
	switch (nData) {
	case 3:
		p1 = (BYTE)atoi(data[0]);
		a = Input(data[2], p1);
		if (data[1][0] == '~') {
			res = Not(a);
			Print(res, p1, fOut);
		}
		else if (strcmp(data[1], "abs") == 0) {
			res = Abs(a);
			Print(res, p1, fOut);
		}
		else if (strcmp(data[1], "digits") == 0) {
			int digits = Digits(a);
			fprintf(fOut, "%d\n", digits);
		}
		else if (strcmp(data[1], "to_string") == 0) {
			char* s = ToString(a);
			fprintf(fOut, "%s\n", s);
			free(s);
		}
		else if (strcmp(data[1], "is_prime") == 0) {
			bool s = IsPrime(a);
			fprintf(fOut, "%d\n", s);
		}
		else {
			if (strlen(data[1]) > 2) {
				p2 = (BYTE)atoi(data[1] + 7);
			}
			else {
				p2 = (BYTE)atoi(data[1]);
			}
			char* s = ToBaseN(a, p2);
			fprintf(fOut, "%s\n", s);
			free(s);
		}
		break;
	case 4:
		char temp[] = "pow max min";
		pos = (char*)strstr(temp, data[1]);
		p1 = (BYTE)atoi(data[0]);
		if (pos) {
			a = Input(data[2], p1);
			b = Input(data[3], p1);
			if (pos - temp == 0) {
				res = Pow(a, b);
			}
			else if (pos - temp == 4) {
				res = Max(a, b);
			}
			else res = Min(a, b);
		}
		else {
			a = Input(data[1], p1);
			if (strlen(data[2]) == 1) {
				b = Input(data[3], p1);
				res = Calculate(a, b, data[2][0]);
			}
			else {
				int n;
				if (p1 == 10) {
					n = atoi(data[3]);
				}
				else {
					char* temp = BinToDec(data[3]);
					n = atoi(temp);
					free(temp);
				}
				res = Shift(a, n, data[2][0]);
			}
		}
		Print(res, p1, fOut);
		break;
	}

	Free(&a); Free(&b); Free(&res);
	free(data);
	data = NULL;
	free(line);
	line = NULL;
}

void RunProgram(char* fileIn, char* fileOut) {
	FILE* fIn = fopen(fileIn, "r");
	FILE* fOut = fopen(fileOut, "w");

	long long line = 1;
	while (1) {
		printf("Test %d:\n", line);
		RunLine(fIn, fOut);
		line++;
		printf("------------------------------------------\n");
		if (feof(fIn)) break;
	}

	fclose(fIn);
	fclose(fOut);
}

//--------------------------------------------------------------------

int main(int argc, char** argv) {
	RunProgram(argv[1], argv[2]);
}