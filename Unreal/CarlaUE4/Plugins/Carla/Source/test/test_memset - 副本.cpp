#include <cstdio>
#include <iostream>
#include <exception>
#include <cstring>
using namespace std;

int main()
{
	std::cout << 0.00001/0.0000000001 << std::endl;
	try{
		int y = 1 - 1;
		int x = 1 / y;
	}
	catch(const std::exception& e){
		std::cout << e.what() << std::endl;
	}
	std::cout << "finish" << std::endl;
	getchar();
	return 0;
}