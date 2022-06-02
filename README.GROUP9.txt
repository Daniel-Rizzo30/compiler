Daniel Rizzo, Eric Truong, Linda Ng - Group 9
CSCI 360
Compiler Project
Xiaojie Zhang
March 23rd, 2021
Project Desciption text file - README.GROUP9.txt

The files included in the project submission are as follows:
CompilerMain.cpp - contains the main code that compiles the test cases
function.h - contains the class Function, which contains data for each function created by the compiler
compiler.out - the executable file that is made from compiling through "g++ -o compiler.out CompilerMain.cpp"
README.GROUP9.txt - this file, for description of the project

Test case format: 
The format of the test cases we used requires every curly bracket to have it own line. We also require each variable 
declaration to have its own line. For example, int a=1, b=2, c=3; would have to be int a=1; int b=2; int c=3;. Finally, 
there must be not spaces between variables, the equal sign and the variable assignment just like the previous example. 
To run the code, it must be ran at the command line using ./compiler.out testcase#.cpp. The assembly code will then 
output within the terminal.

To run each test file from the .out file type the command: 
./compiler.out testcase1.cpp OR 
./compiler.out testcase2.cpp

Example Format (using testcase2): 

int main()
{
    int a[5]={10,74,54,46,7};
    int min_inx=0;
    for(int i=0;i<4;i=i+1)
    {
        min_inx=i;
        for(int j=i+1;j<5;j=j+1)
        {
            if(a[j]<a[min_inx])
            {
                min_inx=j;
            }
        }
        int temp=a[min_inx];
        a[min_inx]=a[i];
        a[i]=temp;
    }
}

Each group member wrote a paragraph describing what they did for the project: 

Daniel: 
The first night the team and I met, I created the function class header and provided basic implementation of the class,
such as the assembly_instructions vector<string>, the Prologue/Epilogue members, and the operator<< overload. I created 
the main() function in the CompilerMain.cpp. I set up the Parser() function within this file as well, up until the main 
while(getline()) loop starts (except for the regex declarations). I parsed the first line in the function being used in 
the Parser for its parameters. This meant distinguishing between when a 64 or 32 bit register is used. 
I made the PushArrayElementAddress() function within the function class as well.
I also wrote the code for checking if a function is called within a line. This meant moving the variables into their 
proper registers / on the stack in backwards order. In order to do this I made the Type Enum in the function class and 
reworked the way variables were stored. Previously they were stored in a vector of pairs, but I changed it so that they're 
stored in a vector of 3-tuples, which include this Type Enum. This helped immensely with storing variables into registers.
The recursive call to Parser upon a the line starting a new function was also written by me, it went through a few 
iterations, but the simple if statement checking for "int" and "()" in the line is what worked the best. 
I also made sure that if a function does not include a return statement, then our Parser would be able to add a "return 0" 
assembly instruction to the end of the assembly_instructions_ vector.

Eric:
I wrote the convert_variable function which translates arithmetic, array creation, array element assignment, and variable 
assignment into assembly instruction. I also wrote code to track where each variable is stored, the increment type of 
each for loop and code to add labels used by if and for statements. 

Linda: 
I worked on making arrays which specify an index location work correctly in arithmetic, from assigning a value to 
adding and subtracting. This was then used by me to convert if statements and for loops.


MAIN():
This function copies the testcase file name from its parameters and sets up the Parser function by creating a vector of 
Function pointers, an fstream to read from the testcase, and a string to go line by line. It then passes these to the 
Parser function. 
Upon the Parser function's return, the main function goes through each element of the vector<Function*> and outputs the 
assembly instructions of each using the << operator, then deletes these functions.

FUNCTION.H:
This contains the function class, along with any c++ function implementation needed for manipulating these functions. 
The most fundamental part of the function class is the operator<< overload, which makes it so that a function put through 
<< will output its assembly instructions, and updates the prologue and epilogue so that they are correct. 
The class also contains the assembly_instructions_ vector to store all instructions for the output, and also a vector of 
3-tuples to store information about variables, whether local or not. 
PushArrayElementAddress() is only called once within the main file, so it doesn't do much, but it was made to try to 
simplify the amount of code space that was for trying to access array variable elements.

PARSER():
This function is called for each function within the testcase. It is therefore recursive, creating a new function whenever
it comes upon a line that contains a new function. It creates a dynamic Function object for the function in the testcase, 
and Parses the first line of this function for parameters. It takes parameters from the registers or the stack above the 
function, and puts them into the function's stack, or just tracks the location of them if they were already put onto the 
stack as extra parameters. 
Then it parses each line in the testcase for: 
A call to another function, 
A For Loop, 
An If statement, 
Math with an already existing variable, or
A return statement - if there is no return statement it will add instructions for return 0 at the end of the function.

A call to another function: 
The for loop searches for the name of another function within the Fn vector<Function*>. If it is found, the parameter list 
in the call is parsed, and the parameters are put into the correct 64/32 bit registers, or on the stack, in reverse order. 
After doing that it calls the function, then puts its return value into a variable only if it is called for. Then it 
breaks the for loop, and forces the while loop into skipping over the rest of the lines in this exact loop.

For Loop:
For statements are parsed by reading the first 2 conditions of the for statement to create the increment variable 
and its limit.  The creation of the increment variable is pushed and the comparison for the statement is stored for 
later.  When the for statement is finished the increment (i++) is pushed and then the label is created, followed by 
the stored comparison and then a jump to the original function.

If Statement:
If statements are parsed by reading the inside of the if statement and checking if the variables inside are apart of 
an array and if it is we move the array to a register. After that, the jump to the outside label is called and then 
the things inside the if statement are ran. If a bracket is called, then the if statement is finished and the label 
for the outside of the if function is created.

Return Statement: 
If the return keyword is found through the regex search, the variable is taken from the line, and the location of it is 
found through the vector of variable information, and that is put into %eax. 
If no return statement is ever found, then outside of the while loop the instructions for return 0; are added to the end 
of the function's assembly_instructions_.

Math: 
convert_variable() is called upon there being math with an already existing variable.

CONVERT_VARIABLE():
convert_variable() works by first taking out the int statement if there is any, which leaves the variable=value part 
to be sent to convert_variable. From there, the variable and the value are separated as strings and the function checks 
for a number of conditions including if the value contains arithmetic, if the value is an array or if the variable is 
part of an array and the value is an immediate. These conditions contained in the if statements will do the necessary 
translation from c++ to assembly instruction. 