# Funny_notes

## 1. Difference between Parameter and Argument

Argument and Parameter are terms associated with functions. The key difference between them is that **an argument is the data passed at the time of calling a function, while a parameter is a viriable defined by a function that receives a value when the function is called**.  

```C
/* Here a and b are parameters. */
int add(int a, int b)
{
    return a+b;
}

int main()
{
    int a = 1;
    int b = 2;

    /* Here a and b are arguments. */
    int c = add(a,b);
    return c;
}
```

## 2. Some words

Annotation 注释
