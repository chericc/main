# steps

## test_main.cc

```c++
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

```

## field_trials.py

```bash
python3 field_trials.py header > registered_field_trials.h
```

## protobuf

```
sudo apt-get install protobuf-compiler

protoc debug.proto --cpp_out=.
```