
#include <stdio.h>
#include <array>
#include <vector>
#include <stdint.h>

int price(int len)
{
    static std::array<int,32> price_array{0,1,5,8,9,10,17,17,20,24,30};
    return price_array[len];
}

int price_rod(int rod_len, uint64_t &times)
{
    ++times;

    if (0 == rod_len)
    {
        return 0;
    }

    int price_max = 0;
    int cut_len = 0;
    for (int i = 1; i <= rod_len; ++i)
    {
        int price_tmp = price(i) + price_rod(rod_len - i, times);
        if (price_tmp > price_max)
        {
            cut_len = i;
            price_max = price_tmp;
        }
    }

    return price_max;
}

int main()
{
    uint64_t times = 0;
    int price_rod_max = price_rod(10, times);
    printf("price=%d, times=%lu\n", price_rod_max, times);
    return 0;
}