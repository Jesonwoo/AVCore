
#include <common/corelog.hpp>
#include <Windows.h>

extern void TestFFVideoEncoder();
extern void TestLogurur();

int main()
{
	corelog::init(0);
    TestFFVideoEncoder();
	//TestLogurur();
    system("pause");
    return 0;
}