# steps of coverage with gcov

## compile and linking 

```bash
mkdir build

cd build

g++ -c -fprofile-arcs -ftest-coverage -fprofile-abs-path ../test.cpp -o test.o

ls
test.gcno  test.o

g++ test.o -lgcov -o test.out

ls
test.gcno  test.o  test.out
```

## running

```bash
./test.out

ls
test.gcda  test.gcno  test.o  test.out
```

## parsing

```bash
# sudo apt-get install lcov

lcov -c -d . -o test.info

ls
test.gcda  test.gcno  test.info  test.o  test.out

genhtml -o test_coverage test.info

ls
test_coverage  test.gcda  test.gcno  test.info  test.o  test.out
```