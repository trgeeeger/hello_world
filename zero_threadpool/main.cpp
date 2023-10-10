#include<iostream>
#include<thread>
using namespace std;

void func1()
{
    cout<<"fun1 into"<<endl;
}

void func2(int a,int b)
{
    cout<<"fun2 a+b"<<a+b<<endl;
}

void func3(int &c)
{
    cout<<"fun3 c="<<&c<<endl;
    c+=10;
}

class A
{
public:
    void func4(int a)
    {
   //     std::this_thread::sleep_for(std::chrono::second(1));
        cout<<"thread:"<<name_<<",fun4 a="<<a<<endl;
    }
    void setName(string name)
    {
        name_=name;
    }
    void dispalyName()
    {
        cout<<"this:"<<this<<",name:"<<name_<<endl;
    }
    void play()
    {
        cout<<"play call"<<endl;
    }
private:
    string name_;
};

void func5()
{
    cout<<"func5 into sleep"<<endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cout<<"func5 leave"<<endl;
}

void func6()
{
    cout<<"this is func6"<<endl;
}

int main()
{
    //传0个值
    cout<<"main1 start"<<endl;
    std::thread t1(func1);
    t1.join();

    ///////////////////传两个值
    cout<<"main2 start"<<endl;
    int a=10,b=20;
    std::thread t2(func2,a,b);
    t2.join();

    //////////////////传引用
    cout<<"main3 start"<<endl;
    int c=10;
    std::thread t3(func3,std::ref(c));
    t3.join();
    cout<<"main3 c = "<<c<<endl;

    //////////////////传类成员函数
    cout<<"main4 start"<<endl;
    A* ptr=new A();
    ptr->setName("darren");
    std::thread t4(&A::func4,ptr,10);
    t4.join();
    delete ptr;

    /////////////////detach
    cout<<"main5 start"<<endl;
    std::thread t5(&func5);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    t5.detach();
   // t5.join();
    cout<<"\n main5 end \n"<<endl;

    ////////////////move
    cout<<"main6 start"<<endl;
    int x=10;
    std::thread t6_1(&func6);
    std::thread t6_2(std::move(t6_1));
  //  t6_1.join();
    t6_2.join();
    return 0;
}