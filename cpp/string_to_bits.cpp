#include <cstdint>
#include <iostream>
#include <vector>
#include <bitset>
using namespace std;

typedef uint32_t unit;
typedef uint8_t byte;
#define UNIT_LEN (sizeof(unit)*8)

int string_to_bits(string &src, vector<unit> &out);
void print_bits(vector<uint32_t> v, int len);
bool is_zero(byte *s, int len);
unit divide2(byte *s, int len);

int string_to_bits(string &src, vector<unit> &out) {
	int len = src.size();
	byte *tmp = new byte[len];
	for(int i=0; i < len; i++) 
		*(tmp+i) = src[i] - '0';
	int curr_len = 0;
	int unit_len = UNIT_LEN;

	out.clear();
	vector<unit>::iterator iter;
	while(!is_zero(tmp, len)) {
		unit bit = divide2(tmp, len);
		int index = curr_len / unit_len;
		if(index + 1 > out.size())
			out.push_back(0);
		iter = out.begin() + index;
		*iter |= (bit << (curr_len - index * unit_len));
		++ curr_len;
	}
	return curr_len;
}

void print_bits(vector<unit> v, int len) {
	string s(len, '\0');
	int index, bit;
	for(int i=len; i>0; i--) {
		index = i / UNIT_LEN;
		bit = (i-1) % UNIT_LEN;
		s[len-i] = '0'+ ((v[index]>>bit)&1);
	}
	cout<<len<<" "<<s<<endl;
}

bool is_zero(byte *s, int len) {
	for(int i=0; i<len; i++)
		if(s[i] != 0) return false;
	return true;
}

unit divide2(byte *s, int len) {
	unit ret = s[len-1] % 2;
	byte flag = 0;
	for(int i=0; i<len; i++) {
		int val = flag * 10 + s[i];
		flag = val % 2;
		s[i] = val / 2;
	}
	return ret;
}

int main(int argc, char** argv) {
	vector<unit> bits;
	int bit_len;

#ifdef __GNUC__
	if(argc != 2) {
		cout<<"Usage: "<<argv[0]<<" "<<"<number>"<<endl;
		exit(1);
	}
	bit_len = string_to_bits(string(argv[1]), bits);
	print_bits(bits, bit_len);
#endif

#if defined(_WIN32) || defined(_WIN64)
	bit_len = string_to_bits(string("12345"), bits);
	print_bits(bits, bit_len);

	bit_len = string_to_bits(string("4000004"), bits);
	print_bits(bits, bit_len);

	bit_len = string_to_bits(string("1234567890012345678900123456789"), bits);
	print_bits(bits, bit_len);

	system("pause");
#endif
}