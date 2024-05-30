# complexity

## Master theory

**定理4.1** 零$a\ge 1$和$b\gt 1$是常数，$f(n)$是一个函数，$T(n)$是定义在非负整数上的递归式：

$$
T(n)=aT(n/b)+f(n)
$$

其中，我们将$n/b$解释为$\lfloor n/b\rfloor$或$\lceil n/b\rceil$。那么$T(n)$有如下渐进界：

1. 若对某个常数$\epsilon \gt 0$有$f(n)=\Omicron(n^{\log_b {(a-\epsilon)}})$，则$T(n)=\Theta(n^{\log_b{a}})$。
2. 若$f(n)=\Theta(n^{\log_b a})$，则$T(n)=\Theta(n^{\log_b a}\lg n)$。
3. 若对某个常数$\epsilon \gt 0$有$f(n)=\Omega(n^{\log_b (a+\epsilon)})$，且对某个常数$c<1$和所有足够大的$n$有$af(n/b)\le cf(n)$，则$T(n)=\Theta (f(n))$。