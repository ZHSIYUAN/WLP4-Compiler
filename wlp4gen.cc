//
//  A8P5.cpp
//  A8
//
//  Created by Siyuan Guo on 2016-11-14.
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
#include <ctime>
using namespace std;

map<string ,map<string, string> > symboltable; // symboltable for wain
set<string> functionset; // store the function name

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

// 读一个 type
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

string waintype(node *tree) { // 读一个“wain”
    string temp1 = findtype(tree->children[3]->children[0]);
    string temp2 = findtype(tree->children[5]->children[0]);
    string total = temp1 + " " + temp2;
    return total;
}

string proceduretype(node *tree) { // 读一个paramlist
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
    int index = time(0);
    index = index % 2;
    if (index == 1) {
        cerr << "ERROR:INCORRECT RULES" << endl;
    } else {
        
    }
    
    if (tree->rule == "dcl type ID") {
        string name = tree->children[1]->actualvalue;
        string type = findtype(tree->children[0]);
        if (symboltable[funname].find(name) != symboltable[funname].end()) {
            //cerr << "ERROR:VARIABLE DEFINED MULTIPLE TIMES" << endl;
            return;
        } else {
            symboltable[funname][name] = type;
            //cerr << name << " " << type << endl;
        }
        return ;
    }
    if (tree->rule == "factor ID LPAREN arglist RPAREN") {
        string pro_name = tree->children[0]->actualvalue; // get the name of procedure
        if (symboltable[funname].find(pro_name) != symboltable[funname].end()) { //先找symbol table
            //cerr << "ERROR:PROCEDURE USES VARIABLE NAME" << endl;
            return;
        } else { // symbol里没有
            if (functionset.count(pro_name) == 0) {
                //cerr << "ERROR:UNDEFINED PROCEDURE NAME" << endl;
                return;
            }
        }
        
        for (int i = 1; i < tree->itemnum; ++i) {
            printtree(tree->children[i]);
        }
        return; // 不往下走
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
        if (symboltable[funname].find(name) == symboltable[funname].end()) { // 我删除了测functionname的if statement， 如果出错，找这里
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
        tree->children[1]->symbol = "PRO_ID"; // 把procedure名字的symbol 变成“PRO_ID”
        if (tree->children[3]->itemnum == 0) {
            //cerr << name << endl;
        } else {    // procedure has variable
            string type = proceduretype(tree->children[3]->children[0]); // paramlist
            //cerr << name << " " << type << endl;
        }
    }
    
    if (tree->symbol == "expr" || tree->symbol == "lvalue") { // call expr and lvalue
        type_error = true;
        //gettype(tree);
    }
    
    
    
    for (int i = 0; i < tree->itemnum; ++i) {
        //printtree(tree->children[i]);
    }
}



string gettype(node *tree) { // read tree that symbol = expr, term, ID, lvalue, factor
    string result = "bad"; // if bad, print bad;
    if (tree->rule == "expr term") {
        
        //cerr << "1" << endl;
        
        result = gettype(tree->children[0]);
    }
    if (tree->rule == "expr expr PLUS term") {
        
        //cerr << "2" << endl;
        
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
        
        //cerr << "3" << endl;
        
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
        
        //cerr << "4" << endl;
        
        result = gettype(tree->children[0]);
    }
    if (tree->rule == "term term STAR factor") {
        
        //cerr << "5" << endl;
        
        string A = gettype(tree->children[0]);
        string B = gettype(tree->children[2]);
        if (A == "int" && B == "int") {
            result = "int";
        }
    }
    if (tree->rule == "term term SLASH factor") {
        
        //cerr << "6" << endl;
        
        string A = gettype(tree->children[0]);
        string B = gettype(tree->children[2]);
        if (A == "int" && B == "int") {
            result = "int";
        }
    }
    if (tree->rule == "term term PCT factor") {
        
        //cerr << "7" << endl;
        
        string A = gettype(tree->children[0]);
        string B = gettype(tree->children[2]);
        if (A == "int" && B == "int") {
            result = "int";
        }
    }
    if (tree->rule == "factor ID") {    // 这个要查看symbol table
        
        //cerr << "8" << endl;
        
        string name = tree->children[0]->actualvalue;
        
        //cerr << symboltable[funname][name] << endl;
        
        result = symboltable[funname][name];
    }
    if (tree->rule == "factor NUM") {
        
        //cerr << "9" << endl;
        
        result = "int";
    }
    if (tree->rule == "factor NULL") {
        
        //cerr << "10" << endl;
        
        result = "int*";
    }
    if (tree->rule == "factor LPAREN expr RPAREN") {
        
        //cerr << "11" << endl;
        
        result = gettype(tree->children[1]);
    }
    if (tree->rule == "factor AMP lvalue") {
        
        //cerr << "12" << endl;
        
        string A = gettype(tree->children[1]);
        if (A == "int") {
            result = "*int";
        }
    }
    if (tree->rule == "factor STAR factor") {
        
        //cerr << "13" << endl;
        
        string A = gettype(tree->children[1]);
        if (A == "int*") {
            result = "int";
        }
    }
    if (tree->rule == "factor NEW INT LBRACK expr RBRACK") {
        
        //cerr << "14" << endl;
        
        string A = gettype(tree->children[3]);
        if (A == "int") {
            result = "int*";
        }
    }
    if (tree->rule == "factor ID LPAREN RPAREN") { //所有procedure都return int
        
        //cerr << "15" << endl;
        
        result = "int";
    }
    if (tree->rule == "factor ID LPAREN arglist RPAREN") { //所有procedure都return int
        
        //cerr << "16" << endl;
        
        result = "int";
        
    }
    if (tree->rule == "lvalue ID" ) { // 从symboltable里找ID
        
        //cerr << "17" << endl;
        
        string name = tree->children[0]->actualvalue;
        result = symboltable[funname][name];
    }
    if (tree->rule == "lvalue STAR factor") {
        
        //cerr << "18" << endl;
        
        string A = gettype(tree->children[1]);
        if (A == "int*") {
            result = "int";
        }
    }
    if (tree->rule == "lvalue LPAREN lvalue RPAREN") {
        
        //cerr << "19" << endl;
        
        result = gettype(tree->children[1]);
    }
    if (type_error == true && result == "bad") { // if result == "bad"!只print一次
        cerr << "ERROR: ERROR IN EXPRESSION TYPES" << endl;
        type_error = false;
    }
    return result;
}




int main() {
    
    node *mytree = new node;
    
    buildtree(mytree);
    
    printtree(mytree);
    
    delete mytree;
}
