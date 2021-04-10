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

The interpreter depends on some libraries, extensions and tools, make sure you have installed them correctly before trying to build the project.

#### Libraries

* The **GNU**  `Multiple Precision Arithmetic` (GMP) Library: [https://gmplib.org/](https://gmplib.org/)

#### Extensions

* The `Statement Exprs` extension: [https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html#Statement-Exprs](https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html#Statement-Exprs)

* The `Nested Functions` extension: [https://gcc.gnu.org/onlinedocs/gcc/Nested-Functions.html#Nested-Functions](https://gcc.gnu.org/onlinedocs/gcc/Nested-Functions.html#Nested-Functions)

* The `Labels as Values` extension: [https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html#Labels-as-Values](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html#Labels-as-Values)

#### Tools
* The `gcc` (**GNU** Compiler Collection) program or any other C compiler supporting the required extensions: [https://gcc.gnu.org/](https://gcc.gnu.org/)

* The `CMake` tool, used to execute the CMakeLists.txt file and create the necessary to compile the project: [https://cmake.org/](https://cmake.org/)

* The `Make` tool, used to compile the interpreter if you have used the `Unix Makefile` generator with `CMake`. Otherwise, you have to use the right tool depending on which generator you have used: [https://www.gnu.org/software/make/](https://www.gnu.org/software/make/)

## Building the Docker image
Inside the `Repository`, there is everything necessary to create a `Docker` image that allows the interpreter to work without the user having to install all the necessary requirements and manually compile the source code of the project.

Once you have cloned this `Repository`, go inside it and build the image using the `Dockerfile` contained in the project. If you want you can give it a name:
```
docker build -t davi0k/elite .
```

Next, make sure the build was successful. If everything is correct, this command should show the image you just created:
```
docker images
```

Then, check that the image works correctly:
```
docker run -it --rm davi0k/elite -v
```
This command starts a new interactive `Container` and that will be deleted and cleaned after its execution. It should return the installed version of the interpreter.

You can also directly run the demos contained in the examples directory:
```
docker run -it --rm davi0k/elite examples/calculator.eli
docker run -it --rm davi0k/elite examples/complex.eli
docker run -it --rm davi0k/elite examples/triangle.eli
```

If you want, you can run any of your scripts through the image you just built. To do this, you need to mount the folder or the file you want to use in the container that you are going to start:
```
docker run -it --rm --mount src=$(pwd),target=/usr/src/elite/,type=bind davi0k/elite script.eli
```

These command will serve the container every file and folder contained in the current directory.

## Using the CLI
To run a script, use the following dedicated Command-Line Interface (**CLI**) syntax:
```
.\elite.exe [path] [-v] [-h]
```
If you want, you can use the `REPL` (Read Eval Print Loop) by running the **CLI** without any positional parameters.

## Example scripts
A simple Arithmetic Calculator made using some Control-Flow statements.

```
set result: 0; #Define a global variable

#Use an infinite loop to allow more than one operation
while true: {
    print("Input two Numbers:");

    #Get inputs and convert them using the specifics NativeFunction Functions
    set x: number(input()), y: number(input()); 

    print("Insert an operator (+, -, *, /, ^):");
    
    set operator: input();

    if operator == '+': result = x + y;
    if operator == '-': result = x - y;
    if operator == '*': result = x * y;
    if operator == '/': result = x / y;
    if operator == '^': result = x ^ y;

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

set number: 25;

start = stopwatch();

print("The corresponding fibonacci number is: ", fibonacci(number));
print("The corresponding factorial number is: ", factorial(number));

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

**You will find more Demos in the examples and benchmarks folders.**

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