#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

int num(char c) {
	if(c < '0' || c > '9') {
		cerr<<"invalid paratemer"<<endl;
		exit(1);
	}
	return c-'0';
}

char* string_multiplication(char *op1, char *op2) {
	int len1 = strlen(op1);
	int len2 = strlen(op2);
	char *result = new char[len1+len2];
	for(int i=0; i<len1+len2; i++)
		result[i] = 0;
	char *p1 = op1+len1;
	int max_index = 0;
	while(--p1 >= op1) {
		char *p2 = op2+len2;
		while(--p2 >= op2) {
			int l1 = op1 + len1 - 1 - p1;
			int l2 = op2 + len2 - 1 - p2;
			int index = l1 + l2;
			int val = result[index] + num(*p1) * num(*p2);
			result[index] = val % 10;
			int carry = val / 10;
			while(carry > 0) {
				++index;
				val = result[index] + carry;
				result[index] = val % 10;
				carry = val / 10;
			}
			if(max_index < index) 
				max_index = index;
		}
	}
	//process done
	char *result_str = new char[max_index+1];
	char *r1 = result+max_index;
	char *r2 = result_str;
	while(r1 >= result) 
		*(r2++) = *(r1--) + '0';
	*r2 = '\0';
	delete []result;
	return result_str;
}

int main(int argc, char** argv) {
#ifdef __GNUC__
	if(argc != 3) {
		cout<<"Usage: "<<argv[0]<<" op1 op2"<<endl;
		exit(1);
	}
	char *op1 = argv[1];
	char *op2 = argv[2];
#else
	char *op1 = "1234";
	char *op2 = "5678";
#endif
	cout<<op1<<" * "<<op2<<" = "<<string_multiplication(op1, op2)<<endl;
#if defined(_WIN32) || defined(_WIN64)
	system("pause");
#endif
}
