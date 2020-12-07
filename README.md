# Elite - Multi-Paradigm Stack-Based Interpreter

Elite is a Multi-Paradigm and Stack-Based interpreter written in C. It has a simple and easy grammar and works with a Compiler written following the `Pratt's Parser` technique and a Virtual Machine built using  `Direct Threading` dispatching. 

### Language Keywords
The interpreter currently supports a total of **22** keywords. Those are:
```
and | class | define | do | 
else | empty | exit | false | 
for | global | if | not | 
or | return | set | static |
super | true | this | undefined | 
void | while
```

## Building the interpreter

If you need to compile and build the project manually, here's how you can do it.

**1)** Clone this repository using `git` in your machine:
```
git clone https://github.com/Davi0k/elite.git
```
Otherwise, you can use the `GitHub CLI`:
```
gh repo clone Davi0k/elite
```
**2)** Create and switch to a new folder in which to build the interpreter:
```
mkdir build
cd build
```
	
**3)** Use `CMake` to build the project:
```
cmake -G "Unix Makefiles" ..
```
**Note:**  *The default generator is `Unix Makefiles`, if you want to change it use a different content for -G flag.*
	
**4)** Use `MakeFile` to compile the build produced by `CMake` and to create the interpreter's binaries:
```
make
```
**5)** Finally, you can verify the correct installation by running the **CLI** with the appropriate flag:
```
.\elite.exe -v
```
If the output correctly shows a version of the software, it means that the installation was successful.

**Remember to check the project requirements before moving on.**

## Requirements

The interpreter depends on some libraries and tools, make sure you have installed them correctly before trying to build the project.

#### Libraries

* The **GNU**  `Multiple Precision Arithmetic` (GMP) Library: [https://gmplib.org/](https://gmplib.org/)

#### Tools
* The `gcc` (**GNU** Compiler Collection) program or any other C compiler: [https://gcc.gnu.org/](https://gcc.gnu.org/)

* The `CMake` tool, used to execute the CMakeLists.txt file and create the necessary to compile the project: [https://cmake.org/](https://cmake.org/)

* The `Make` tool, used to compile the interpreter if you have used the `Unix Makefile` generator with `CMake`. Otherwise, you have to use the right tool depending on which generator you have used: [https://www.gnu.org/software/make/](https://www.gnu.org/software/make/)

## Using the CLI
To run a script, use the following dedicated Command-Line Interface (**CLI**) syntax:
```
.\elite.exe [path] [-v]
```

## Example scripts
A simple Arithmetic Calculator made using some Control-Flow statements.

```
set result: 0; #Define a global variable

#Use an infinite loop to allow more than one operation
while true: {
    print("Input two Numbers:");

    #Get inputs and convert them using the specifics Native Functions
    set x: number(input()), y: number(input()); 

    print("Insert an operator (+, -, *, /):");
    
    set operator: input();

    if operator == '+': result = x + y;
    if operator == '-': result = x - y;
    if operator == '*': result = x * y;
    if operator == '/': result = x / y;

    print("The result is: ", result); #Print the result
}
```

A simple Script to calculate the `factorial` and the corresponding number of the `fibonacci` series using recursive algorithms and measuring the execution time.

```
#Defining a recursive function to calculate the corresponding fibonacci number
define fibonacci(n) {
    if (n == 0) or (n == 1): 
	    return n;
    
    return fibonacci(n - 1) + fibonacci(n - 2);
}

#Defining a recursive function to calculate the factorial
define factorial(n) {
    if n > 0: 
	    return n * factorial(n - 1);
    else: return 1;
}

set start, stop; #Define some global variables

start = stopwatch();

set number: 25;

print("The corresponding fibonacci number is ", fibonacci(number));
print("The factorial is ", factorial(number));

stop = stopwatch();

print("Execution Time (s): ", stop - start);
```

A small Script using the OOP Paradigm to calculate the validity of a triangle.

```
#Define a new class
class Triangle {
    set A: 0, B: 0, C: 0; #Add some class members

    #Define a constructor to initialize instances of this class
    define Triangle(A, B, C) {
        this.A = A;
        this.B = B;
        this.C = C;
    }

    define check: this.A + this.B + this.C == 180;
}

set triangle: Triangle(30, 60, 90); #Create an instance of the class

if triangle.check():
    print("The Triangle is valid.");
else: print("The Triangle is not valid.");
```

## License

This project is released under the `MIT License`. You can find the original license source here: [https://opensource.org/licenses/MIT](https://opensource.org/licenses/MIT).

```
MIT License

Copyright (c) 2020 Davide Casale

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```