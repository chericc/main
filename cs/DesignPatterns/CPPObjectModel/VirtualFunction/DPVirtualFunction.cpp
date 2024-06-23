#include <Public/DPVirtualFunction.h>

#include <cstdio>
#include <iostream>
using std::cout;
using std::endl;

PRIVATE_BEGIN

/*
这里演示C++对象继承关系里函数地址的关系，特别是虚函数的实现方式。
*/

class BaseA {
   public:
    void ANormalOne() { cout << "BaseA::ANormalOne" << endl; }
    void ANormalTwo() { cout << "BaseA::ANormalTwo" << endl; }
    virtual void AVirtualOne() { cout << "BaseA::AVirtualOne" << endl; }
    virtual void AVirtualTwo() { cout << "BaseA::AVirtualTwo" << endl; }
};

class BaseB {
   public:
    void BNormalOne() { cout << "BaseB::BNormalOne" << endl; }
    void BNormalTwo() { cout << "BaseB::BNormalTwo" << endl; }
    virtual void BVirtualOne() { cout << "BaseB::BVirtualOne" << endl; }
    virtual void BVirtualTwo() { cout << "BaseB::BVirtualTwo" << endl; }
};

class BaseC {
   private:
    void test();
};

class DerivedA : public BaseA, public BaseB, public BaseC {
   public:
    // virtual void AVirtualOne() { cout << "DerivedA::AVirtualOne" << endl; }
    virtual void AVirtualTwo() { cout << "DerivedA::AVirtualTwo" << endl; }
    // virtual void BVirtualOne() { cout << "DerivedA::BVirtualOne" << endl; }
    virtual void BVirtualTwo() { cout << "DerivedA::BVirtualTwo" << endl; }
};

void Example() {
    /*
    DerivedA继承BaseA和BaseB。
    */
    DerivedA da;
    DerivedA* pda = &da;
    printf("address of DerivedA=%p\n", &da);

    /*
    有关类成员函数指针的一些测试。
    如何声明一个指向类的成员函数的指针？
    答：从语义上而言，由于最终调用需要一个具体的对象，因此这里的成员函数指针并不是一个独立的函数。
    我们可以尝试通过观察成员函数里实际保存的值，猜想出来编译器对C++对象模型的部分实现。
    */
    void (DerivedA::*pFunc)(void) =
        &DerivedA::ANormalOne;  // 定义一个成员函数指针
    (da.*pFunc)();              // . 调用
    (pda->*pFunc)();            // -> 调用

    void (DerivedA::*pVirtualFunc)(void) =
        &DerivedA::AVirtualOne;  // 这里指向的是一个虚函数的指针
    (da.*pVirtualFunc)();
    (pda->*pVirtualFunc)();

    /*
    这段代码存在编译警告，因为类的成员函数指针没有被合理的打印出来。
    这段代码在VC和GCC上的输出不一致。

    以下是针对GCC的一些结论：
    1.类的普通成员函数指针指向代码段一个固定的位置。这个位置似乎在代码段。
    2.类的虚函数为一个较小的值。对于同一个虚函数，这个值在基类和子类中的值有一致的，也有不一致的。并且似乎同一个类中，这个值总是有序增加的。
    */
#if false
	printf("address of BaseA::ANormalOne=%p\n", &BaseA::ANormalOne);
	printf("address of BaseA::ANormalTwo=%p\n", &BaseA::ANormalTwo);
	printf("address of BaseA::AVirtualOne=%p\n", &BaseA::AVirtualOne);
	printf("address of BaseA::AVirtualTwo=%p\n", &BaseA::AVirtualTwo);

	printf("address of BaseB::BNormalOne=%p\n", &BaseB::BNormalOne);
	printf("address of BaseB::BNormalTwo=%p\n", &BaseB::BNormalTwo);
	printf("address of BaseB::BVirtualOne=%p\n", &BaseB::BVirtualOne);
	printf("address of BaseB::BVirtualTwo=%p\n", &BaseB::BVirtualTwo);

	printf("address of DerivedA::ANormalOne=%p\n", &DerivedA::ANormalOne);
	printf("address of DerivedA::ANormalTwo=%p\n", &DerivedA::ANormalTwo);
	printf("address of DerivedA::BNormalOne=%p\n", &DerivedA::BNormalOne);
	printf("address of DerivedA::BNormalTwo=%p\n", &DerivedA::BNormalTwo);
	//printf("address of DerivedA::AVirtualOne=%p\n", &DerivedA::AVirtualOne);
	printf("address of DerivedA::AVirtualTwo=%p\n", &DerivedA::AVirtualTwo);
	//printf("address of DerivedA::BVirtualOne=%p\n", &DerivedA::BVirtualOne);
	printf("address of DerivedA::BVirtualTwo=%p\n", &DerivedA::BVirtualTwo);
#endif

    /*
    试验环境为64bit。
    这里我们尝试分析虚函数表的构成。

    在GCC的内部实现中，实例化的类首地址存有一个指向虚函数表的指针。
    对于存在虚函数的BaseA和BaseB，其大小为8字节，这就是存储了一个指向虚函数表的指针。
    对于不存在虚函数的BaseC，其大小为1字节。这里填充一个字节主要是实例化需要有一个地址的需要。
    */
    cout << "sizeof(BaseA) = " << sizeof(BaseA) << endl;
    cout << "sizeof(BaseB) = " << sizeof(BaseB) << endl;
    cout << "sizeof(BaseC) = " << sizeof(BaseC) << endl;
    cout << "sizeof(DerivedA) = " << sizeof(DerivedA) << endl;

    {
        BaseA iBaseA;
        BaseB iBaseB;
        DerivedA iDerA;

        /*
        这里，取出的地址存储的是指向虚地址表的指针，因此是一个二重指针。
        对这个二重指针进行取值操作，即可得到虚地址表里存储的地址。
        */
        intptr_t** ppBaseA = reinterpret_cast<intptr_t**>(&iBaseA);
        intptr_t** ppBaseB = reinterpret_cast<intptr_t**>(&iBaseB);
        intptr_t** ppDerA_1 = reinterpret_cast<intptr_t**>(&iDerA);
        intptr_t** ppDerA_2 =
            reinterpret_cast<intptr_t**>((char*)(&iDerA) + sizeof(intptr_t));

        /*
        虚函数表里存储的应该是虚函数的实际地址，因此虚函数表与虚函数的个数对应。
        注意这里的写法，pp是一个指向指针数组的指针，因此必须先求数组在进行求子元素运算

        另注：在VisualStudio下，虚函数表以0结尾，而在GCC下，虚函数表并没有以0结尾，因此取参数时使用固定的值。

        在获取到虚函数表后，我们将它转换为一个普通函数进行调用。注意，由于虚函数很简单，没有使用this指针，因此行为是正常的。
        我们可以发现，每个类的虚函数表依次存储着这个类的虚函数地址。
        对于子类，重写了父类的方法的，则指针指向新的地址，没有重写的，使用原地址。
        有多个基类的，对应有多个虚函数表指针。
        */
        typedef void (*FuncType)(void);
        int i = 0;
        while (/*(*ppBaseA)[i] != 0 && */ i < 2) {
            intptr_t p1 = (*ppBaseA)[i];
            cout << "iBaseA[][" << i << "] = " << (void*)p1 << endl;
            ((FuncType)p1)();
            ++i;
        }
        i = 0;
        while (/*(*ppBaseB)[i] != 0 && */ i < 2) {
            intptr_t p1 = (*ppBaseB)[i];
            cout << "iBaseB[][" << i << "] = " << (void*)p1 << endl;
            ((FuncType)p1)();
            ++i;
        }
        i = 0;
        while (/*(*ppDerA_1)[i] != 0 && */ i < 2) {
            intptr_t p1 = (*ppDerA_1)[i];
            cout << "iDerA_1[][" << i << "] = " << (void*)p1 << endl;
            ((FuncType)p1)();
            ++i;
        }
        i = 0;
        while (/*(*ppDerA_2)[i] != 0 && */ i < 2) {
            intptr_t p1 = (*ppDerA_2)[i];
            cout << "iDerA_2[][" << i << "] = " << (void*)p1 << endl;
            ((FuncType)p1)();
            ++i;
        }
    }
}

PRIVATE_END

DP_BEGIN

void DPVirtualFunction::Play() { Example(); }

DP_END