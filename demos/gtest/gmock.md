# gmock参考

## 单元测试简介

定义：一个单元测试是一段自动化的代码，这段代码调用被测试的工作单元，之后对这个单元的单个最终结果的某些假设进行校验。单元测试几乎都是用单元测试框架编写的；只要产品代码不发生变化，单元测试的结果是稳定的（引用：《单元测试的艺术》）。

优点：

1. 单元测试是验证代码功能的最有效办法。可以避免人的不可靠性引入的缺陷。
2. 单元测试帮我们塑造设计。如果一个被测对象很难测试，则这个对象往往存在高耦合、边界不清晰、功能不明确等问题，就需要思考设计是否需要优化。
3. 单元测试是最好的文档。单元测试给出了功能说明、使用方式，能让其它人很轻松的了解一个模块。

特性：

1. 自动化；可重复执行；容易执行；任何人都可以执行（使用起来没有难度）
2. 运行快（使用起来不耗时。如果很慢，就会影响测试积极性）
3. 结果稳定（结果可预期）
4. 完全隔离（不会受其它测试干扰）

## 单元测试框架

单元测试一般都是用单元测试框架编写的。

GoogleTest测试框架是谷歌的C++测试和模拟框架。GoogleTest可以作为单元测试框架使用，但同时支持任意类型的测试（官方文档上如此描述）。


## mock、stub、fake

相同点：三者都是指被测试的程序依赖的一些外部对象，目的都是用来更快的获得我们需要的数据。
不同点：
fake对象通常是对生产环境的一个更快的模拟（比如耗时的操作）。fake强调对真实环境的简化。
stub对象通常是直接返回预定义的数据（而没有任务副作用）。stub强调对环境的替换，不访问真实数据或者不产生副作用。
mock对象会记录对它们的调用，随后可以验证对它们的调用符合预期。mock强调对调用的检验。
这些分类是对模仿外部对象的一个分类，测试过程中应该根据实际情况使用。
参考：https://dev.to/milipski/test-doubles---fakes-mocks-and-stubs

## 为何使用gmock

直接编码实现mock的问题

- 编码mock很枯燥且不容易复用（比如要监测一个接口被调用了几次，就需要实现一段代码）
- 编码质量可能不高（不好用，不好懂）

gmock的优势

- 提前优化设计（为了进行gmock测试，就需要将被测试对象和外部对象进行有效的隔离，从而间接实现了模块化）
- 测试依赖了一些昂贵资源（比如查询或者计算很耗时）
- 测试依赖了一些不容易获得的资源（比如测试环境根本没有网络）
- 测试例外情况，而例外是不容易触发的
- 需要关注模块和外部模块是如何交互的。仅仅通过交互结束之后的状态很难准确判断分析。
- 依赖的外部模块还不存在，并且也不想去手写模拟。

gmock的角色

- gmock可以作为一个设计工具使用。设计接口，并用gmock进行不断验证，这样可以产生更好的设计。
- gmock可以用来探查被测模块和外部模块的依赖并在这个过程中减少依赖。

## TDD（Test-Driven Development）

1. 对于一些很复杂的模块，修改之后涉及到大量的功能测试验证，费时费力。
2. 功能和需求是否对应，可能要等到功能已经做出来之后才发现做错了。
3. 由于重构会带来很多潜在问题，因此即使代码结构有很强的重构需求，也不敢轻易重构。
4. 如果一个模块的对外依赖是抽象接口，则意味着将这个模块的对外依赖进行替换之后，就能够在主机平台上执行测试。此时，嵌入式平台上不能运行或者不方便运行的第三方工具都可以用来对测试进行运行时检测，比如内存占用、内存泄漏、热点分析等等。

## 第三方工具（工具接口）

1.部分集成开发环境对测试框架（比如googletest）有适配，可以方便的进行单元测试，控制测试项，展示测试结果、测试覆盖率等内容。

## 使用gmock

### 使用gmock定义mock类

假设现在有一个外部类`Uploader`：

```C++
class UploaderInterface
{
public:
    virtual ~UploaderInterface() = default;
    virtual int connect() = 0;
    virtual int disconnect() = 0;
    virtual int upload(void *data, size_t size) = 0;
};
```

1. 定义一个mock类继承于外部类。
2. 在`public`中用`MOCK_METHOD`宏定义重写这些虚函数。`MOCK_METHOD`宏的书写方式可以参考gmock_cook_book.md。

就得到如下类：

```C++
class UploaderMock : public UploaderInterface
{
public:
    MOCK_METHOD(int, connect, (), (override));
    MOCK_METHOD(int, disconnect, (), (override));
    MOCK_METHOD(int, upload, (void*, size_t), (override));
};
```

### 文件组织

（建议）如果被mock的类属于内部类，则可以直接将mock类定义在测试源文件中（比如upload_test.cpp）。如果被mock的类是一个外部类（比如由其它小组管理），则可以将mock单独放在头文件中，并和被mock的类放在同一个模块内。

### mock类的使用

1. 引入mock类名（比如引入头文件，引入名字空间）
2. 创建一些mock对象
3. 给出对这些mock对象的预期（expectation），比如某个调用一共会被调用多少次（有很多预期的语法）
4. 利用这些mock对象对测试对象执行一些操作，同时还可以在这期间利用gtest进行检查操作结果。如果操作过程中，某个mock对象的调用不符合预期，就会打印错误。
5. 当mock对象被销毁时，其设定的所有预期都会被检查（因此需要注意每个测试场景中，对象不存在泄漏）。

注意：必须先设定对mock对象的调用的预期，然后才能执行对这些对象的调用（不能反过来）。但是可以不设置预期直接调用（这个调用就成了一个无关调用了，可以参考后续章节）。

### 设定预期

mock对象有效的关键在于设定预期。

#### 原则

预期既不能过于严格，也不能过于宽松。如果预期过于严格，则一些不相关的修改也会引起测试失败，如果预期过于宽松，则部分BUG会被忽略。

#### 用法

`EXPECT_CALL`宏的形式：

```c++
EXPECT_CALL(mock_object, method(matchers))
    .Times(cardinality)
    .WillOnce(action)
.WillRepeatedly(action);
```

`EXPECT_CALL`宏有两个参数，第一个是预期作用的mock对象，第二个是其成员函数和参数。宏后是对这个对象的预期。
第二个参数中的函数和参数的含义是“匹配”，只有匹配上的调用才会被记录。比如，预期某个接口会被调用一次，则只有匹配的调用被调用一次时才算作调用一次（如果最终多没有匹配上，则这个预期就没有满足而报错）。

#### 动作（action）

以预期的方式告诉某个接口将来会做什么，动作可以是简单的返回某个值，也可以是调用一个函数。

#### 几个特殊的例子

1.同一个预期型的多次设定`WillOnce`动作，最终返回的调用顺序由上至下。

```c++
TEST(Example, eg2)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(_,_))
        .WillOnce(Return(0))
        .WillOnce(Return(1))
        .WillOnce(Return(2));
    
    EXPECT_EQ(0, uploader->upload(nullptr, 0));
    EXPECT_EQ(1, uploader->upload(nullptr, 0));
    EXPECT_EQ(2, uploader->upload(nullptr, 0));

}
```

2.多个预期的依次设定，最终匹配的顺序和设定的顺序相反。

```c++
TEST(Example, eg1)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(_,_))
        .WillOnce(Return(0));
    EXPECT_CALL(*uploader, upload(_,1))
        .WillOnce(Return(0));
    EXPECT_CALL(*uploader, upload(_,0))
        .WillOnce(Return(-1));
    
    EXPECT_EQ(-1, uploader->upload(nullptr, 0));
    EXPECT_EQ(0, uploader->upload(nullptr, 1));
    EXPECT_EQ(0, uploader->upload(nullptr, 2));

}
```

这个例子中，如果是按定义顺序搜索，则会因为粘性问题报错。
3.黏性特点（sticky）

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(_,_))
        .WillOnce(Return(1));
    EXPECT_CALL(*uploader, upload(nullptr,0))
        .WillOnce(Return(0));
    
    EXPECT_EQ(0, uploader->upload(nullptr, 0));
    EXPECT_EQ(1, uploader->upload(nullptr, 0));

}
```

这里例子会报错，原因是匹配具有粘性。一个匹配即使已经达到了其匹配上限，也不会失效，反而会因为匹配次数超过了限定次数报错。

## 手册

### 创建mock类

#### `MOCK_METHOD`

```c++
class MyMock {
 public:
  MOCK_METHOD(ReturnType, MethodName, (Args...));
  MOCK_METHOD(ReturnType, MethodName, (Args...), (Specs...));
};
```

其中，前三个参数为函数的原型（拆分成三个部分），第四个参数为限定词，比如`const`, `override`, `noexcept`。

如果这些参数中存在逗号，则可以将参数取别名，或者将参数再加一层小括号。比如

```c++
MOCK_METHOD((std::pair<bool,int>),Method,());
```

如果需要mock一个C函数，可以将这个函数封装到一个类的函数中再mock。

#### 处理无关调用

如果一个调用没有设置预期（使用`EXPECT_CALL`），则在后续调用的时候会产生警告（默认行为）。这些警告是可以忽略的（不影响最终测试的结果）。
如果需要显式的指定这些调用是合理的或者不合理的，则可以使用`NiceMock`和`StrictMock`创建mock对象，使用`NiceMock`创建的对象触发了没有设置预期的调用时，不会产生警告，使用`StrictMock`创建的对象在这种情况下引起测试失败。
尽量不要显式指定这些要求，因为这会使得测试变得过于严格。

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    // uploader = std::make_shared<UploaderMock>();
    uploader = std::make_shared<NiceMock<UploaderMock>>();

    // ...

}
```

#### 忽略无关参数

如果某个接口参数很多，但是实际上只需要关注其中的部分参数，则可以用以下方法将参数简化：

```C++
class UploaderMockMy : public UploaderInterface
{
public:
    MOCK_METHOD(int, connect, (), (override));
    MOCK_METHOD(int, disconnect, (), (override));

    int upload(void *data, size_t size) override
    {
        return my_upload(size);
    }

    MOCK_METHOD(int, my_upload, (size_t));
};
```

#### mock具体类（concrete class）

一些被测对象可能依赖的是一个具体类，所谓具体类是相比抽象类而言的（具体类直接使用，抽象类需要具体化之后才能使用）。如果依赖的是一个抽象类，则所有的调用都是针对的抽象接口，如果是一个具体类，则程序可能会去访问类中的具体成员。

不应该尝试去mock一个具体类，相反应该在模块设计的时候将外部依赖作为接口使用（面向接口编程）。

面向接口编程引入的问题：

- 增加了额外的抽象
- 引入了虚函数调用成本

面向接口编程带来的好处：

- 更容易编写测试代码
- 面向接口进行编程时，可以很容易的扩展或者修改这个接口类。如果使用具体类，由于一个具体类并不一定只服务当前模块，因此不方便扩展或修改这个类。
- 类的实现需要改变时，如果是接口类，则只需要改变这个接口类的具体化即可，如果是实体类，则所有引用的位置都可能需要调整。

（引用）尽管是否使用面向接口编程处理特定的问题值得权衡，但是这个技术在java社区中已经有相当多的应用，并且也被证明在很多场景中是有效的。

#### 把调用委托给fake对象

mock可以以代理的方式使用一个fake对象。示例如下：

```c++
class RecordInterface
{
public:
    virtual ~RecordInterface() = default;
    virtual int init(Duration duration, Callback cb) = 0;
    virtual int getFrames() = 0;
};
```

这是一个查询卡录像的接口类，初始化时给定查询范围和回调函数，然后调用`getFrames`获取录像帧。这个接口类的模拟是比较复杂的（可能要记录传入的时间段和回调，并模拟返回帧），因此直接创建一个fake对象进行实例化。

```c++
class RecordFake : public RecordInterface
{
public:
    int init(Duration duration, Callback cb) override
    {
        // some implementation
        return 0;
    }

    int getFrames() override
    {
        // some implementation
        return 0;
    }

protected:
    // data member ...
};
```

有了fake对象之后，再通过代理的方式创建mock类：

```c++
class RecordMock : public RecordInterface
{
public:
    MOCK_METHOD(int, init, (Duration duration, Callback cb), (override));
    MOCK_METHOD(int, getFrames, (), (override));

    void DelegateToFake()
    {
        ON_CALL(*this, init)
            .WillByDefault([this](Duration duration, Callback cb)
                {
                    return _fake.init(duration, cb);
                });
        ON_CALL(*this, getFrames)
            .WillByDefault([this]()
                {
                    return _fake.getFrame();
                });
    }

protected:
    RecordFake _face;
};
```

注意：

应该尽量保证测试类角色的单一，不要混淆一个mock类和fake类。一个测试类扮演的角色越多，则这个测试类能够被应用的场景就越窄。

#### 把调用委托给真实的对象

方法和委托给fake对象一致。

### 匹配器（matcher）

回顾`EXPECT_CALL`宏的形式，其第二个参数给出了匹配器。

```c++
EXPECT_CALL(mock_object, method(matchers))
    .Times(cardinality)
    .WillOnce(action)
	.WillRepeatedly(action);
```

当对一个对象设置预期时，这个预期的触发条件就是其调用和匹配器匹配。

#### 具体值匹配

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(nullptr, _))
    // EXPECT_CALL(*uploader, upload(Eq(nullptr), _))
        .WillOnce(Return(0));

    uploader->upload(nullptr, 0);
}
```

匹配`upload`第一个参数为`nullptr`的调用。

注：匹配器中直接写某个值，其含义是与这个值相等（和`Eq(nullptr)`含义相同）。

注："`_`"表示和任意值匹配。

#### 单条件匹配

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(_, Gt(5)))
        .WillOnce(Return(0));

    uploader->upload(nullptr, 6);
}
```

匹配调用`upload`第二个参数大于5 的调用。

#### 复合条件匹配

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(_, AllOf(Gt(10), Lt(15))))
        .WillOnce(Return(0));

    uploader->upload(nullptr, 11);
}
```

匹配`upload`第二个参数大于10且小于15的调用。

#### 匹配的搜索

当一个mock函数被调用时，越晚定义的预期越早匹配。这个特性可以用来实现不同的参数具有不同动作的效果。

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(_,_))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(*uploader, upload(nullptr,_))
        .WillRepeatedly(Return(-1));

    EXPECT_EQ(0, uploader->upload((void*)0x100, 0));
    EXPECT_EQ(-1, uploader->upload(nullptr, 0));
}
```

这个例子给定了`upload`返回0和-1的两种预期。

#### 整体参数匹配

除了关注单个参数，还可以利用`.With(matcher)`关注参数之间的关系。

```c++
class EncodeInterface
{
public:
    virtual ~EncodeInterface() = default;
    virtual int encode(const void *src,size_t src_size, void *dst, size_t dst_size) = 0;
};

class EncodeMock : public EncodeInterface
{
public:
    MOCK_METHOD(int,encode,(const void *src,size_t src_size, void *dst, size_t dst_size));
};

TEST(ExampleA, eg)
{
    EncodeMock encoder;
    EXPECT_CALL(encoder, encode(Ne(nullptr),Gt(0),Ne(nullptr),Gt(0)))
        .With(AllOf(Args<0,2>(Ne()),Args<1,3>(Le())))
        .WillRepeatedly(Return(0));

    void *p_src = (void*)0x100;
    void *p_dst = (void*)0x200;

    encoder.encode(p_src, 5, p_dst, 10);
    encoder.encode(p_src, 10, p_dst, 10);
}
```

首先对单个参数进行限制，要求输入指针和输出指针均不能为空指针，并且要求大小均大于0。在这个基础上，同时对参数之间的关系做出限制，要求输入指针不能等于输出指针，并且输入的大小不超过输出的大小。

#### 使用自定义谓词（predicate）

如果需要自定义谓词，可以用`Truly`加谓词来实现。

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    auto is_even = [](int i){return i % 2 ? false : true;};

    EXPECT_CALL(*uploader, upload(_,Truly(is_even)));

    uploader->upload((void*)0x1, 1000);
}
```

预期传入的大小为偶数。

#### 匹配不可复制的参数

在给定一个调用的预期时，预期的值会被拷贝存储。如果参数不能拷贝，可以用`std::ref`来进行对象化封装。

#### 匹配对象成员

如果传参类型为对象，则可以去匹配这个对象的数据成员或者属性（表现为无参常成员函数）。

```c++
class EncodeTask
{
public:
    const void *src{nullptr};
    size_t src_size{0};
    void *dst{nullptr};
    size_t dst_size{0};

    int id() const
    {
        return 100;
    }
};

class EncodeInterface
{
public:
    virtual ~EncodeInterface() = default;
    virtual int encode(const EncodeTask &task) = 0;
};

class EncodeMock : public EncodeInterface
{
public:
    MOCK_METHOD(int,encode,(const EncodeTask &task));
};

TEST(ExampleA, eg)
{
    EncodeMock encoder;

    // auto matcher = Field(&EncodeTask::src,Ne(nullptr));
    // auto matcher = Property(&EncodeTask::id,Ge(0));

    auto matcher = AllOf(
            Field(&EncodeTask::src,Ne(nullptr)),
            Property(&EncodeTask::id,Ge(0))
    );

    EXPECT_CALL(encoder, encode(matcher));
    
    EncodeTask task;
    task.src = (void*)0x100;

    encoder.encode(task);
}
```

这个例子中，将编码参数组合为一个任务对象，可以去单独匹配这个对象中的特定成员。

这种匹配方式中，如果要检查成员之间的关系，可以利用自定义谓词实现。

#### 匹配指针指向的值

可以利用`Pointee`来解引用并匹配指针指向的值，`Pointee`可以自动识别空指针并引发测试失败。形式如：`Pointee(matcher)`。

比如，预期一个`int *`形式的参数其指向的值为正：

```
EXPECT_CALL(mock, func(Pointee(Gt(0))));
```

#### 自定义匹配器

使用框架提供的`MATCHER`宏可以快速定义一个匹配器。

```c++
MATCHER(size_matcher, ""){return (arg % 10) == 0;}

TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(_,size_matcher()));

    uploader->upload((void*)0x1, 1000);
}
```

当这些匹配器不能很好的满足需求的时候，就需要创建一个自定义的匹配器。

```c++
class EncodeTask
{
public:
    const void *src{nullptr};
    size_t src_size{0};
    void *dst{nullptr};
    size_t dst_size{0};
};

class EncodeTaskMatcher
{
public:
    using is_gtest_matcher = void;

    explicit EncodeTaskMatcher(int max_size)
        : _max_size(max_size) {}

    bool MatchAndExplain(const EncodeTask &task,
                         std::ostream * /* listener */) const
    {
        return (task.src != task.dst)
            && (task.src_size <= task.dst_size)
            && (task.src_size <= _max_size)
            && (task.dst_size <= _max_size);
    }

    void DescribeTo(std::ostream *os) const
    {
        *os << "task valid ";
    }

    void DescribeNegationTo(std::ostream *os) const
    {
        *os << "task invalid ";
    }

private:
    const int _max_size;
};

class EncodeInterface
{
public:
    virtual ~EncodeInterface() = default;
    virtual int encode(const EncodeTask &task) = 0;
};

class EncodeMock : public EncodeInterface
{
public:
    MOCK_METHOD(int,encode,(const EncodeTask &task));
};

TEST(ExampleA, eg)
{
    EncodeMock encoder;

    auto matcher = AllOf(EncodeTaskMatcher(1024));

    EXPECT_CALL(encoder, encode(matcher));
    
    EncodeTask task;
    task.src = (void*)0x100;
    task.dst = (void*)0x200;
    task.src_size = 100;
    task.dst_size = 100;

    encoder.encode(task);
}
```

#### 匹配容器

框架提供了一些工具用来匹配容器中的元素，比如：

```
ElementsAre(1, Gt(0), _, 5)) // 有4个元素，且分别满足上述条件
UnorderedElementsAre(1, Gt(0), _, 5)) // 存在4个元素，且满足上述条件
```

#### 匹配器的注意事项

匹配器必须是能反复使用的，因此不能有任何副作用。

### 设定预期（expectation）

#### 克制的设定预期

设定不合理的预期，不如不设定预期，因为不合理的预期是对被测对象的不合理限制，这会让被测对象难以维护。

#### 不允许调用接口

设定调用次数为0即可。

```
    EXPECT_CALL(*uploader, upload)
        .Times(0);
```

注：如果不关注参数匹配，则不写即可。

#### 设定调用顺序

如果存在多个预期，则出现某个调用时会逐个匹配。此时并没有要求后定义的预期需要匹配到。要设定调用的顺序，需要显式的指定。

单个顺序指定：

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    {
        InSequence seq;
        EXPECT_CALL(*uploader, connect);
        EXPECT_CALL(*uploader, upload);
        EXPECT_CALL(*uploader, disconnect);
    }

    uploader->connect();
    uploader->upload((void*)0x100, 0);
    uploader->disconnect();
}
```

多个顺序要求（有向无环图，DAG）：

```c++
class MyTestInterface
{
public:
    virtual ~MyTestInterface() = default;
    virtual void init() = 0;
    virtual void deinit() = 0;
    virtual void do_sth_a() = 0;
    virtual void do_sth_b() = 0;
};

class MyTest : public MyTestInterface
{
public:
    MOCK_METHOD(void,init,(), (override));
    MOCK_METHOD(void,deinit,(), (override));
    MOCK_METHOD(void,do_sth_a,(), (override));
    MOCK_METHOD(void,do_sth_b,(), (override));
};

TEST(ExampleA, eg)
{
    MyTest test;
    Sequence seq_a, seq_b;

    /**
     *         -- do_sth_a -- 
     * init -- |            | -- deinit
     *         -- do_sth_b -- 
     */

    EXPECT_CALL(test,init)
        .InSequence(seq_a, seq_b);

    EXPECT_CALL(test,do_sth_a)
        .Times(AnyNumber())
        .InSequence(seq_a);

    EXPECT_CALL(test,do_sth_b)
        .Times(AnyNumber())
        .InSequence(seq_b);

    EXPECT_CALL(test,deinit)
        .InSequence(seq_a, seq_b);
    
    test.init();
    test.do_sth_b();
    test.do_sth_a();
    test.deinit();
}
```

#### 控制预期的失效（Retire）

匹配搜索的顺序：参考匹配器章节。

预期不会自动失效，除非显式指定。

```c++
TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    EXPECT_CALL(*uploader, upload(_,_))
        .WillOnce(Return(-1));
    EXPECT_CALL(*uploader, upload(_,2))
        .WillOnce(Return(0));

    uploader->upload(nullptr, 2);
    // uploader->upload(nullptr, 2);
}
```

如果再次调用，就会报错，因为第一个预期并没有失效。

可以指定`.RetiresOnSaturation()`令预期触发后自动失效。

### 动作（action）

#### 定制动作

如果内置的动作不合适，就可以使用定制的动作。

```c++
int func_action_upload(void *data, size_t size)
{
    return 0;
}

class obj_action_upload
{
public:
    int upload(void *data, size_t size)
    {
        return 0;
    }
};

auto lambda_action_upload = [](void *data, size_t size){return 0;};

TEST(Example, eg)
{
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();
    obj_action_upload action_connect;

    EXPECT_CALL(*uploader, upload)
        .WillOnce(&func_action_upload)
        .WillOnce(Invoke(&action_connect,&obj_action_upload::upload))
        .WillOnce(lambda_action_upload);

    uploader->upload(nullptr, 0);
    uploader->upload(nullptr, 0);
    uploader->upload(nullptr, 0);
}

```

## 参考

> 《单元测试的艺术》
