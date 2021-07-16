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
    print(y); // 0 is printed on console
}

```
