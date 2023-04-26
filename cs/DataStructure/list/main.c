
#include <stdio.h>

#include "list.h"

void test() {
    int arr[] = {1,3,5,2,4,6};
    NODE *list = list_gen(arr, sizeof(arr)/sizeof(arr[0]));
    list_print (list);

    list = list_reverse(list);
    list_print(list);
    return ;
}

void test2() {
    int arr[] = {1,3,5,2,4,6};
    NODE *list = list_gen(arr, sizeof(arr)/sizeof(arr[0]));
    list_print (list);

    list_eg1(list);
    list_print(list);
    return ;
}

void test3() {
    int arra[] = {1,3,5};
    NODE *lista = list_gen(arra, sizeof(arra)/sizeof(arra[0]));
    list_print (lista);

    int arrb[] = {2,4};
    NODE *listb = list_gen(arrb, sizeof(arrb)/sizeof(arrb[0]));
    list_print (listb);

    NODE *list = list_merge(lista, listb);
    list_print(list);
    return ;
}

void test4() {
    int arra[] = {1,2,3};
    NODE *lista = list_gen(arra, sizeof(arra)/sizeof(arra[0]));
    list_print (lista);

    int arrb[] = {4,5};
    NODE *listb = list_gen(arrb, sizeof(arrb)/sizeof(arrb[0]));
    list_print (listb);

    NODE *list = list_append(lista, listb);
    list_print(list);
    return ;
}

int main() {
    printf ("this is amazing!\n");

    // test();
    // test2();
    // test3();
    test4();

    return 0;
}