# CMM (C--) Language
CMM or C-- programming language is a a stripped-down version of C language, because it has the similar syntax with many restrictions (such as the absence of many libraries or dynamic memory allocation). All this means that CMM is a full-Turing language.

Сontent:
* [CMM Syntax](#cmm-syntax)
* [CMM Frontend](#cmm-frontend)
* [CMM Optimizer](#cmm-optimizer)
* [CMM Reversed frontend](#cmm-reversed-frontend)
* [CMM Backend](#cmm-backend)

## CMM Syntax

Let's begin with some basic things. 
1. The single data type in CMM is a `double` type.
2. That is why there is no sense to define return-value type.
3. Every program must have its entry point — `main` function. Example:

```C++
main
{
    print(5); // number 5 is printed in console.
    // Also you can use such comments, as in C++.
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
                // also there are arcsin, arccos, arctg and arcctg
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
10. Up to this point we have considered only `print` i/o function. But also there is a `scan` function, which gets the single variable as argument. Example: `scan(x);`.
11. Also there is a `power` function, which work in the same way is `^` operator: `power(x, 2);` is `x^2`.
12. Unique feature of CMM language is that it supports derivative calculations. For example:
```C++
main
{
    x = 8;
    y = deriv(x, x^4 + sin(x) + 999); // the same as // y = 4 * x^3 + cos(x)
    // the first  argument is a variable with respect to which the derivative is taken
    // the second argument is some exxpression
}
```
13. As in C language, there are conditional jumps:

```C++
main
{
    scan(x);
    scan(y);
    
    if (x == y) // expression in brackets ALWAYS contains comparison sign
    {
        print(x * y);
        x = x + y;
    }
    // else is not supported in this case, so use "if" again
    
    if (x != y)
    {
        print(6); // even in case of a single command in "if" body, braces (`{' or '}') are ALWAYS USED
    }
    
    while (x > y)  // all rules with "while" are the same
    {             
        y = y + 1; // here such cases as "while (7 > 5)" always compare these numbers, but 
        print(y);  // there is no sence to write in such way, because command "break" is absent
    }
}
```
14. The last important thing that multi-function calls are supported in the same way as in C, so recursion is supported too:

```C++
main
{
    x = 13;
    print(fibonach(x));
}

fibonach (x)
{
    if (x == 0)
    {
        return 0;
    }
	
    if (x == 1)
	{
        return 1;
    }
	
    c = fibonach(x - 1) + fibonach(x - 2);
	
    return c;
}
```

## CMM Frontend

The aim of CMM frontend is to parse the syntax of written program and create tree-file in the following Extended Backus–Naur Form (EBNF):

* AbstractSyntaxTree ::= ‘{‘ FunctionDeclaration ‘}’
* FunctionDeclaration ::= “nil” | "function-declaration" '{' FunctionImplementation '}' '{' FunctionDeclaration '}'
* FunctionImplementation ::= Identifier ‘{‘ FunctionImplementationParameter ’}’ ’{‘ Block ‘}’
* Identifier ::= ['a'-'z' 'A'-'Z' '_'] ['a'-'z' 'A'-'Z' '0'-'9' '_']*
* FunctionImplementationParameter ::= “nil” | "function-implementation-parameter" '{' FunctionImplementationParameter '}' '{' Variable '}'
* Variable ::= Identifier “{nil} {nil}”
* Block ::= Concatenation
* Concatenation ::= “nil” | "concatenation" '{' Statement '}' '{' Concatenation '}'
* Statement ::= If | While | Operator
* If ::= "if" '{' Condition '}' '{' Block '}'
* While ::= "while" '{' Condition '}' '{' Block '}'
* Сondition ::= Expression
* Operator ::= AssignmentOperator | FunctionСallOperator | ReturnOperator | MathematicalOperator
* AssignmentOperator ::= '=' '{' Variable '}' '{' Expression '}'
* FunctionCallOperator ::= ‘$’ Identifier '{' FunctionCallParameter '}' "{nil}"
* FunctionCallParameter ::= “nil” | "function-call-parameter" '{ FunctionCallParameter '}') '{' Expression '}'
* ReturnOperator ::= "return" '{' Expression '}' "{nil}"
* MathematicalOperator ::= ['+' '-' '<' "<=" "==" '>' ">=" "||" '*' '/' "&&" '^'] '{' Expression '}' '{' Expression '}'
* Expression ::= ExpressionPrimeTerm | FunctionCallOperator | MathematicalOperator
* ExpressionPrimeTerm ::= Variable | Number
* Number ::= ['0'-'9']+ “{nil} {nil}“

To convert program into tree-file use `frontend.exe` in console:
```batch
"frontend.exe" source.cmm tree_file.tree
```

## CMM Optimizer

Optimizer is language-independent part of CMM pack. It optimizes tree and calculate all derivatives. To launch optimizer use `optimizer.exe`:
```batch
"optimizer.exe" tree_file.tree optm_tree.tree
```

## CMM Reversed frontend

The aim of this part of CMM pack is to convert tree into CMM program. Everything is easy. Just call:
```batch
"frontend_rev.exe" file.tree program.cmm
```

## CMM Backend

CMM Backend convert `.tree` file into `VASM (Vokerlee assembler)`, description of which you [can read here](https://github.com/Vokerlee/Introduction-to-compiler-technologies/tree/master/5.%20nCPU). 
```batch
"backend.exe" file.tree asm_code.vasm
```

After all actions you should compile `.vasm` file with `asm.exe`:
```batch
"asm.exe" asm_code.vasm exe_code.ncpu
```

And such binary code is to be launched by `nCPU.exe`:

```batch
"nCPU.exe" exe_code.ncpu
```
To know in more detail  how to work with `VASM` and compile `.vasm` programs you [can read here](https://github.com/Vokerlee/Introduction-to-compiler-technologies/tree/master/5.%20nCPU#compilation).
