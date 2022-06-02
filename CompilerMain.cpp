/* 
    Daniel Rizzo, Eric Truong, Linda Ng - Group 9
    CSCI 360
    Compiler Project
    Xiaojie Zhang
    March 23rd, 2021
    CompilerMain.cpp
    Main file for CSCI 360 Compiler Project
*/

#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <sstream>
#include <vector>
#include <math.h>
#include "function.h"
using namespace std;

void convert_variable(string equation, Function &f1) // Eric
{
  regex arrays("[a-z]+\\[+[0-9]+\\]");
  smatch submatch;
  regex array_index("[a-z]+\\[+[a-zA-Z_-]");
  string variable = equation.substr(0, equation.find("="));
  string value = equation.substr(equation.find("=") + 1, equation.length() - 1);
  if (variable.find("[") != string::npos && value.find("{") == string::npos) //if an index of an array is being assigned a new value
  { // checks if line contains an index
    size_t start_posl = variable.find('[');
    size_t end_posl = variable.find(']');
    string left_var = variable.substr(0, start_posl);
    string array_il = variable.substr(start_posl + 1, end_posl - start_posl - 1);     
    size_t start_posr = value.find('[');
    size_t end_posr = value.find(']');
    string right_var = value.substr(0, start_posr);
    string array_ir = value.substr(start_posr + 1, end_posr - start_posr - 1);   
    if (regex_search(variable, submatch, array_index)) { // checks if left side contains an index
      f1.assembly_instructions_.push_back("movl    " + f1.return_location(array_il) + ", %eax");
      f1.assembly_instructions_.push_back("cltq");
      string temp = left_var + "[0]";
      f1.assembly_instructions_.push_back("movl    " + f1.return_location(temp).substr(0, f1.return_location(temp).length() - 1) + ",%rax,4), %edx"  );
    }
    if (regex_search(value, submatch, array_index)) { // checks if right side contains an index
      f1.assembly_instructions_.push_back("movl    " + f1.return_location(array_ir) + ", %eax");
      f1.assembly_instructions_.push_back("cltq");
      string temp = right_var + "[0]";
      f1.assembly_instructions_.push_back("movl    " + f1.return_location(temp).substr(0, f1.return_location(temp).length() - 1) + ",%rax,4), %eax"  );
    }
  }
  else if (regex_search(equation, submatch, arrays)) //if an array is being created - local
  { //checks if line contains an array, in order to find array size
    f1.has_locals_ = true; // Function has local variables
    size_t start_pos = equation.find('[');
    size_t end_pos = equation.find(']');
    string array_size = equation.substr(start_pos + 1, end_pos - start_pos - 1);
    string array_items = value.substr(1,value.length()-3); //needed to parse array items
    string item; //temp storage for array items
    for (int i = stoi(array_size) - 1; i >= 0; i--) // From last index to first index
    {
      string index = variable.substr(0, variable.find("[") + 1) + to_string(i) + "]"; //needed to store array item if needed later 
      if(array_items.find(",") != array_items.npos) 
      {
        item = array_items.substr(array_items.find_last_of(",")+1, array_items.npos); // Get last element
        array_items.assign(array_items, 0, array_items.find_last_of(",")); // Take it off of the list
      }
      else 
      {
        item = array_items; // First element is only one left.
      }
      f1.track_variable(index); //stores location of item in the index and location in machine
      f1.assembly_instructions_.push_back("movl    $" + item + ", " + f1.return_offset() +"(%rbp)");
    }
    f1.track_stack_variable(variable.substr(0, variable.find('[')), f1.offset); // Set array's address using array[0]'s address.
  }
  else if (value.find("+") != string::npos) //addition assignment
  {
    if (regex_search(value, submatch, array_index)){
    string an_array_element = value.substr(value.find("+") +1, value.length());
    string index = an_array_element.substr(an_array_element.find('[') + 1, an_array_element.find(']') - an_array_element.find('[') - 1);
     if (f1.return_location(index) != "") // If index is a variable instead an immediate
        {
            string array_name = an_array_element.substr(0, an_array_element.find('['));
            f1.assembly_instructions_.push_back("movl    " + f1.return_location(index) + ", %eax"); // Move value of index into eax
            f1.assembly_instructions_.push_back("cltq"); // Copy eax into 64 bit register rax
            f1.assembly_instructions_.push_back("leaq    0(,%rax,4), %rdx"); // Move from array into rdx
            f1.assembly_instructions_.push_back("movq    " + f1.return_location(array_name) + ", %rax"); // array into rax
            f1.assembly_instructions_.push_back("addq    %rdx, %rax");
            f1.assembly_instructions_.push_back("movl    (%rax), %edx");
            f1.assembly_instructions_.push_back("movl    " + f1.return_location(value.substr(0, value.find("+"))) + ", %eax");
            f1.assembly_instructions_.push_back("addl    %edx, %eax");
            f1.assembly_instructions_.push_back("movl    %eax, " + f1.return_location(variable));
        }
    }
    else{
      f1.assembly_instructions_.push_back("movl    " + f1.return_location(value.substr(0, value.find("+"))) + ", %eax");
    if (value.substr(value.find("+") + 1, value.length() - 3).find_first_not_of("0123456789") != string::npos){
      f1.assembly_instructions_.push_back("addl    " + f1.return_location(value.substr(value.find("+") + 1, value.length() - 3)) + ", %eax");}
    else {f1.assembly_instructions_.push_back("addl    $" + value.substr(value.find("+") + 1, value.length() - 3) + ", %eax");}
    if (f1.return_location(variable) != ""){
    f1.assembly_instructions_.push_back("movl    %eax, " + f1.return_location(variable));}
    else {
    f1.has_locals_ = true; // Function has local variables
    f1.track_variable(variable); //used to store variable if needed again
    f1.assembly_instructions_.push_back("movl    %eax, " + f1.return_location(variable));}
    }
  }
  else if (value.find("-") != string::npos) //subtraction assignment
  {
    if (regex_search(value, submatch, array_index)){
     string an_array_element = value.substr(value.find("-") +1, value.length());
    string index = an_array_element.substr(an_array_element.find('[') + 1, an_array_element.find(']') - an_array_element.find('[') - 1);
     if (f1.return_location(index) != "") // If index is a variable instead an immediate
        {
            string array_name = an_array_element.substr(0, an_array_element.find('['));
            f1.assembly_instructions_.push_back("movl    " + f1.return_location(index) + ", %eax"); // Move value of index into eax
            f1.assembly_instructions_.push_back("cltq"); // Copy eax into 64 bit register rax
            f1.assembly_instructions_.push_back("leaq    0(,%rax,4), %rdx"); // Move from array into rdx
            f1.assembly_instructions_.push_back("movq    " + f1.return_location(array_name) + ", %rax"); // array into rax
            f1.assembly_instructions_.push_back("addq    %rdx, %rax");
            f1.assembly_instructions_.push_back("movl    (%rax), %edx");
            f1.assembly_instructions_.push_back("movl    " + f1.return_location(value.substr(0, value.find("-"))) + ", %eax");
            f1.assembly_instructions_.push_back("subl    %edx, %eax");
            f1.assembly_instructions_.push_back("movl    %eax, " + f1.return_location(variable));
        }
    }
    else{
    f1.assembly_instructions_.push_back("movl    " + f1.return_location(value.substr(0, value.find("-"))) + ", %eax");
    f1.assembly_instructions_.push_back("subl    " + f1.return_location(value.substr(value.find("-") + 1, value.length() - 3)) + ", %eax");
    f1.assembly_instructions_.push_back("movl    %eax, " + f1.return_location(variable));
  }
  }
  else if (value.find("*") != string::npos) //multiplication assignment
  {
    f1.assembly_instructions_.push_back("movl    " + f1.return_location(value.substr(0, value.find("*"))) + ", %eax");
    f1.assembly_instructions_.push_back("imull   " + f1.return_location(value.substr(value.find("*") + 1, value.length() - 3)) + ", %eax");
    f1.assembly_instructions_.push_back("movl    %eax, " + f1.return_location(variable));
  }
  else if (f1.return_location(variable) != "" && (value.substr(0, value.length()-1).find_first_not_of("0123456789") != string::npos)){ // variable to variable assignment
    string not_number_value = value.substr(0, value.length()-1);
    f1.assembly_instructions_.push_back("movl    " + f1.return_location(not_number_value) + ", %eax");
    f1.assembly_instructions_.push_back("movl    %eax, " + f1.return_location(variable));
  }
  // Linda
  else if (f1.return_location(variable) == "" && regex_search(value, submatch, array_index)){ // new variable to index assignment
    f1.PushArrayElementAddress(value); 
    f1.has_locals_ = true; // Function has local variables
    f1.track_variable(variable); //used to store variable if needed again
    f1.assembly_instructions_.push_back("movl    %eax, " + f1.return_location(variable));
  }
  else //regular variable assignment
  {
    f1.has_locals_ = true; // Function has local variables
    f1.track_variable(variable); //used to store variable if needed again
    f1.assembly_instructions_.push_back("movl    $" + value.substr(0,value.length()-1) + ", " + f1.return_offset() + "(%rbp)");
  }
}
void for_statement_called(Function &f1) //Added by Eric (When For statements are called)
{
  f1.conditional_counter++;
  f1.assembly_instructions_.push_back("jmp     Outside_Conditional_Statement" + to_string(f1.conditional_counter));
  f1.assembly_instructions_.push_back("Conditional_Statement" + to_string(f1.conditional_counter) + ":");
}
void outside_conditional_statement(Function &f1) //Added by Eric (When if/for statments are finished)
{
    string temp = f1.conditional_compare();
    if (temp != "")
    {
      string temp2 = f1.pop_incrementor();
      f1.assembly_instructions_.push_back("addl    $1, " + temp2);
      f1.assembly_instructions_.push_back("Outside_Conditional_Statement" + to_string(f1.conditional_counter) + ":");
      f1.assembly_instructions_.push_back(temp);
      f1.assembly_instructions_.push_back("jle     Conditional_Statement" + to_string(f1.conditional_counter));
    }
    else
    {
      f1.assembly_instructions_.push_back("Outside_Conditional_Statement" + to_string(f1.conditional_counter) + ":");
    }
    f1.conditional_counter--; //decreases counter since conditional statement is done
}

/**
 * Parses through the inputted file to set up a function and its parameters, then adds code to the function's
 * assembly instructions array. Meant to be used as a recursive function, so that if the parser finishes a function's
 * code and obtains a new line that starts a new function, Parser() will be called again.
 * @param input is the fstream to be reading from, passed by reference so it keeps track of what line it's at. 
 * @param full_line is the line currently at in the input file.
 * @param Fn is the vector of Functions that keeps track of the order of functions, and separates them. It uses pointers
 * so that the elements in the vector (when dereferenced) give us the actual function object that was worked on.
 * @param f1 is the current function the parser is working on. For example a recursive call to Parser would be
 * Parser(input, full_line, Fn, f2), where f2 is the new function obtained from the parser.
 */
void Parser(fstream &input, string &full_line,
            vector<Function *> &Fn)
{
  // Set up function object
  Function *f1 = new Function;
  string return_type = full_line.substr(full_line.find("int"), full_line.find(" "));
  string function_name = full_line.substr((full_line.find(" ") + 1),
                                          ((full_line.find("(")) - (full_line.find(" ") + 1)));
  f1->function_name_ = function_name; // Set data members of f1
  f1->return_type_ = return_type;
  Fn.push_back(f1);
  full_line.assign(full_line, full_line.find("(") + 1, full_line.npos); // Removes the used tokens from full_line.

  // Search for all variables
  smatch match;
  regex variable("int|=");
  regex if_statement("if");
  regex for_statement("for");
  regex returns("return");
  regex equals("=");
  regex array_index("[a-z]+\\[+[a-zA-Z_-]");

  // Dan - Search for function parameters and allocate and track. All within the current full_line. 
  // Make a data member in the function header to keep track of what variable names are, and their addresses 
  // relative to rbp. 
  int parameter = 0;
  int stack_offset = 16; 
  while (full_line.find("int ") != full_line.npos) // while there are parameters still in the line
  { // p must be < 6, as only 6 arguments are put in registers, the extras were pushed onto the stack (Lec 3)
    full_line.assign(full_line, full_line.find("int ") + 4, full_line.npos); // Removes the "int " token from full_line.
    string name = full_line.substr(0, full_line.find_first_of(",)")); // Parse for the name, which ends with either , )
    string reg;
    bool is_array = (name.find('[') != name.npos); // The parameter is an array, or not
    if (parameter == 0) // move from %edi or %rdi - all for 32 bits for int, 64 bit for arrays - which are passed as addresses
    { 
      if (is_array) // Array - use 64 bit register
      {
        reg = "%rdi, ";
      }
      else // Only an int - use 32 bit register
      {
        reg = "%edi, ";
      }
    }
    else if (parameter == 1) // %esi or %rsi
    { 
      if (is_array) // Array - use 64 bit register
      {
        reg = "%rsi, ";
      }
      else // Only an int - use 32 bit register
      {
        reg = "%esi, ";
      }
    }
    else if (parameter == 2) // %edx or %rdx
    { 
      if (is_array) // Array - use 64 bit register
      {
        reg = "%rdx, ";
      }
      else // Only an int - use 32 bit register
      {
        reg = "%edx, ";
      }
    }
    else if (parameter == 3) // %ecx or %rcx
    { 
      if (is_array) // Array - use 64 bit register
      {
        reg = "%rcx, ";
      }
      else // Only an int - use 32 bit register
      {
        reg = "%ecx, ";
      }
    }
    else if (parameter == 4) // %r8d or %r8
    { 
     if (is_array) // Array - use 64 bit register
      {
        reg = "%r8, ";
      }
      else // Only an int - use 32 bit register
      {
        reg = "%r8d, ";
      }
    }
    else if (parameter == 5) // %r9d or %r9
    { 
      if (is_array) // Array - use 64 bit register
      {
        reg = "%r9, ";
      }
      else // Only an int - use 32 bit register
      {
        reg = "%r9d, ";
      }
    }
    else // If there are seven or more parameters, this route is used, both arrays and ints use stack offset of 8. 
    { string array_name = name.substr(0, name.find("["));
      f1->track_stack_variable(name, stack_offset); // Create a pair in f1 for the stack parameter
            if (array_name != ""){
        f1->track_stack_variable(array_name, stack_offset);
      }
      stack_offset += 8; // Increment stack's offset for where the next parameter on the stack will be
    }
    parameter++; // Increment the number of parameters in this function
    if (parameter <= 6) // If there are still <= 6 parameters in the function, move the parameter from the register.
    {
      if (is_array) // Use rbp offset of 8 if array address
      {
        string array_name = name.substr(0, name.find("["));
        f1->track_array_parameter(array_name); // Create variable within f1 
        f1->assembly_instructions_.push_back("movq    " + reg + f1->return_location(array_name)); // Use movq for 64 bit
      }
      else // Use regular rbp offset
      {
        f1->track_variable(name); // Create a pair for the parameter / variable within f1. 
        f1->assembly_instructions_.push_back("movl    " + reg + f1->return_location(name));
      }
    }
  }

  // Parse each line
  bool return_statement = false; // Keep track of if there's a return statement already in the function. 
  while (getline(input, full_line))
  {
    
    // Search for a new function in the line
    if (full_line.find("int ")!= full_line.npos && full_line.find("()") != full_line.npos) // If this line is a new function 
    {
      Parser(input, full_line, Fn); // Recursively call parser to deal with the new function. 
    }

    // Search for function call and search for = to assign return value to
    bool cont = false; // Bool to check if it should skip the rest of the loop
    for (size_t i= 0; i < Fn.size(); i++) { // Search each element in the function vector
      regex call(Fn[i]->function_name_); // Create a regex object from each function's name
      if (regex_search(full_line,match,call)) // if there's a match
      {
        f1->leaf_ = false; // It's not a leaf function, as it calls another function
        string assign_to = "";
        if (regex_search(full_line,match,equals))  // Search for = assignment 
        {
          // Get variable to assign the return value to
          assign_to = full_line.substr(0, full_line.find("=")); 
        }
        // Dan - Throw all parameters into registers
        int open = full_line.find("("); // Find parentheses that start and end the list of parameters
        int close = full_line.find(")");  
        string parameter_list = full_line.substr(open+1, close-open-1); // Get list of parameters into one string

        // Count the number of parameters used in the function being called.
        int calling_parameter = 1; // Assume there's one parameter
        for (int i = 0; i < parameter_list.length(); i++) 
        {
          if (parameter_list[i] == ',') // If there is a comma in the parameter list
          {
            calling_parameter++; // That means there's another parameter
          }
        }
        int total  = calling_parameter;
        // Parse parameter list
        while (parameter_list != "") // While there are still parameters in the list, if any 
        {
          // Get the LAST parameter in the list
          string a_param = "";
          if (parameter_list.find_last_of(",") != parameter_list.npos) // If it's not the first parameter in the list
          { // Can look for a comma to separate the parameters
            a_param = parameter_list.substr(parameter_list.find_last_of(",")+1, parameter_list.npos);
          }
          else // It is the first and only parameter left in the list
          {
            a_param = parameter_list; // Copy last parameter into a_param
            parameter_list = ""; // Set parameter list to be blank so while loop terminates
          } 

          // Determine if the parameter is a variable or an immediate OR AN ARRAY so it can pass the address using leaq
          string value = "", type;
          if (f1->return_location(a_param) != "") // If it's a known variable
          {
            value = f1->return_location(a_param);
          }
          else // Otherwise it must be an immediate
          {
            value = "$" + a_param;
          }
          // Insert into the proper registers
          string reg = "";
          calling_parameter--; // Decrement the amount of parameters already dealt with
          if (calling_parameter == 0) // move to %edi or %rdi
          {
            if (f1->return_type(a_param) == Function::Type::Integer) // If parameter's an integer
            {
              reg = ", %edi";
            }
            else // If parameter's an array
            {
              reg = ", %rdi";
            }
          }
          else if (calling_parameter == 1) // %esi or %rsi
          {
            if (f1->return_type(a_param) == Function::Type::Integer)
            {
              reg = ", %esi";
            }
            else 
            {
              reg = ", %rsi";
            }
          }
          else if (calling_parameter == 2) // %edx or %rdx
          {
            if (f1->return_type(a_param) == Function::Type::Integer)
            {
              reg = ", %edx";
            }
            else 
            {
              reg = ", %rdx";
            }
          }
          else if (calling_parameter == 3) // %ecx or %rcx
          {
            if (f1->return_type(a_param) == Function::Type::Integer)
            {
              reg = ", %ecx";
            }
            else 
            {
              reg = ", %rcx";
            }
          }
          else if (calling_parameter == 4) // %r8d or %r8
          {
            if (f1->return_type(a_param) == Function::Type::Integer)
            {
              reg = ", %r8d";
            }
            else 
            {
              reg = ", %r8";
            }
          }
          else if (calling_parameter == 5) // %r9d or %r9
          {
            if (f1->return_type(a_param) == Function::Type::Integer)
            {
              reg = ", %r9d";
            }
            else 
            {
              reg = ", %r9";
            }
          }
          if (calling_parameter < 6) 
          {
            if (f1->return_type(a_param) == Function::Type::Integer) // If parameter is an integer
            {
              f1->assembly_instructions_.push_back("movl    " + value + reg); // Put in a 32 bit register
            }
            else  // If parameter is an array
            {
              f1->assembly_instructions_.push_back("leaq    " + value + reg); // Load address into 64 bit register
            }
          }
          // For when there are more than 6 parameters, the extras will go on the stack
          else 
          {
            if (f1->return_type(a_param) == Function::Type::Integer) // If parameter is an integer
            {
              f1->assembly_instructions_.push_back("movl    " + value + ", %eax"); // Put in a 32 bit register
              f1->assembly_instructions_.push_back("cltq"); // Change eax into rax so it's 64 bits. 
            }
            else  // If parameter is an array
            {
              f1->assembly_instructions_.push_back("leaq    " + value + ", %rax"); // Load address into 64 bit register
            }
            f1->assembly_instructions_.push_back("pushq   %rax"); // Push 8 bytes onto stack
            f1->assembly_instructions_.push_back("subq    $8, %rsp");
          }
          // For when it is still dealing with <= 6 parameters
          // Take the current parameter off of the parameter list. 
          parameter_list.assign(parameter_list, 0, parameter_list.find_last_of(","));
        } // end while

        // Finally call the actual function in assembly code. 
        f1->assembly_instructions_.push_back("call    " + Fn[i]->function_name_);
        f1->assembly_instructions_.push_back("addq    $16, %rsp"); // Reclaim stack space

        // Assign return value to variable if applicable
        if (assign_to.find("int ") != full_line.npos) // If the variable to assign to is being declared
        {
          assign_to.assign(assign_to, assign_to.find("int ")+4, assign_to.npos); // Make assign_to into JUST the variable's name
          f1->track_variable(assign_to); // Track new variable
        }
        if (assign_to != "") 
        {
          f1->assembly_instructions_.push_back("movl    %eax, " + f1->return_location(assign_to));
        }
        cont = true;
        break;
      }
    } // end for 
    if (cont) {
      continue; // Skip the rest of this exact loop in the while loop
    }
    
    // For - Linda
    if (regex_search(full_line, match, for_statement)) //checks if line contains for statement
    {
      vector<string> for_loop;
      string loop_argument = full_line.substr(full_line.find("(") + 1, full_line.find(")") - full_line.find("(") - 1);
      stringstream s_stream(loop_argument); //create string stream from the string
      while (s_stream.good())
      {
        string substr;
        getline(s_stream, substr, ';'); //get first string delimited by semicolon
        for_loop.push_back(substr);
      }
      string for_loop_equation = for_loop.at(0).substr(for_loop.at(0).find(" ") +1, for_loop.at(0).length()) + ";";
      convert_variable(for_loop_equation, *f1);
      for_statement_called(*f1);
      string for_loop_left_compare = for_loop.at(1).substr(0, for_loop.at(1).find("<"));
      string for_loop_right_compare = for_loop.at(1).substr(for_loop.at(1).find("<") + 1, for_loop.at(1).length());
      int right_int = atoi(for_loop_right_compare.c_str());
      f1->Store_Conditional("For","cmpl    $" + to_string((right_int - 1)) + ", " + f1->return_location(for_loop_left_compare)); //stores compare function
      f1->increment_tracker.push_back(f1->return_location(for_loop_left_compare));
    }

    // If - Linda
    else if (regex_search(full_line, match, if_statement)) //checks if line contains if statement
    {
      f1->conditional_counter++; //added by Eric
      char comparison = '<';
      string left_compare = full_line.substr(full_line.find("(") + 1, full_line.find(comparison) - full_line.find("(") - 1); // string on left side of comparison
      string right_compare = full_line.substr(full_line.find(comparison) + 1, full_line.find(")") - full_line.find(comparison) - 1); // string on right side of comparison
      size_t start_posl = left_compare.find('[');
      size_t end_posl = left_compare.find(']');
      string left_var = left_compare.substr(0, start_posl);
      string array_il = left_compare.substr(start_posl + 1, end_posl - start_posl - 1);     
      size_t start_posr = right_compare.find('[');
      size_t end_posr = right_compare.find(']');
      string right_var = right_compare.substr(0, start_posr);
      string array_ir = right_compare.substr(start_posr + 1, end_posr - start_posr - 1);   
      if (regex_search(left_compare, match, array_index)) { // checks if left side contains an index 
        f1->assembly_instructions_.push_back("movl    " + f1->return_location(array_il) + ", %eax");
        f1->assembly_instructions_.push_back("cltq");
        string temp = left_var + "[0]";
        f1->assembly_instructions_.push_back("movl    " + f1->return_location(temp).substr(0, f1->return_location(temp).length() - 1) + ",%rax,-4), %edx"  );
      }
        if (regex_search(right_compare, match, array_index)) { // checks if right side contains an index
        f1->assembly_instructions_.push_back("movl    " + f1->return_location(array_ir) + ", %eax");
        f1->assembly_instructions_.push_back("cltq");
        string temp = right_var + "[0]";
        f1->assembly_instructions_.push_back("movl    " + f1->return_location(temp).substr(0, f1->return_location(temp).length() - 1) + ",%rax,-4), %eax"  );
        }
        f1->assembly_instructions_.push_back("cmpl    %eax, %edx");
        f1->assembly_instructions_.push_back("jge     Outside_Condtional_Statement" + to_string(f1->conditional_counter)); //added by Eric 
        f1->Store_Conditional("If","");
    }

    // Line starts with already existing variable
    else if (regex_search(full_line, match, variable)) //checks if line contains variable assignment and/or arithmetic
    {
      stringstream reader(full_line);
      string storage;
      while (reader >> storage)
      {
        if (storage == "int")
        {
        }
        else
        {
          convert_variable(storage,*f1);
        }
      }
    }

    // Return Statement - Linda
    else if (regex_search(full_line, match, returns)) //checks if line contains return statement
    {
      return_statement = true;
      string return_v = full_line.substr(full_line.find("return ")+7, full_line.find(";")-full_line.find("return ")-7);
      f1->assembly_instructions_.push_back("movl    " + f1->return_location(return_v) + ", %eax");
    }
    else if (full_line.find("}") != string::npos && f1->conditional_counter != 0) //added by Eric (If/For a conditional statement is ended)
    {
        outside_conditional_statement(*f1);
    }
  } // end while

  if (!return_statement) // If there's no return statement, add return 0
  {
    f1->assembly_instructions_.push_back("movl    $0, %eax");
  }
  if (f1->function_name_ == "main") { // Only close input file if this is the last function - which is always main.
    input.close();
  }
}

int main(int argc, char **argv)
{
  // Get input file name
  if (argc != 2)
  {
    cout << "Usage: " << argv[0] << " <inputfilefilename>" << endl;
    return 0;
  }
  const string input_filename(argv[1]);

  // Set up Parser
  vector<Function *> Fn;
  fstream input;
  string full_line;
  input.open(input_filename, fstream::in);
  getline(input, full_line);
  Parser(input, full_line, Fn);

  // Print out each Function's assembly code
  for (size_t i = 0; i < Fn.size(); i++)
  {
    cout << (*Fn[i]);
    delete Fn[i];
    Fn[i] = nullptr;
  }
  return 0;
}