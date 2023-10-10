#include <iostream>
#include <zero_threadpool.h>
using namespace std;
void func0()
{
cout << "func0()" << endl;
}
void func1(int a)
{
cout << "func1() a=" << a << endl;
}
void func2(int a, string b)
{
cout << "func1() a=" << a << ", b=" << b<< endl;
}
void test1() // 简单测试线程池
{
ZERO_ThreadPool threadpool;
threadpool.init(2);
threadpool.start(); // 启动线程池
5 异常处理
C++ Core Guidelines (isocpp.github.io)
std::exception_ptr (Utilities) - C++ 中文开发手册 - 开发者手册 - 腾讯云开发者社区-腾讯云
(tencent.com)
make_exception_ptr - C++ Reference (cplusplus.com)
重点参考：MSVC中的异常处理 | Microsoft Docs
// 假如要执行的任务
threadpool.exec(1000,func0);
threadpool.exec(func1, 10);
threadpool.exec(func2, 20, "darren");
threadpool.waitForAllDone();
threadpool.stop();
}
int func1_future(int a)
{
cout << "func1() a=" << a << endl;
return a;
}
string func2_future(int a, string b)
{
cout << "func1() a=" << a << ", b=" << b<< endl;
return b;
}
void test2() // 测试任务函数返回值
{
ZERO_ThreadPool threadpool;
threadpool.init(2);
threadpool.start(); // 启动线程池
// 假如要执行的任务
std::future<decltype (func1_future(0))> result1 =
threadpool.exec(func1_future, 10);
std::future<string> result2 = threadpool.exec(func2_future, 20, "darren");
// auto result2 = threadpool.exec(func2_future, 20, "darren");
std::cout << "result1: " << result1.get() << std::endl;
std::cout << "result2: " << result2.get() << std::endl;
threadpool.waitForAllDone();
threadpool.stop();
}
int main()
{
// test1(); // 简单测试线程池
test2(); // 测试任务函数返回值
cout << "Hello World!" << endl;
return 0;
}