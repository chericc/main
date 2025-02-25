# 摊还分析

在摊还分析中，我们求数据结构的一个操作序列中所执行的所有操作的平均时间，来评价操作的代价。这样，我们就可以说明一个操作的平均代价是很低的，即使序列中某个单一操作的代价很高。摊还分析不同于平均情况分析，它并不涉及概率，它可以保证最坏情况下每个操作的平均性能。

## 聚合分析

利用聚合分析，我们证明对所有$n$，一个$n$个操作的序列最坏情况下花费的总时间为$T(n)$，因此，在最坏的情况下，每个操作的平均代价，或**摊还代价**为$T(n)/n$。

以栈操作为例：

### 栈操作

我们增加一个新的栈操作`multipop(k)`，它删除栈$S$栈顶的$k$个对象，如果栈中对象数少于$k$，则将整个栈的内容都弹出。

代码如下：

```c++
void multipop(k) {
    while (!empty() && k > 0) {
        pop();
        k -= 1;
    }
}
```

我们分析一下由$n$个`push`、`pop`和`multipop`组成的操作序列在一个空栈上的执行情况。序列中一个`multipop`操作的最坏情况代价为$O(n)$，因为栈的大小最大为$n$。因此，任意一个栈操作的最坏情况时间为$O(n)$，从而一个$n$个操作的序列的最坏情况代价为$O(n^2)$。虽然这个分析是正确的，但是通过单独分析每个操作的最坏情况代价得到的操作序列的操作序列的最坏情况时间并不是一个确界。

通过使用聚合分析，我们考虑整个序列的$n$个操作，可以得到更好的上界。实际上，虽然一个单独的`multipop`操作可能代价很高，但在一个空栈上执行$n$个`push`、`pop`和`multipop`的操作序列，代价至多是`O(n)`。分析如下：

当将一个对象压入栈后，我们至多将其弹出一次，因此，对于一个非空的栈，可以执行的`pop`的次数（包括`multipop`中的`pop`）最多与`push`操作的次数相当，即最多$n$次。因此，对任意的$n$值，任意一个由$n$个`push`、`pop`和`multipop`组成的操作序列，最多花费$O(n)$的时间。一个操作的平均时间为$O(n)/n=O(1)$。在聚合分析中，我们将每个操作的摊还代价设定为平均代价。因此在此例中，所有三种栈操作的摊还代价都是$O(1)$。

## 核算法

用核算法进行摊还分析时，我们对不同操作赋予不同费用，赋予某些操作的费用可能多与或少于其实际代价。我们将赋予一个操作的费用称为它的摊还代价。当一个操作的摊还代价超出其实际代价时，我们将差额存入数据结构中的特定对象，存入的差额称为信用。对于后续操作中摊还代价小于实际代价的情况，信用可以用来支付差额。因此，我们可以将一个操作的摊还代价分解为其实际代价和信用。

如果用$c_i$表示第$i$个操作的真实代价，用$\hat{c_i}$表示其摊还代价，则对任意$n$个操作的序列，要求$$\displaystyle\sum_{i=1}^{n}\hat{c_{i}} \ge \sum_{i=1}^{n} c_{i}$$

数据结构中存储的信用恰好等于总摊还代价与总实际代价的差值，即$\displaystyle\sum_{i=1}^{n}\hat{c_{i}} - \sum_{i=1}^{n} c_{i}$。

还是以栈操作为例：

### 栈操作

