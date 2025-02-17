# Machine Learning

## Basis

> What is Machine Learning?

Machine learning is a subdomain of computer science that focuses on algorithms which help a computer learn from data without explicit programming.

> AI vs ML vs DS

**Artificial intelligence** is an area of computer science, where the goal is to enable computers and amchines to perform human-like tasks and simulate human behavior.

**Machine learning** is subset of AI that tries to solve a specific problem and make predictions using data.

**Data science** is a field that attempts to find patterns and draw insights from data(might use ML!).

> Types of Machine Learning

**Supervised learning** - uses labeled inputs(meaning the input has a corresponding output label) to train models and learn outputs.

**Unsupervised learning** - uses unlabeled data to learn about patterns in data.

**Reinforcement learning** - agent learning in interactive environment based on rewards and penalties.

## Supervised Learning

Features: 

- Qualitative - categorical data(finite number of categories or groups).
- Quantitative - numerical valued data(could be discrete or continuous).

Supervised Learning Tasks

- Classification - predict discrete classes.
  - Binary classification
  - Multiclass classification
- Regression - predict continuous values

- How do we make the models learn?
- How can we tell whether or not it's learning?

### Supervised Learning Dataset

- Training dataset: loss is used to make adjustment(to model, called training).
- Validation dataset: reality check during/after traning to ensure model can handle unseen data(NO traning).
- Testing dataset: used as to check how generalizable the final chosen model is.

### Loss Functions

L1 Loss:

$loss=sum(|y_{real}-y_{predicted}|)$

L2 Loss:

$loss=sum((y_{real}-y_{predicted})^2)$

Binary Cross-Entropy Loss:



## env

Alternative: `colab.research.google.com`

`http://archive.ics.uci.edu/dataset/159/magic+gamma+telescope`

```bash
pip install --upgrade pip
pip install numpy
pip install pandas
pip install matplotlib
```

## dataset

```bash
        28.7967   16.0021  2.6449  0.3918  0.1982   27.7004    22.011  -8.2027   40.092   81.8828  g
0       31.6036   11.7235  2.5185  0.5303  0.3773   26.2722   23.8238  -9.9574   6.3609  205.2610  g
1      162.0520  136.0310  4.0612  0.0374  0.0187  116.7410  -64.8580 -45.2160  76.9600  256.7880  g
2       23.8172    9.5728  2.3385  0.6147  0.3922   27.2107   -6.4633  -7.1513  10.4490  116.7370  g
3       75.1362   30.9205  3.1611  0.3168  0.1832   -5.5277   28.5525  21.8393   4.6480  356.4620  g
4       51.6240   21.1502  2.9085  0.2420  0.1340   50.8761   43.1887   9.8145   3.6130  238.0980  g
...         ...       ...     ...     ...     ...       ...       ...      ...      ...       ... ..
19014   21.3846   10.9170  2.6161  0.5857  0.3934   15.2618   11.5245   2.8766   2.4229  106.8258  h
19015   28.9452    6.7020  2.2672  0.5351  0.2784   37.0816   13.1853  -2.9632  86.7975  247.4560  h
19016   75.4455   47.5305  3.4483  0.1417  0.0549   -9.3561   41.0562  -9.4662  30.2987  256.5166  h
19017  120.5135   76.9018  3.9939  0.0944  0.0683    5.8043  -93.5224 -63.8389  84.6874  408.3166  h
19018  187.1814   53.0014  3.2093  0.2876  0.1539 -167.3125 -168.4558  31.4755  52.7310  272.3174  h

[19019 rows x 11 columns]
```