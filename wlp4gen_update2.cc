//
//  A10P5.cpp
//  A10
//
//  Created by Siyuan Guo on 2016-12-03.
//  Copyright © 2016 Siyuan Guo. All rights reserved.
//


#include <iostream>
#include <string>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <fstream>
#include <set>
using namespace std;

map<string ,map<string, string> > symboltable; // symboltable
set<string> functionset; // store the function name

string firsttype;  // to find out the type of first parameter in wain
int firsttypeindex = 0;

const int maxnode = 15;

struct node {        // struct of tree
    
    string symbol;  // store the name of this node
    
    string rule;  // store the rule
    
    string actualvalue; // actual value
    
    int itemnum;
    
    node *children[maxnode]; //
    
    node() : symbol("NULL"), rule("NULL"), actualvalue("NULL"), itemnum(0){
        for (int i = 0; i < maxnode; ++i) {
            children[i] = NULL;
        }
    }
    
    ~node() {
        for (int i = 0; i < itemnum; ++i) {
            delete children[i];
        }
    }
    
    void addnode(node *mynode, int num) {
        children[num] = mynode;
        ++itemnum;
    }
};

string trim(const string &str) {
    size_t begin = str.find_first_not_of(" \t\n");
    if (begin == string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n");
    
    return str.substr(begin, end - begin + 1);
}

// get the first element of a string
string getfirst(string c) {
    stringstream ss(c);
    string a;
    ss >> a;
    return a;
}

bool checkterminal(string input) {
    if (input == "BOF" || input == "BECOMES" || input == "COMMA" || input == "ELSE" || input == "EOF" || input == "EQ" || input == "GE" || input == "GT" ||
        input == "ID" || input == "IF" || input == "INT" || input == "LBRACE" ||
        input == "LE" || input == "LPAREN" || input == "LT" || input == "MINUS" ||
        input == "NE" || input ==  "NUM" || input == "PCT" || input == "PLUS" ||
        input == "PRINTLN" || input == "RBRACE" || input == "RETURN" || input == "RPAREN" || input == "SEMI" || input == "SLASH" || input == "STAR" || input == "WAIN" || input == "WHILE" || input == "AMP" || input == "LBRACK" ||
        input == "RBRACK" || input == "NEW" || input == "DELETE" || input == "NULL") {
        return true;
    } else {
        return false;
    }
}

void buildtree(node *tree) {
    string input;
    if (getline(cin, input)) { // there is more lines
        trim(input); // cut extra space
        string first = getfirst(input);
        if (checkterminal(first)) { // if terminal
            stringstream ss(input);
            string temp;
            ss >> temp;
            tree->symbol = temp;
            ss >> temp;
            tree->actualvalue = temp; // stores the actual name of terminals
        } else { // if nonterminal
            tree->rule = input;
            stringstream ss(input);
            string temp;
            ss >> temp;
            tree->symbol = temp;
            for (int i = 0; ss >> temp; ++i) {
                node *newnode = new node;
                buildtree(newnode);
                tree->addnode(newnode, i);
            }
        }
        
    } else {}
}

// read in a type
string findtype(node *tree) {
    if (tree->itemnum == 0) {
        return tree->actualvalue;
    } else {
        string name;
        name = tree->children[0]->actualvalue;
        for (int i = 1; i < tree->itemnum; ++i) {
            name = name + tree->children[i]->actualvalue;
        }
        return name;
    }
}

string waintype(node *tree) { // read “wain”
    string temp1 = findtype(tree->children[3]->children[0]);
    string temp2 = findtype(tree->children[5]->children[0]);
    string total = temp1 + " " + temp2;
    return total;
}

string proceduretype(node *tree) { // read paramlist
    string temp;
    if (tree->itemnum == 1) {
        temp = findtype(tree->children[0]->children[0]);
    } else {
        temp = findtype(tree->children[0]->children[0]) + " " + proceduretype(tree->children[2]);
    }
    return temp;
}


string gettype(node *tree); // forward declaration

string funname = "notfunction";
bool start = false;

bool type_error = true;

bool rule_error = true;

void printtree(node *tree) {
    if (tree->rule == "dcl type ID") {
        string name = tree->children[1]->actualvalue;
        string type = findtype(tree->children[0]);
        if (symboltable[funname].find(name) != symboltable[funname].end()) {
            //cerr << "ERROR:VARIABLE DEFINED MULTIPLE TIMES" << endl;
            return;
        } else {
            symboltable[funname][name] = type;
            //cerr << name << " " << type << endl;
            if (funname == "wain" && firsttypeindex == 0) { // 改动 A10P2
                firsttype = type;
                ++firsttypeindex;
            }
        }
        return ;
    }
    if (tree->rule == "factor ID LPAREN arglist RPAREN") {
        string pro_name = tree->children[0]->actualvalue; // get the name of procedure
        if (symboltable[funname].find(pro_name) != symboltable[funname].end()) {
            //cerr << "ERROR:PROCEDURE USES VARIABLE NAME" << endl;
            return;
        } else {
            if (functionset.count(pro_name) == 0) {
                //cerr << "ERROR:UNDEFINED PROCEDURE NAME" << endl;
                return;
            }
        }
        
        for (int i = 1; i < tree->itemnum; ++i) {
            printtree(tree->children[i]);
        }
        return;
    }
    if (tree->rule == "factor ID LPAREN RPAREN") { // if procedure like this
        string pro_name = tree->children[0]->actualvalue;
        if (symboltable[funname].find(pro_name) != symboltable[funname].end()) {
            //cerr << "ERROR:PROCEDURE USES VARIABLE NAME2" << endl;
            return;
        } else {
            if (functionset.count(pro_name) == 0) {
                //cerr << "ERROR:UNDEFINED PROCEDURE NAME EMPTY PAREN" << endl;
            }
        }
        for (int i = 1; i < tree->itemnum; ++i) {
            printtree(tree->children[i]);
        }
        return;
    }
    if (tree->symbol == "ID") {
        string name = tree->actualvalue;
        if (symboltable[funname].find(name) == symboltable[funname].end()) {
            //cerr << "ERROR:VARIABLE NOT DEFINED2" << endl;
            return;
        }
    }
    if (tree->symbol == "main") { // main function cannot call itself
        if (start) {         // decide if need to print newline
            //cerr << "\n";
        }
        start = true;
        string name = "wain";
        if (functionset.count(name) == 1) {
            //cerr << "ERROR:MAIN FUNCTION DEFINED MULTIPLE TIMES"<< endl;
            return;
        }
        functionset.insert(name);
        funname = name;
        string type = waintype(tree);
        //cerr << "wain" << " " << type << endl;
    }
    if (tree->symbol == "procedure") {
        if (start) {         // decide if need to print newline
            //cerr << "\n";
        }
        start = true;
        
        string name = tree->children[1]->actualvalue;
        if (functionset.count(name) == 1) {
            //cerr << "ERROR:FUNCTION DEFINED MULTIPLE TIMES"<< endl;
            return;
        }
        functionset.insert(name);
        funname = name;
        tree->children[1]->symbol = "PRO_ID";
        if (tree->children[3]->itemnum == 0) {
            //cerr << name << endl;
        } else {    // procedure has variable
            string type = proceduretype(tree->children[3]->children[0]); // paramlist
            //cerr << name << " " << type << endl;
        }
    }
    
    if (tree->symbol == "expr" || tree->symbol == "lvalue") { // call expr and lvalue
        type_error = true;
        gettype(tree);
    }
    
    for (int i = 0; i < tree->itemnum; ++i) {
        printtree(tree->children[i]);
    }
}



string gettype(node *tree) { // read tree that symbol = expr, term, ID, lvalue, factor
    string result = "bad"; // if bad, print bad;
    if (tree->rule == "expr term") {
        result = gettype(tree->children[0]);
    }
    if (tree->rule == "expr expr PLUS term") {
        string A = gettype(tree->children[0]);
        string B = gettype(tree->children[2]);
        if (A == "int" && B == "int") {
            result = "int";
        } else if (A == "int*" && B == "int") {
            result = "int*";
        } else if (A == "int" && B == "int*") {
            result = "int*";
        }
    }
    if (tree->rule == "expr expr MINUS term") {
        string A = gettype(tree->children[0]);
        string B = gettype(tree->children[2]);
        if (A == "int" && B == "int") {
            result = "int";
        } else if (A == "int*" && B == "int") {
            // well typed
            result = "int*";
        } else if (A == "int*" && B == "int*") {
            result = "int";
        }
    }
    if (tree->rule == "term factor") {
        result = gettype(tree->children[0]);
    }
    if (tree->rule == "term term STAR factor") {
        string A = gettype(tree->children[0]);
        string B = gettype(tree->children[2]);
        if (A == "int" && B == "int") {
            result = "int";
        }
    }
    if (tree->rule == "term term SLASH factor") {
        string A = gettype(tree->children[0]);
        string B = gettype(tree->children[2]);
        if (A == "int" && B == "int") {
            result = "int";
        }
    }
    if (tree->rule == "term term PCT factor") {
        string A = gettype(tree->children[0]);
        string B = gettype(tree->children[2]);
        if (A == "int" && B == "int") {
            result = "int";
        }
    }
    if (tree->rule == "factor ID") {
        string name = tree->children[0]->actualvalue;
        result = symboltable[funname][name];
    }
    if (tree->rule == "factor NUM") {
        result = "int";
    }
    if (tree->rule == "factor NULL") {
        result = "int*";
    }
    if (tree->rule == "factor LPAREN expr RPAREN") {
        result = gettype(tree->children[1]);
    }
    if (tree->rule == "factor AMP lvalue") {
        string A = gettype(tree->children[1]);
        if (A == "int") {
            result = "*int";
        }
    }
    if (tree->rule == "factor STAR factor") {
        string A = gettype(tree->children[1]);
        if (A == "int*") {
            result = "int";
        }
    }
    if (tree->rule == "factor NEW INT LBRACK expr RBRACK") {
        string A = gettype(tree->children[3]);
        if (A == "int") {
            result = "int*";
        }
    }
    if (tree->rule == "factor ID LPAREN RPAREN") { //所有procedure都return int
        result = "int";
    }
    if (tree->rule == "factor ID LPAREN arglist RPAREN") { //所有procedure都return int
        result = "int";
        
    }
    if (tree->rule == "lvalue ID" ) { // 从symboltable里找ID
        string name = tree->children[0]->actualvalue;
        result = symboltable[funname][name];
    }
    if (tree->rule == "lvalue STAR factor") {
        string A = gettype(tree->children[1]);
        if (A == "int*") {
            result = "int";
        }
    }
    if (tree->rule == "lvalue LPAREN lvalue RPAREN") {
        result = gettype(tree->children[1]);
    }
    if (type_error == true && result == "bad") { // if result == "bad"!只print一次
        //cerr << "ERROR: ERROR IN EXPRESSION TYPES" << endl;
        type_error = false;
    }
    return result;
}


// MIPS CODE AREA

map< string ,map< string, int > > postable;  // table that store position of each variable
string mipsfun = "nofunction"; // stores the name of function
int offset = 0; // stores the offset of PC
string id1; // name of variable in $1
string id2; // name of variable in $2
int whileloop = 0; // whileloop counter avoid duplication
int ifstate = 0; // if statement counter avoid duplication
int deletecounter = 0; // counter for delete to avoid duplication
int newcounter = 0; // counter for new to avoid duplication
int argnumber = 0;

// reads in a register, print mips code which pushs the register value into stack
int push_stack(string regist) {
    cout << "sw " << regist << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    int position = offset;
    offset = offset - 4; // decrease offset
    return position;
}

// reads in a register, and stack position, print mips code which stores the value in stack to register provided
void pop_stack(string regist, int position) {
    cout << "lw " << regist << ", " << position << "($29)" << endl;
    cout << "add $30, $30, $4" << endl;
    offset = offset + 4; // increase offset
}

// CODE AREA


void refer_lvalue(node *tree); // forward declaration

// only read in a node with symbol name "expr", "term", "factor", "test", then print corresponding mips code according to this node's rule. The result is always stored in $3
void code(node *tree) {
    if (tree->rule == "expr term") { // if rule is "expr term"
        code(tree->children[0]);
    }
    if (tree->rule == "term factor") { // if rule is "term factor"
        code(tree->children[0]);
    }
    if (tree->rule == "factor LPAREN expr RPAREN") { // if rule is "factor LPAREN expr RPAREN"
        code(tree->children[1]);
    }
    if (tree->rule == "factor ID") { // if rule is
        string name = tree->children[0]->actualvalue;
        int position = postable[mipsfun][name];
        cout << "lw $3, " << position << "($29)" << endl;
    }
    if (tree->rule == "expr expr PLUS term") { // A10P3
        string typeleft = gettype(tree->children[0]);
        string typeright = gettype(tree->children[2]);
        if (typeleft == "int" && typeright == "int") {
            code(tree->children[0]);
            int position = push_stack("$3");
            code(tree->children[2]);
            cout << "add $5, $3, $0" << endl;
            pop_stack("$3", position);
            cout << "add $3, $3, $5" << endl;
        } else if (typeleft == "int*") {
            code(tree->children[0]);
            int position = push_stack("$3");
            code(tree->children[2]);
            cout << "add $5, $3, $0" << endl;
            cout << "mult $5, $4" << endl;
            cout << "mflo $5" << endl;
            pop_stack("$3", position);
            cout << "add $3, $3, $5" << endl;
        } else if (typeright == "int*") {
            code(tree->children[0]);
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            int position = push_stack("$3");
            code(tree->children[2]);
            cout << "add $5, $3, $0" << endl;
            pop_stack("$3", position);
            cout << "add $3, $3, $5" << endl;
        }
        
    }
    if (tree->rule == "expr expr MINUS term") { // A10P3
        string typeleft = gettype(tree->children[0]);
        string typeright = gettype(tree->children[2]);
        if (typeleft == "int" && typeright == "int") {
            code(tree->children[0]);
            int position = push_stack("$3");
            code(tree->children[2]);
            cout << "add $5, $3, $0" << endl;
            pop_stack("$3", position);
            cout << "sub $3, $3, $5" << endl;
        } else if (typeleft == "int*" && typeright == "int") {
            code(tree->children[0]);
            int position = push_stack("$3");
            code(tree->children[2]);
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            pop_stack("$5", position);
            cout << "sub $3, $5, $3" << endl;
        } else if (typeleft == "int*" && typeright == "int*") {
            code(tree->children[0]);
            int position = push_stack("$3");
            code(tree->children[2]);
            pop_stack("$5", position);
            cout << "sub $3, $5, $3" << endl;
            cout << "div $3, $4" << endl;
            cout << "mflo $3" << endl;
        }
    }
    if (tree->rule == "term term STAR factor") { // if rule is "term term STAR factor"
        code(tree->children[0]);
        int position = push_stack("$3");
        code(tree->children[2]);
        cout << "add $5, $3, $0" << endl;
        pop_stack("$3", position);
        cout << "mult $3, $5" << endl;
        cout << "mflo $3" << endl;
    }
    if (tree->rule == "term term SLASH factor") { // if rule is "term term SLASH factor"
        code(tree->children[0]);
        int position = push_stack("$3");
        code(tree->children[2]);
        cout << "add $5, $3, $0" << endl;
        pop_stack("$3", position);
        cout << "div $3, $5" << endl;
        cout << "mflo $3" << endl;
    }
    if (tree->rule == "term term PCT factor") { // if rule is "term term PCT factor"
        code(tree->children[0]);
        int position = push_stack("$3");
        code(tree->children[2]);
        cout << "add $5, $3, $0" << endl;
        pop_stack("$3", position);
        cout << "div $3, $5" << endl;
        cout << "mfhi $3" << endl;
    }
    if (tree->rule == "factor NUM") { // if rule is "factor NUM"
        cout << "lis $3" << endl;
        cout << ".word " << tree->children[0]->actualvalue << endl;
    }
    if (tree->rule == "test expr LT expr") { // if rule is "test expr LT expr" A10P4
        string type = gettype(tree->children[0]);
        code(tree->children[0]);
        cout << "add $6, $3, $0" << endl;
        code(tree->children[2]);
        if (type == "int") {
            cout << "slt $3, $6, $3" << endl;
        }
        if (type == "int*") {
            cout << "sltu $3, $6, $3" << endl;
        }
    }
    if (tree->rule == "test expr GT expr") { // if rule is "test expr GT expr" A10P4
        string type = gettype(tree->children[0]);
        code(tree->children[0]);
        cout << "add $6, $3, $0" << endl;
        code(tree->children[2]);
        if (type == "int") {
            cout << "slt $3, $3, $6" << endl;
        }
        if (type == "int*") {
            cout << "sltu $3, $3, $6" << endl;
        }
    }
    if (tree->rule == "test expr EQ expr") { // if rule is "test expr EQ expr" A10P4
        string type = gettype(tree->children[0]);
        code(tree->children[0]);
        cout << "add $6, $3, $0" << endl;
        code(tree->children[2]);
        if (type == "int") {
            cout << "slt $7, $6, $3" << endl;
            cout << "slt $8, $3, $6" << endl;
        }
        if (type == "int*") {
            cout << "sltu $7, $6, $3" << endl;
            cout << "sltu $8, $3, $6" << endl;
        }
        cout << "add $3, $7, $8" << endl;
        cout << "sub $3, $11, $3" << endl;
    }
    if (tree->rule == "test expr NE expr") { // if rule is "test expr NE expr" A10P4
        string type = gettype(tree->children[0]);
        code(tree->children[0]);
        cout << "add $6, $3, $0" << endl;
        code(tree->children[2]);
        if (type == "int") {
            cout << "slt $7, $6, $3" << endl;
            cout << "slt $8, $3, $6" << endl;
        }
        if (type == "int*") {
            cout << "sltu $7, $6, $3" << endl;
            cout << "sltu $8, $3, $6" << endl;
        }
        cout << "add $3, $7, $8" << endl;
    }
    if (tree->rule == "test expr LE expr") { // if rule is "test expr LE expr" A10P4
        string type = gettype(tree->children[0]);
        code(tree->children[0]);
        cout << "add $6, $3, $0" << endl;
        code(tree->children[2]);
        if (type == "int") {
            cout << "slt $3, $3, $6" << endl;
        }
        if (type == "int*") {
            cout << "sltu $3, $3, $6" << endl;
        }
        cout << "sub $3, $11, $3" << endl;
    }
    if (tree->rule == "test expr GE expr") { // if rule is "test expr GE expr" A10P4
        string type = gettype(tree->children[0]);
        code(tree->children[0]);
        cout << "add $6, $3, $0" << endl;
        code(tree->children[2]);
        if (type == "int") {
            cout << "slt $3, $6, $3" << endl;
        }
        if (type == "int*") {
            cout << "sltu $3, $6, $3" << endl;
        }
        cout << "sub $3, $11, $3" << endl;
    }
    if (tree->rule == "factor NULL") { //A10P1
        cout << "add $3, $11, $0" << endl;
    }
    if (tree->rule == "factor AMP lvalue") { // A10P1
        refer_lvalue(tree->children[1]);
    }
    if (tree->rule == "factor STAR factor") { // A10P1
        code(tree->children[1]);
        cout << "lw $3, 0($3)" << endl;
    }
    if (tree->rule == "factor NEW INT LBRACK expr RBRACK") { // A10P2
        int newindex = newcounter;
        ++newcounter;
        code(tree->children[3]);
        int position1 = push_stack("$1");
        cout << "add $1, $3, $0" << endl;
        int position31 = push_stack("$31");
        cout << "lis $5" << endl;
        cout << ".word new" << endl;
        cout << "jalr $5" << endl;
        pop_stack("$31", position31);
        pop_stack("$1", position1);
        cout << "bne $3, $0, skipnew" << newindex << endl;
        cout << "add $3, $11, $0" << endl;
        cout << "skipnew" << newindex << ":" << endl;
    }
    if (tree->rule == "factor ID LPAREN RPAREN") { // A10P5
        string procedure = tree->children[0]->actualvalue;
        int position29 = push_stack("$29");
        int position31 = push_stack("$31");
        cout << "lis $5" << endl;
        cout << ".word F" << procedure << endl;
        cout << "jalr $5" << endl;
        pop_stack("$31", 4);
        pop_stack("$29", 8);
    }
    if (tree->rule == "factor ID LPAREN arglist RPAREN") { //A10P6
        string procedure = tree->children[0]->actualvalue;
        int position29 = push_stack("$29");
        int position31 = push_stack("$31");
        code(tree->children[2]);
        while (argnumber != 0) {
            cout << "add $30, $30, $4" << endl;
            
            //改了。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
            offset = offset + 4;
            
            --argnumber;
        }
        cout << "lis $5" << endl;
        cout << ".word F" << procedure << endl;
        cout << "jalr $5" << endl;
        pop_stack("$31", 4);
        pop_stack("$29", 8);
    }
    if (tree->rule == "arglist expr") { //A10P6
        code(tree->children[0]);
        push_stack("$3");
        ++argnumber;
    }
    if (tree->rule == "arglist expr COMMA arglist") {//A10P6
        code(tree->children[0]);
        // push above
        push_stack("$3");
        ++argnumber;
        
        code(tree->children[2]); // will do push as well
    }
}

// read in lvalue
void refer_lvalue(node *tree) {  // A10P1
    if (tree->rule == "lvalue LPAREN lvalue RPAREN") {
        refer_lvalue(tree->children[1]);
    }
    if (tree->rule == "lvalue STAR factor") {
        code(tree->children[1]);
    }
    if (tree->rule == "lvalue ID") {
        string name = tree->children[0]->actualvalue;
        int position = postable[mipsfun][name];
        cout << "lis $3" << endl;
        cout << ".word " << position << endl;
        cout << "add $3, $29, $3" << endl;
    }
}




// read in lvalues and its expr on the right hand side
void process_lvalue(node *lvalue, node *expr) {          //A10P1
    if (lvalue->rule == "lvalue LPAREN lvalue RPAREN") {
        process_lvalue(lvalue->children[1], expr);
    }
    if (lvalue->rule == "lvalue STAR factor") {
        code(lvalue->children[1]);
        int position = push_stack("$3");
        code(expr);
        pop_stack("$5", position);
        cout << "sw $3, 0($5)" << endl;
    }
    if (lvalue->rule == "lvalue ID") {
        string name = lvalue->children[0]->actualvalue; // handle "lvalue"
        code(expr); // handle "expr"
        int position = postable[mipsfun][name]; // insert in variable symbol table
        cout << "sw $3, " << position << "($29)" << endl;
    }
}




// only reads in the node which has symbol "statements" and "statement", then print out corresponding mips code according to different production fules
void state_code(node *tree) {
    if (tree->rule == "statements") { // if production rule is "statements"
        return;
    }
    if (tree->rule == "statements statements statement") {
        state_code(tree->children[0]); // handle "statements"
        state_code(tree->children[1]); // handle "statement"
    }
    if (tree->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        code(tree->children[2]); // handle "expr"
        int position_1 = push_stack("$1");
        int position_31 = push_stack("$31");
        cout << "add $1, $3, $0" << endl;
        cout << "lis $5" << endl;
        cout << ".word print" << endl;
        cout << "jalr $5" << endl;
        pop_stack("$31", position_31);
        pop_stack("$1", position_1);
        return;
    }
    if (tree->rule == "statement lvalue BECOMES expr SEMI") {
        /*string name = lvalue_name(tree->children[0]); // handle "lavlue"
         code(tree->children[2]); // handle "expr"
         int position = postable[name]; // insert in variable symbol table
         cout << "sw $3, " << position << "($29)" << endl;
         */
        process_lvalue(tree->children[0], tree->children[2]); // A10P1
    }
    if (tree->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        int index = whileloop;
        ++whileloop;
        cout << "loop" << index << ":" << endl;
        code(tree->children[2]); // handle "test"
        cout << "beq $3, $0, endloop" << index << endl;
        state_code(tree->children[5]); // handle "statements"
        cout << "beq $0, $0, loop" << index << endl;
        cout << "endloop" << index << ":" << endl;
    }
    if (tree->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        int ifindex = ifstate;
        ++ifstate;
        code(tree->children[2]); // handle "test"
        cout << "beq $3, $0, else" << ifindex << endl;
        state_code(tree->children[5]); // handle "statements"
        cout << "beq $0, $0, endif" << ifindex << endl;
        cout << "else" << ifindex << ":" << endl;
        state_code(tree->children[9]); // handle "statements"
        cout << "endif" << ifindex << ":" << endl;
    }
    if (tree->rule == "statement DELETE LBRACK RBRACK expr SEMI") { // A10P2
        int deleteindex = deletecounter;
        ++deletecounter;
        code(tree->children[3]);
        cout << "beq $3, $11, deleteskip" << deleteindex << endl;
        int position1 = push_stack("$1");
        cout << "add $1, $3, $0" << endl;
        int position31 = push_stack("$31");
        cout << "lis $5" << endl;
        cout << ".word delete" << endl;
        cout << "jalr $5" << endl;
        pop_stack("$31", position31);
        pop_stack("$1", position1);
        cout << "deleteskip" << deleteindex << ":" << endl;
    }
}

// POSTABLE AREA
// only read in node which has symbol name "dcls"
void pos_table(node *tree) {
    if (tree->rule == "dcls") { // if production rule is "dcls"
        return;
    }
    if (tree->rule == "dcls dcls dcl BECOMES NUM SEMI") { // if production rule is "dcls dcls dcl BECOMES NUM SEMI"
        pos_table(tree->children[0]); // handle "dcls"
        
        string name = tree->children[1]->children[1]->actualvalue; // get id name
        string value = tree->children[3]->actualvalue; // handle "NUM", get value
        cout << "lis $3" << endl;
        cout << ".word " << value << endl;
        int position = push_stack("$3"); // push value into stack
        postable[mipsfun][name] = position; // store variable into symbol table
    }
    if (tree->rule == "dcls dcls dcl BECOMES NULL SEMI") { // A10P1
        pos_table(tree->children[0]);
        string name = tree->children[1]->children[1]->actualvalue;
        cout << "add $3, $11, $0" << endl; // let $3 == 1
        int position = push_stack("$3");
        postable[mipsfun][name] = position;
    }
}



// params area

void param_code(node *tree) { // read params.............有问题需要改
    if (tree->rule == "params") {
        return;
    }
    if (tree->rule == "params paramlist") {
        param_code(tree->children[0]);
    }
    if (tree->rule == "paramlist dcl") {
        string name = tree->children[0]->children[1]->actualvalue;
        postable[mipsfun][name] = offset;
        cout << "sub $30, $30, $4" << endl;
        offset = offset - 4;
    }
    if (tree->rule == "paramlist dcl COMMA paramlist") {
        string name = tree->children[0]->children[1]->actualvalue;
        postable[mipsfun][name] = offset;
        cout << "sub $30, $30, $4" << endl;
        offset = offset - 4;
        param_code(tree->children[2]);
    }
}

// PRINTMIPS AREA
// process the parse tree again to produce corresponding mips code read in the root of parse tree
void printmips(node *tree) {
    if (tree->symbol == "main") { // if in main function
        
        mipsfun = "wain";
        funname = "wain"; // .....................................
        
        
        cout << "wain:" << endl;
        cout << "sub $29, $30, $4" << endl;
        id1 = tree->children[3]->children[1]->actualvalue; // get id name of $1 and $2
        id2 = tree->children[5]->children[1]->actualvalue;
        push_stack("$1");
        postable[mipsfun][id1] = 0; // put $1 and $2 into variable symbol table
        push_stack("$2");
        postable[mipsfun][id2] = -4;
        
        if (firsttype == "int") { // A10P2
            cout << "add $2, $0, $0" << endl;
            int position31 = push_stack("$31");
            cout << "lis $5" << endl;
            cout << ".word init" << endl;
            cout << "jalr $5" << endl;
            pop_stack("$31", position31);
            cout << "lw $2, -4($29)" << endl;
        } else {
            int position31 = push_stack("$31");
            cout << "lis $5" << endl;
            cout << ".word init" << endl;
            cout << "jalr $5" << endl;
            pop_stack("$31", position31);
        }
        
        for (int i = 0; i < tree->itemnum; ++i) {
            if (tree->children[i]->symbol == "expr") { // if symbol is "expr"
                code(tree->children[i]);
            }
            if (tree->children[i]->symbol == "statements") { // if symbol is "statements"
                state_code(tree->children[i]);
            }
            if (tree->children[i]->symbol == "dcls") { // if symbol is "dcls"
                pos_table(tree->children[i]);
            }
        }
        
        if (offset != 0) { // pop all the things from stack and return $30 to its original position
            cout << "lis $5" << endl;
            cout << ".word " << offset << endl;
            cout << "sub $30, $30, $5" << endl;
            offset = 0;
        }
        cout << "jr $31" << endl; // jump out
        
        return;
    }
    if (tree->symbol == "procedure") {
        int originoffset = offset;
        offset = 0;
        string procedurename = tree->children[1]->actualvalue;
        mipsfun = procedurename;
        funname = procedurename; // ...................................
        
        cout << "F" << procedurename << ":" << endl;
        cout << "sub $29, $30, $4" << endl;
        
        param_code(tree->children[3]);
        
        int position1 = push_stack("$1"); // store all the register that must not be changed
        int position2 = push_stack("$2");
        //$4 will not be changed
        int position5 = push_stack("$5");
        int position6 = push_stack("$6");
        int position7 = push_stack("$7");
        int position8 = push_stack("$8");
        int position9 = push_stack("$9");
        for (int i = 0; i < tree->itemnum; ++i) {
            if (tree->children[i]->symbol == "dcls") {
                
                //cerr << "haha1" << endl;
                
                pos_table(tree->children[i]);
            }
            if (tree->children[i]->symbol == "statements") {
                
                //cerr << "haha2" << endl;
                
                state_code(tree->children[i]);
            }
            if (tree->children[i]->symbol == "expr") { // $3 stores the value to be returned
                
                //cerr << "haha3" << endl;
                
                code(tree->children[i]);
                pop_stack("$9", position9);
                pop_stack("$8", position8);
                pop_stack("$7", position7);
                pop_stack("$6", position6);
                pop_stack("$5", position5);
                pop_stack("$2", position2);
                pop_stack("$1", position1);
                cout << "add $30, $29, $4" << endl;
                cout << "jr $31" << endl;
                offset = originoffset;
                break;
            } // if "expr"
        } // for loop
    }
    for (int i = 0; i < tree->itemnum; ++i) { // recursively read in subnodes
        printmips(tree->children[i]);
    }
    
}




int main() {
    
    node *mytree = new node;
    
    buildtree(mytree);
    
    printtree(mytree);
    
    funname = "notfunction"; // ...................................
    
    cout << ".import print" << endl; // import print proceudre
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;
    cout << "lis $4" << endl; // set constant values
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "beq $0, $0, wain" << endl;
    printmips(mytree);
    
    delete mytree;
}
