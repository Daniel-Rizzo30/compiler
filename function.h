/*  
    Daniel Rizzo, Eric Truong, Linda Ng - Group 9
    CSCI 360
    Compiler Project
    Xiaojie Zhang
    March 23rd, 2021
    function.h
    Header file for the function class
*/

#ifndef FUNCTION_H
#define FUNCTION_H

#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <utility>
using namespace std;

class Function {
    public: 

    // Constructors
    Function(string function_name, string return_type, bool is_leaf = true,int return_value = 0 ) :
            function_name_{function_name}, return_type_{return_type}, leaf_{is_leaf}, return_value_{return_value} {
    }
    Function() = default;

    // Accessor Functions
    string getFunctionName() {
        return function_name_;
    }
    string getReturnType() {
        return return_type_;
    }
    bool getLeaf() {
        return leaf_;
    }
    int getReturnValue() {
        return return_value_;
    }
    enum Type {Integer, LocalArray, ParameterArray, NotFound}; // Enum for specifying what kind of data type a variable is.
    
    /**
     * Pushes Array Element's Address for any manipulation
     * @brief takes a string like e[i] or a[2] and outputs movement of address into eax, only for eax...
     * @param an_array_element is a string like e[i] or a[2].
     * Maybe add a register string parameter to get what reg to put it in?
     */
    void PushArrayElementAddress(string an_array_element) {
        string array_name = an_array_element.substr(0, an_array_element.find('['));
        string index = an_array_element.substr(an_array_element.find('[') + 1, an_array_element.find(']') - an_array_element.find('[') - 1);
        if (return_location(index) != "") // If index is a variable instead an immediate
        {
            assembly_instructions_.push_back("movl    " + return_location(index) + ", %eax"); // Move value of index into eax
            assembly_instructions_.push_back("cltq"); // Copy eax into 64 bit register rax
            string open_parentheses = return_location(array_name); // Copy base address of array into another string
            open_parentheses.pop_back(); // Take last character ')' off of the string
            assembly_instructions_.push_back("movl    " + open_parentheses + ",%rax,4), %eax"); // Move from array into eax
        }
        else // Index is an immediate
        {
            string array_offset = to_string(4 * stoi(index)); // 4 * the index above base address is the address of the element
            // to_string() and stoi() convert int to string and string to int respectively
            assembly_instructions_.push_back("movq    " + return_location(array_name) + ", %rax"); // movq x(%rbp), %rax
            assembly_instructions_.push_back("movl    " + array_offset + "(%rax), %eax"); // Copy from array into eax
        }
    }
 
    /**
     * Epilogue Setter
     * @post the epilogue portion of code is correctly set depending on if the function is a leaf, 
     * and based on how much red zone space it has used
     */
    void SetEpilogue() {
        /*
            "You modify the top of the stack when a function's 128 byte Red Zone is full, or when fitting 
            a leaf function inside of the caller's Red Zone when the caller has local variables.""
        */
        if (!leaf_ && has_locals_) {  // Also depends on how many variables there are, but I'm not sure how to check that yet
            Epilogue[0] = "leave"; // rsp != rbp
        }
        else {
            Epilogue[0] = "popq    %rbp"; // rbp = rsp
        }
    }

    /** 
    * Operator<< Overload
    * @param some_function: is the object to be displayed.
    * @returns the output of all the function's assembly instructions
    */
    friend ostream& operator<<(ostream &out, Function &some_function) {
        some_function.SetEpilogue();
        out << some_function.function_name_ << ": " << endl;
        out << some_function.Prologue[0] << endl << some_function.Prologue[1] << endl;
        if (!some_function.leaf_ && some_function.has_locals_) { // If it's not a leaf function and has local variables, 
            out << "subq    $48, %rsp" << endl; // Change the stack pointer to add space. 
        }
        for (size_t i = 0; i < some_function.assembly_instructions_.size(); i++) {
            out << some_function.assembly_instructions_[i] << endl;
        }
        out << some_function.Epilogue[0] << endl << some_function.Epilogue[1] << endl << endl;
    }

    // Variable functionalities

    // Stores variables and offset from rbp (eric) - Set Enum to Integer
    void track_variable(string name) 
    {
        offset = offset - 4;
        tuple<string, int, Type> store(name, offset, Integer);
        variable_storage.push_back(store);
    }
    // Stores the base address of an array parameter - Set Enum to ParameterArray
    void track_array_parameter(string name) 
    {
        string true_name = name.substr(0, name.find("["));
        offset = offset - 8;
        tuple<string, int, Type> store(name, offset, ParameterArray);
        variable_storage.push_back(store);
    }
    // Creates a pair for a parameter that was put on the stack, or for the base address of a Local Array
    void track_stack_variable(string name, int stack_offset)
    {
        bool duplicate = false;
        for(size_t i = 0; i < variable_storage.size(); i++) // Search each already existing variable
        {
            if (get<1>(variable_storage[i]) == stack_offset) // If there already exists a variable with that offset
            {
                duplicate = true; // This new variable must be a local array base pointer
                break;
            }
        }
        tuple<string, int, Type> store;
        if (duplicate)
        { // If there's another variable with the same offset, for example e and e[0] would have the same offset.
            store = make_tuple(name, stack_offset, LocalArray);
        }
        else 
        { // Otherwise it's just an integer on the stack with a positive offset
            store = make_tuple(name, stack_offset, Integer);
        }
        //pair<string,int> store = make_pair(name, stack_offset);
        variable_storage.push_back(store);
    }
    // Returns offset in the form of a string (eric)
    string return_offset() 
    {
        return to_string(offset);
    }
    // Returns location of variable as a string (eric)
    string return_location(string name) 
    {
        for (int i = 0; i < variable_storage.size(); i++)
        {
            if(get<0>(variable_storage[i]) == name)
            {
                return to_string(get<1>(variable_storage[i])) + "(%rpb)"; 
            }
        }
        return ""; //happens if values doesnt exist in the machine
    }
    // Get the type of variable based on the name of the variable
    Type return_type(string name) 
    {
        for (int i = 0; i < variable_storage.size(); i++)
        {
            if(get<0>(variable_storage[i]) == name)
            {
                return get<2>(variable_storage[i]); // Return the Type of this variable.
            }
        }
        return NotFound; // If not found
    }
    void Store_Conditional(string type, string increment) //added by Eric
    {
        pair<string,string> store = make_pair(type, increment);
        conditional_tracker.push_back(store);
    }
    string conditional_compare()
    {
        string comp = conditional_tracker[conditional_tracker.size() - 1].second;
        conditional_tracker.pop_back();
        return comp;
    }
    string pop_incrementor()
    {
        string temp = increment_tracker[increment_tracker.size() - 1];
        increment_tracker.pop_back();
        return temp;
    }

    // Public Data Members, will need to change these sometimes 
    vector<string> increment_tracker; //used to track incrementers in For Statements
    vector<pair<string, string>> conditional_tracker; //use to track conditional compares
    vector<string> assembly_instructions_;
    vector<tuple<string, int, Type>> variable_storage; // Vector of 3-tuple for storinh variable information  
    bool leaf_ = true; // Change to false if function calls another function
    bool has_locals_ = false; // Change to true if function creates local variables
    int return_value_;
    string function_name_;
    string return_type_;
    int offset = 0; 
    int conditional_counter = 0;

    private:

    // Private Data Members, want these to be protected
    const string Prologue[2] = {"pushq   %rbp", "movq    %rsp, %rbp"};
    string Epilogue[2] = {"popq    %rbp","ret"};
};
#endif