# CMM (C--) Language
CMM or C-- programming language is a a stripped-down version of C language, because it has the similar syntax with many restrictions (such as the absence of many libraries or dynamic memory allocation).

* [CMM Syntax](#cmm-syntax)

## CMM Syntax

Let's begin with some basic things. 
1. The single data type in CMM is a `double` type.
2. That is why there is no sense to define return-value type.
3. Every program must have its entry point — `main` function Example:

```C++
main
{
    print(5); // number 5 is printed in console.
    // Also you can use such comments, as in C++
}
```
4. There is an important feature: all functions don't need any declaration before its usage. So `main` functions should be always the first. Example:

```C++
main
{
    call_print();
}

call_print()
{
    print(5);
}
```
5. Working with variables is the following: they don't need any declaration; the first usage of certain variable is its first declaration (as in Basic language). Example:
```C++
main
{
    print(x); // is possible, but printed value is undefined
    
    y = 0;
    print(y); // 0 is printed in console
}
```
6. To calculate some expressions the following syntax is used:

```C++
main
{
    x = 0;
    y = 8.8;
    
    x = x + 7;
    y = y - x * y;             // is possible
    x = x^(x * y - 8 * x / y); // is possible too
    
    y = sin(x); // cos, tg, ctg, sh, ch, th, cth, ln can be used too
    
}
```
7. Some questionable actions, such as division by 0, can cause the result that variable is corrupted (has NAN value, for example), so be very careful.
8. To pass variables to functions you should do such actions:

```C++
main
{
    x = 7;
    y = 9;
    
    print_2values(x, y);
}

print_2values(arg1, arg2)
{
    print(arg1);
    print(arg2);
}
```
9. Up to this point we have considered only `"void"` functions. But functions can return the value:

```C++
main
{
    x = 8;
    y = get_value(x);
}

get_value(arg)
{
    return arg + 8; // to write more beautiful you can write // return (arg + 8);
}
```
