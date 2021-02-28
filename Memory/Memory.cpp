#include<iostream>
#include<Windows.h>
#include"Alloctor.h"
using namespace std;
struct node{
	int a = 0;
	//char* ch[10];
};
int n = sizeof(node);

int main()
{
	char* data[14];
	for (size_t i = 0; i < 12; ++i) {
		data[i] = new char;
	}

	for (size_t i = 0; i < 12; ++i) {
		delete data[i];
	}
	//std::cout << ptr->a << std::endl;
	Sleep(1000000);
}
