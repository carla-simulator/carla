#include <cstdio>
#include <cstring>
using namespace std;

int main()
{
	
	float arr[10];
	memset(arr, 0b0100, sizeof(arr));
	printf("%lf\n", arr[0]);
	return 0;
}