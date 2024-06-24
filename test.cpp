#include <iostream>
#include <random>
#include <bitset>
#include <climits> // For UINT_MAX
#include <algorithm>
#include <map>
#include <time.h>

static unsigned int g_seed;
inline void mySrand(int seed) { g_seed = seed; }
inline int myRandInt()
{
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}
inline int myRandInt(unsigned int maxSize) { return myRandInt() % maxSize; }
inline int myRandInt(int a, int b) { return (a + myRandInt(b - a)); }

int main()
{
	unsigned int solution = 0;
	int i = 0;
	while (i < 16)
	{
		solution |= 3 << (30 - i * 2);
		++i;
	}
	std::cout << solution << std::endl;
	return 0;
}
