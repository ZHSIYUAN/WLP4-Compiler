#include <set>
#include <string>
#include <iostream>
#include <list>
#include <sstream>
#include <fstream>
#include <map>
#include <stack>
#include <vector>
#include <utility>
using namespace std;

set<string> terms;
set<string> nonterms;
//set<string> prods;
map<string, map<string, int> > setTable;
map<string, pair<vector<string>, map<string, string> > > symTable;
map<string, vector<string> > paraTable;
set<string> funName;
string start;
string function = "N/A";
string globalTempName = "N/A";
int paraCount = 0;
int X = 0;
int Y = 0;
bool error = false;
int offset = 0;
ifstream fin("grammar");

struct Tree {
	string rule;
	list<string> tokens;
	list<Tree*> children;

	~Tree() {
	for(list<Tree*>::iterator it=children.begin(); it != children.end(); it++) {  // delete all subtrees
		delete (*it);
		}
	}
};


void readsyms(set<string> &t) {
    int n;
    string temp;
    getline(fin,temp);
    istringstream iss(temp);
    iss >> n;
    if (iss.fail()) return;
    for(int i = 0; i < n; i++) {
        getline(fin,temp);
        t.insert(temp);
    }
}

void readInTree(Tree *t){
	string tmp;
	if (!(getline(cin, tmp))) return;
	else {
		t->rule = tmp;
		stringstream tin(tmp);
		while (tin >> tmp) {
			(t->tokens).push_back(tmp);
		}
		list<string>::iterator it = (t->tokens).begin();
		if (terms.count(*it) > 0) return;
		it++;
		for (; it != (t->tokens).end(); it++) {
			Tree *n = new Tree();
			readInTree(n);
			(t->children).push_back(n);
		}
	}
}	

string givenName(Tree *t) {
	string temp;
	list<string>::iterator id = (t->tokens).begin();
	temp = *(++id);
	return temp;
}

			
string givenType(Tree *t) {
	string temp;
	list<Tree*>::iterator tree = (t->children).begin();
	for (; tree != (t->children).end(); tree++)
		temp = temp + givenName(*tree);
	return temp;
}

void readParams(Tree *t) { // only when params is not void
	if (t->rule == "dcl type ID") {
		list<Tree*>::iterator type;
		type = (t->children).begin();
		string typeName = givenType(*type);
		type++;
		string idName = givenName(*type);
		paraTable[function].push_back(idName);
		(symTable[function].first).push_back(typeName);
		return;
	}
	for(list<Tree*>::iterator it=(t->children).begin(); it != (t->children).end(); it++) {
		readParams(*it);
		if (error == true) return;
	}
}

void readWainParams(Tree *t) {
	list<Tree*>::iterator type;
	type = (t->children).begin();
	string typeName = givenType(*type);
	(symTable["wain"].first).push_back(typeName);
	cerr << " " << typeName;
}


string getExpression(Tree*); // pre declare

void checkArg (Tree *t, string proName, int &numPara) {
	if (numPara >= (symTable[proName].first).size()) {
		cerr << " ERROR: TOO MANY PARAMETERS!" << endl;
		error = true;
		return;
	}
	else if ((t->rule == "arglist expr") ||
	    (t->rule == "arglist expr COMMA arglist")) {		//expr Check
		list<Tree*>::iterator child = (t->children).begin();
		string typName = getExpression(*child);
		if (error == true) return;
		if (typName != (symTable[proName].first)[numPara]) {
			cerr << " ERROR: WRONG TYPE OF PARAMETERS!" << endl;
			error = true;
			return;
		}
		if (t->rule == "arglist expr COMMA arglist") {
			numPara++;
			child++; child++;
			checkArg(*child, proName, numPara);
		}
	}
	else if ((t->rule == "factor ID LPAREN arglist RPAREN") ||
		 (t->rule == "factor ID LPAREN RPAREN")) {
			list<Tree*>::iterator child = (t->children).begin();
			string fnName = givenName(*child);
			if (funName.count(fnName) == 0) {
				error = true;
				cerr << " ERROR: CANNOT find that FUNCTION NAME!" << endl;
				return;
			}
	}
	else if (t->rule == "factor ID" || t->rule == "lvalue ID") {
		list<Tree*>::iterator child = (t->children).begin();
		string idName = givenName(*child);
		if (idName == proName) {
			error = true;
			cerr << " ERROR: CANNOT determine " << idName << " is a procedure or a variable!" << endl;
			return;
		}
		else if ((symTable[function].second).find(idName) == (symTable[function].second).end()) {
			cerr << idName;
			cerr << " ERROR: DO NOT HAVE SUCH ID" << endl;
			error = true;
			return;
		}
	}
}


string getExpression(Tree *t) {
	if (error == true) return "noType";
	list<Tree*>::iterator it;
	if (t->rule == "expr expr PLUS term") {
		it = (t->children).begin();
		string lhs = getExpression(*it);
		it++;	it++;
		string rhs = getExpression(*it);
		if (error == true) return "noType";
		else if (lhs == "int*" && rhs == "int*") {
			cerr << " ERROR: TWO POINTER CANNOT BE ADDED!" << endl;
			error = true;
			return "noType";
		}
		else if (lhs == "int*" || rhs == "int*") {
			string result = "int*";
			return result;
		}
		else if (lhs == rhs && rhs == "int") {
			string result = "int";
			return result;
		}
	}
	else if (t->rule == "expr expr MINUS term") {
		it = (t->children).begin();
		string lhs = getExpression(*it);
		it++;	it++;
		string rhs = getExpression(*it);
		if (error == true) return "noType";
		else if (lhs == "int*" && rhs == "int*") return "int";
		else if (lhs == "int" && rhs == "int") return "int";
		else if (lhs == "int*" && rhs == "int") return "int*";
		else {
			cerr << " ERROR: A INT CANNOT BE DIFFERENTIATED BY A POINTER!" << endl;
			error = true;
			return "noType";
		}
	}
	else if ((t->rule == "term term STAR factor") ||
		 (t->rule == "term term SLASH factor") ||
		 (t->rule == "term term PCT factor")) {
		it = (t->children).begin();
		string lhs = getExpression(*it);
		it++; it++;
		string rhs = getExpression(*it);
		if (error == true) return "noType";
		else if (lhs == rhs && lhs == "int") return "int";
		else {
			cerr << " ERROR: WRONG PARAMETERS!" << endl;
			error = true;
			return "noType";
		}
	}
	else if (t->rule == "expr term" || t->rule == "term factor" ||
		 t->rule == "factor NUM" || t->rule == "factor NULL") {
		it = (t->children).begin();
		return getExpression(*it);
	}
	else if ((t->rule == "factor ID") || (t->rule == "lvalue ID")) {
		it = (t->children).begin();
		string tmpId = givenName(*it);
		if ((symTable[function].second).find(tmpId) == (symTable[function].second).end()) {
				cerr << " ERROR: THIS SYMBOL DOES NOT EXIST!" << endl;
				error = true;
				return "noType";
		}
		else return getExpression(*it);
	}
	else if (t->rule == "factor LPAREN expr RPAREN") {
		it = (t->children).begin();
		it++;
		return getExpression(*it);
	}
	else if (t->rule == "factor AMP lvalue") {
		it = (t->children).begin();
		it++;
		string later = getExpression(*it);
		if (later == "int") return "int*";
		else {
			cerr << " ERROR: WRONG TYPE AFTER &" << endl;
			error = true;
			return "noType";
		}
	}
	else if (t->rule == "factor STAR factor" ||
		 t->rule == "lvalue STAR factor") {
		it = (t->children).begin();
		it++;
		string later = getExpression(*it);
		if (later == "int*") return "int";
		else {
			cerr << " ERROR: WRONG TYPE AFTER *" << endl;
			error = true;
			return "noType";
		}
	}
	else if (t->rule == "lvalue LPAREN lvalue RPAREN") {
		it = (t->children).begin();
		it++;
		string lvStr = getExpression(*it);
		return lvStr;
	}
	else if (t->rule == "factor NEW INT LBRACK expr RBRACK") {
		it = (t->children).begin();
		while ((*it)->tokens.front() != "expr") it++;
		if (getExpression(*it) == "int") return "int*";
		else {
			cerr << " ERROR: WRONG TYPE AFTER new int []" << endl;
			error = true;
			return "noType";
		}
	}	
	else if ((t->rule == "factor ID LPAREN arglist RPAREN") ||
		 (t->rule == "factor ID LPAREN RPAREN")) {
		it = (t->children).begin();
		string tmpPro = givenName(*it);
		if ((symTable[function].second).find(tmpPro) != (symTable[function].second).end()) {
			cerr << " ERROR: VARIABLE IS NOT A FUNCTION!" << endl;
			error = true;
			return "noType";
		}
		if (funName.count(tmpPro) == 0) {
			cerr << " ERROR: FUNCTION HAS NOT BEEN DECLARED!" << endl;
			error = true;
			return "noType";
		}
		if (t->rule == "factor ID LPAREN RPAREN") {
			if ((symTable[tmpPro].first).size() != 0) {
				error = true;
				cerr << " ERROR: NUM OF PARAMETERS IS NOT RIGHT!" << endl;
				return "noType";
			}
			else return "int";
		}
		else {
			while (((*it)->tokens).front() != "arglist") it++;
			int numPara = 0;
			checkArg(*it, tmpPro, numPara);
			if (error == true) return "noType";
			else if (numPara+1 != (symTable[tmpPro].first).size()) {
				error = true;
				cerr << numPara << endl;
				cerr << " ERROR: NUM OF PARAMETERS IS NOT RIGHT!" << endl;
				return "noType";
			}
			else return "int";
		}
	}
	else if ((t->tokens).front() == "ID") {
		string idName = givenName(t);
		return (symTable[function].second)[idName];
	}
	else if ((t->tokens).front() == "NUM") return "int";
	else if ((t->tokens).front() == "NULL") return "int*";
	else if (t->rule == "dcl type ID") {
		list<Tree*>::iterator type = (t->children).begin();
		if ((*type)->rule == "type INT STAR") {
			return "int*";
		}
		else { return "int";}
	}
	else {
		error = true; return "noType";
	}
}

int push(string reg) {
	cout << "sw " << reg << ", -4($30)" << endl;
	cout << "sub $30, $30, $4" << endl;
	int position = offset;
	offset -= 4;
	return position;
}

void pop(string reg, int pos) {
	cout << "lw " << reg << ", " << pos << "($29)" << endl;
	cout << "add $30, $30, $4" << endl;
	offset += 4;
}
		
void recordTable(Tree *t) {
	if (t->rule == "dcls dcls dcl BECOMES NUM SEMI") {
		list<Tree*>::iterator dcl = (t->children).begin();
		recordTable(*dcl);	// START
		if (error == true) return;	// END
		string dclName = getExpression(*(++dcl));
		recordTable(*dcl);	// START 2 // END 2
		list<Tree*>::iterator realName;
		realName = ((*dcl)->children).begin();
		realName++;
		string name = givenName(*realName);
		if (error == true) return;
		else if (dclName != "int") {
			error = true;
			cerr << " ERROR: DECLA WRONG TYPE" << endl;
			return;
		}
		dcl++; dcl++;
		stringstream vin(givenName(*dcl));
		int realValue;
		vin >> realValue;
		cout << "lis $3" << endl;
		cout << ".word " << realValue << endl;
		cout << "sw $3, -4($30)" << endl;
		cout << "sub $30, $30, $4" << endl;
		setTable[function][name] = offset;
		offset -= 4;
	}
	else if (t->rule == "dcls dcls dcl BECOMES NULL SEMI") {
		list<Tree*>::iterator dcl = (t->children).begin();
		recordTable(*dcl);	// START
		if (error == true) return;	// END
		string dclName = getExpression(*(++dcl));
		recordTable(*dcl);	// START 2 // END 2
		list<Tree*>::iterator realName;
		realName = ((*dcl)->children).begin();
		realName++;
		string name = givenName(*realName);
		if (error == true) return;
		else if (dclName != "int*") {
			error = true;
			cerr << " ERROR: DECLA WRONG TYPE NULL" << endl;
			return;
		}
		cout << "add $3, $11, $0" << endl;
		cout << "sw $3, -4($30)" << endl;
		cout << "sub $30, $30, $4" << endl;
		setTable[function][name] = offset;
		offset -= 4;
	}
	else if (t->rule == "params") return;
	else if ((t->tokens).front() == "params") {
		list<Tree*>::iterator paraLst = (t->children).begin();
		recordTable(*paraLst);
		if (error == true) return;
	}
	else if ((t->tokens).front() == "paramlist") {
		for (list<Tree*>::iterator paraLst = (t->children).begin(); paraLst != (t->children).end(); paraLst++) {
			if (((*paraLst)->tokens).front() == "dcl") recordTable(*paraLst);
			else if (((*paraLst)->tokens).front() == "paramlist") recordTable(*paraLst);
		}
	}
	else if (t->rule == "dcls") return;
	// new if conditional
	else if (t->rule == "dcl type ID") {
		list<Tree*>::iterator type, id;
		type = (t->children).begin();
		id = type;
		id++;
		string name = givenName(*id);
		if ((symTable[function].second).find(name) != (symTable[function].second).end()) {
			cerr << " ERROR: DOUBLE DEFINED THE VAR!" << endl;
			error = true;
			return;
		}
		// ADDED BECAUSE OF MIPS
		// END ADD
		(symTable[function].second)[name] = givenType(*type);
		cerr << name << " ";
		cerr << (symTable[function].second)[name] << endl;
		// MIPS STARTED
		if (function == "wain") {
			if ((symTable["wain"].second).size() == 1) {
				// MIPS CODE
				cout << "sw $1, -4($30)" << endl;
				cout << "sub $30, $30, $4" << endl;
				setTable["wain"][name] = offset;
				offset -= 4;
				// MIPS CODE
			}
			else if ((symTable["wain"].second).size() == 2) {
				// MIPS CODE
				cout << "sw $2, -4($30)" << endl;
				cout << "sub $30, $30, $4" << endl;
				setTable["wain"][name] = offset;
				offset -= 4;
				// MIPS CODE
				// init read
				if ((symTable["wain"].first)[0] == "int") cout << "add $2, $0, $0" << endl;
				int posReturn = push("$31");
				cout << "lis $5" << endl;
				cout << ".word init" << endl;
				cout << "jalr $5" << endl;
				if ((symTable["wain"].first)[0] == "int") cout << "lw $2, -4($29)" << endl;
				pop("$31", posReturn);
			}
		}
		// MIPS ENDED
	}
}

void subCode(Tree *t);
string tempStore = "N/A";

void code(Tree *t) {
	if (t->rule == "expr expr PLUS term" || t->rule == "expr expr MINUS term") {
		list<Tree*>::iterator expr = (t->children).begin();
		list<Tree*>::iterator term = (t->children).begin();
		term++; term++;
		string lhs = getExpression(*expr);
		string rhs = getExpression(*term);
		if (lhs == rhs && lhs == "int") {
			code(*expr);
			int pos = push("$3");
//			cout << "hello!!!!!!!!!!!!!!!!!!!!!" << pos << endl;
			code(*term);
//			cout << "hello!!!!!!!!!!!!!!!!!!!!!" << pos << endl;
			pop("$5", pos);			
			if (t->rule == "expr expr PLUS term")	cout << "add $3, $3, $5" << endl;
			else cout << "sub $3, $5, $3" << endl;
		}
		else if (t->rule == "expr expr PLUS term" && lhs == "int*") {
			code(*expr);
			int pos = push("$3");
			code(*term);
			cout << "mult $3, $4" << endl;
			cout << "mflo $3" << endl;
			pop("$5", pos);
			cout << "add $3, $3, $5" << endl;
		}
		else if (t->rule == "expr expr PLUS term" && rhs == "int*") {
			code(*expr);
			cout << "mult $3, $4" << endl;
			cout << "mflo $3" << endl;
			int pos = push("$3");
			code(*term);
			pop("$5", pos);
			cout << "add $3, $3, $5" << endl;
		}
		else if (t->rule == "expr expr MINUS term" && lhs == "int*" && rhs == "int") {
			code(*expr);
			int pos = push("$3");
			code(*term);			
			cout << "mult $3, $4" << endl;
			cout << "mflo $3" << endl;
			pop("$5", pos);
			cout << "sub $3, $5, $3" << endl;
		}
		else if (t->rule == "expr expr MINUS term" && lhs == "int*" && rhs == "int*") {
			code(*expr);
			int pos = push("$3");
			code(*term);
			pop("$5", pos);
			cout << "sub $3, $5, $3" << endl;
			cout << "div $3, $4" << endl;
			cout << "mflo $3" << endl;
		}
	}
	else if (t->rule == "term term STAR factor" || t->rule == "term term SLASH factor" ||
		 t->rule == "term term PCT factor") {
		list<Tree*>::iterator term = (t->children).begin();
		code(*term);
		int pos = push("$3");
		list<Tree*>::iterator factor = (t->children).begin();
		factor++; factor++;
		code(*factor);
		pop("$5", pos);
		if (t->rule == "term term STAR factor") {
			cout << "mult $3, $5" << endl;
			cout << "mflo $3" << endl;
		}
		else {
			cout << "div $5, $3" << endl;
			if (t->rule == "term term SLASH factor") cout << "mflo $3" << endl;
			else cout << "mfhi $3" << endl;
		}
	}
	else if (t->rule == "factor ID LPAREN RPAREN") {
		list<Tree*>::iterator child = (t->children).begin();
		string fnName = givenName(*child);
		int pos_29 = push("$29");
		int pos_31 = push("$31");
		cout << "lis $5" << endl;
		cout << ".word F" << fnName << endl;
		cout << "jalr $5" << endl;
		pop("$31", 4);
		pop("$29", 8);
	}
	else if ((t->rule == "arglist expr") || (t->rule == "arglist expr COMMA arglist")) {		//expr Check
		list<Tree*>::iterator child = (t->children).begin();
		if (error == true) return;
		code(*child);
		int posToken = push("$3");
		int size = paraTable[tempStore].size();
		string id = paraTable[tempStore][paraCount];
		paraCount++;
//		cerr << paraCount << endl; // JUST CHECK FOR PARAMETERS COUNT
//		setTable[tempStore][id] = posToken + size * 4;
//		cerr << setTable[tempStore][id] << " " << size << endl; // JUST CHECK FOR OFFSET
		if (t->rule == "arglist expr COMMA arglist") {
			child++; child++;
			code(*child);
		}
	}
	else if (t->rule == "factor ID LPAREN arglist RPAREN") {// TASEKEDEI!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (error == true) return;
		paraCount = 0;		
		list<Tree*>::iterator child = (t->children).begin();
		string fnName = givenName(*child);
		tempStore = fnName;
//		cerr << function << endl; // CHECK FOR FUNCTION NAME
		int pos_29 = push("$29");
		int pos_31 = push("$31");
		child++; child++;
		int tempOffset = offset;
		offset = 0;
		code(*child);
		offset = tempOffset;
		cout << "lis $5" << endl;
		cout << ".word F" << fnName << endl;
		cout << "jalr $5" << endl;		
		int size = (symTable[tempStore].first).size();
		for (int i = 0; i < size; i++) {
			cout << "add $30, $30, $4" << endl;
//			offset += 4;
		}
		pop("$31", 4 + size * 4);
		pop("$29", 8 + size * 4);
		tempStore = "N/A";
	}
	else if (t->rule == "test expr LT expr") {
		list<Tree*>::iterator expr = (t->children).begin();
		code(*expr);
		expr++; expr++;
		cout << "add $6, $3, $0" << endl;
		code(*expr);
		string check = getExpression(*expr);
		if (check == "int") cout << "slt $3, $6, $3" << endl;
		else if (check == "int*") cout << "sltu $3, $6, $3" << endl;
	}
	else if (t->rule == "test expr GT expr") {
		list<Tree*>::iterator expr = (t->children).begin();
		code(*expr);
		expr++; expr++;
		cout << "add $6, $3, $0" << endl;
		code(*expr);
		string check = getExpression(*expr);
		if (check == "int") cout << "slt $3, $3, $6" << endl;
		else if (check == "int*") cout << "sltu $3, $3, $6" << endl;
	}
	else if (t->rule == "test expr NE expr") {
		list<Tree*>::iterator expr = (t->children).begin();
		code(*expr);
		expr++; expr++;
		cout << "add $6, $3, $0" << endl;
		code(*expr);
		string check = getExpression(*expr);
		if (check == "int") {
			cout << "slt $7, $3, $6" << endl;
			cout << "slt $8, $6, $3" << endl;
		}
		else if (check == "int*") {
			cout << "sltu $7, $3, $6" << endl;
			cout << "sltu $8, $6, $3" << endl;
		}
		cout << "add $3, $7, $8" << endl;
	}
	else if (t->rule == "test expr EQ expr") {
		list<Tree*>::iterator expr = (t->children).begin();
		code(*expr);
		expr++; expr++;
		cout << "add $6, $3, $0" << endl;
		code(*expr);
		string check = getExpression(*expr);
		if (check == "int") {
			cout << "slt $7, $3, $6" << endl;
			cout << "slt $8, $6, $3" << endl;
		}
		else if (check == "int*") {
			cout << "sltu $7, $3, $6" << endl;
			cout << "sltu $8, $6, $3" << endl;
		}
		cout << "add $3, $7, $8" << endl;
		cout << "sub $3, $11, $3" << endl;
	}
	else if (t->rule == "test expr LE expr") {
		list<Tree*>::iterator expr = (t->children).begin();
		code(*expr);
		expr++; expr++;
		cout << "add $6, $3, $0" << endl;
		code(*expr);
		string check = getExpression(*expr);
		string slt = "slt";
		if (check == "int*") slt = "sltu";
		cout << slt << " $7, $3, $6" << endl;
		cout << slt << " $8, $6, $3" << endl;
		cout << "add $7, $7, $8" << endl;
		cout << "sub $7, $11, $7" << endl;
		cout << slt << " $3, $6, $3" << endl;
		cout << "add $3, $3, $7" << endl;
	}
	else if (t->rule == "test expr GE expr") {
		list<Tree*>::iterator expr = (t->children).begin();
		code(*expr);
		expr++; expr++;
		cout << "add $6, $3, $0" << endl;
		code(*expr);
		string check = getExpression(*expr);
		string slt = "slt";
		if (check == "int*") slt = "sltu";
		cout << slt << " $7, $3, $6" << endl;
		cout << slt << " $8, $6, $3" << endl;
		cout << "add $7, $7, $8" << endl;
		cout << "sub $7, $11, $7" << endl;
		cout << slt << " $3, $3, $6" << endl;
		cout << "add $3, $3, $7" << endl;		
	}
	else if ((t->rule == "expr term") || (t->rule == "term factor")) {
		list<Tree*>::iterator next = (t->children).begin();
		code(*next);
	}
	else if (t->rule == "factor NEW INT LBRACK expr RBRACK") {
		list<Tree*>::iterator expr = (t->children).begin();
		expr++; expr++; expr++;
		code(*expr);
		int skipX = X;
		X++;
		int onePos = push("$1");
		cout << "add $1, $3, $0" << endl;
		int rePos = push("$31");
//		cerr << ((*expr)->tokens).front() << endl;
		cout << "lis $5" << endl;
		cout << ".word new" << endl;
		cout << "jalr $5" << endl;
		pop("$31", rePos);
		pop("$1", onePos);
		cout << "bne $3, $0, skip" << skipX << endl;
		cout << "add $3, $11, $0" << endl;
		cout << "skip" << skipX << ":" << endl;
	}
	else if (t->rule == "factor LPAREN expr RPAREN") {
		list<Tree*>::iterator expr = (t->children).begin();
		code(*(++expr));
	}
	else if (t->rule == "factor NUM") {
		list<Tree*>::iterator value = (t->children).begin();
		stringstream vin(givenName(*value));
		int realValue;
		vin >> realValue;
		cout << "lis $3" << endl;
		cout << ".word " << realValue << endl;
	}
	else if (t->rule == "factor STAR factor") {
		list<Tree*>::iterator addr = (t->children).begin();
		addr++;
		code(*addr);
		cout << "lw $3, 0($3)" << endl;
	}
	else if (t->rule == "factor AMP lvalue") {
		list<Tree*>::iterator value = (t->children).begin();
		value++;
		subCode(*value);
	}
	else if (t->rule == "factor NULL") {
		cout << "add $3, $11, $0" << endl;
	}
	else if (t->rule == "factor ID") { // will be modified a lot		
		list<Tree*>::iterator idPos = (t->children).begin();
		string chosenName = givenName(*idPos);
		int offsetChosen = setTable[function][chosenName];
		cout << "lw $3, " << offsetChosen << "($29)" << endl;
	}
}

		
void subCode(Tree *t) {
	if (t->rule == "lvalue LPAREN lvalue RPAREN") {
		list<Tree*>::iterator lVal = (t->children).begin();
		lVal++;
		subCode(*lVal);
	}
	else if (t->rule == "lvalue STAR factor") {
		list<Tree*>::iterator subVal = (t->children).begin();
		subVal++;
		code(*subVal);
	}
	else if (t->rule == "lvalue ID") {
		cout << "lis $3" << endl;
		list<Tree*>::iterator idPos = (t->children).begin();
		string chosenName = givenName(*idPos);
		int offsetChosen = setTable[function][chosenName];
		cout << ".word " << offsetChosen << endl;
		cout << "add $3, $29, $3" << endl;
	}
}


void getChange (Tree *t, Tree *expr) {
	if (t->rule == "lvalue ID") {
		list<Tree*>::iterator lname = (t->children).begin();
		string name;
		name = givenName(*lname);
		code(expr);
		int position = setTable[function][name];
		cout << "sw $3, " << position << "($29)" << endl;
	}
	else if (t->rule == "lvalue LPAREN lvalue RPAREN") {
		list<Tree*>::iterator lname = (t->children).begin();
		lname++;
		getChange(*lname, expr);
	}
	else if (t->rule == "lvalue STAR factor") {
		list<Tree*>::iterator lname = (t->children).begin();
		lname++;
		code(*lname);
		int pos = push("$3");
		code(expr);
		pop("$5", pos);
		cout << "sw $3, 0($5)" << endl;
	}
}


void stmtRead(Tree *t) {
	if (t->rule == "statements") return;
	else if (t->rule == "statements statements statement") {
		list<Tree*>::iterator it = (t->children).begin();
		stmtRead(*it);
		stmtRead(*(++it));
		if (error == true) return;
	}
	else if (t->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
		for (list<Tree*>::iterator it = (t->children).begin(); it != (t->children).end(); it++) {
			if (((*it)->tokens).front() == "test") {
				int chose = X;
				X++;
				stmtRead(*it);
				cout << "bne $3, $11, else" << chose << endl;
				it++; it++; it++;
				stmtRead(*it);
				cout << "beq $0, $0, endif" << chose << endl;
				cout << "else" << chose << ":" << endl;
				it++; it++; it++; it++;
				stmtRead(*it);
				cout << "endif" << chose << ":" << endl;
				break;
			}
			if (error == true) return;
		}
	}
	else if (t->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
		for (list<Tree*>::iterator it = (t->children).begin(); it != (t->children).end(); it++) {
			if (((*it)->tokens).front() == "test") {
				int chose = Y;
				Y++;
				cout << "loop" << chose << ":" << endl;
				stmtRead(*it);
				cout << "beq $3, $0, done" << chose << endl;
				it++; it++; it++;
				stmtRead(*it);
				cout << "beq $0, $0, loop" << chose << endl;
				cout << "done" << chose << ":" << endl;
				break;
			}
			if (error == true) return;
		}
	}
	else if (t->rule == "statement lvalue BECOMES expr SEMI") {
		list<Tree*>::iterator expr = (t->children).begin();
		string lhs = getExpression(*expr);
		expr++; expr++;
		string rhs = getExpression(*expr);
		if (error == true) return;
		if (lhs != rhs) {
			error = true;
			cerr << " ERROR: ASSIGNMENT IS NOT RIGHT" << endl;
			return;
		}
		string name;
		list<Tree*>::iterator lvalue = (t->children).begin();
		getChange(*lvalue, *expr);	
	}
	else if ((t->tokens).front() == "test") {
		list<Tree*>::iterator ele = (t->children).begin();
		Tree *lhs = (*ele);
		++ele;
		Tree *rhs = *(++ele);
		string lType = getExpression(lhs);
		string rType = getExpression(rhs);
		if (error == true) return;
		if (lType != rType) {
			error = true;
			cerr << " ERROR: DIFFERENT TYPE WHEN BOOLEAN" << endl;
			return;
		}
		code(t);
	}
	else if (t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		list<Tree*>::iterator expr = (t->children).begin();
		expr++; expr++;
		string eType = getExpression(*expr);
		// MIPS CODES STARTED HERE
		code(*expr);
		int posOne = push("$1");
		cout << "add $1, $3, $0" << endl;
		int posReturn = push("$31");
		cout << "lis $5" << endl;
		cout << ".word print" << endl;
		cout << "jalr $5" << endl;
		pop("$31", posReturn);
		pop("$1", posOne);
		// MIPS CODES END
		if (error == true) return;
		if (eType != "int") {
			error == true;
			cerr << " ERROR: WRONG OUTPUT OF PRINTLN" << endl;
			return;
		}
	}
	else if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
		int recordX = X;
		X++;
		list<Tree*>::iterator expr = (t->children).begin();
		expr++; expr++; expr++;
		string eType = getExpression(*expr);
		if (error == true) return;
		if (eType != "int*") {
			error == true;
			cerr << " ERROR: WRONG OUTPUT OF DELETE" << endl;
		}
		code(*expr);
//		cerr << ((*expr)->tokens).front() << endl;
		cout << "beq $3, $1, skip" << recordX << endl;
		int onePos = push("$1");
		cout << "add $1, $3, $0" << endl;
		int rePos = push("$31");
		cout << "lis $5" << endl;
		cout << ".word delete" << endl;
		cout << "jalr $5" << endl;
		pop("$31", rePos);
		pop("$1", onePos);
		cout << "skip" << recordX << ":" << endl;
	}
}

void print(Tree *t) {
	if (error == true) return;
	if ((t->tokens).front() == "main") {
		funName.insert("wain");
		if (function != "N/A") {
			function = "wain";
			cerr << endl;
		}
		else function = "wain";
		// MIPS CODE
		offset = 0;
		cout << "wain:" << endl;
		// MIPS CODE
		cerr << function;
		for (list<Tree*>::iterator lstT = (t->children).begin(); lstT != (t->children).end(); lstT++) {
			if (((*lstT)->tokens).front() == "dcl") {
				readWainParams(*lstT);
			}
			if ((symTable["wain"].first).size() == 2) {
				if ((symTable["wain"].first)[1] != "int") {
					error = true;
					cerr << "\n ERROR: WRONG SECOND TYPE FOR WAIN" << endl;
					return;
				}
				break;
			}
		}
		cerr << "\n";
		for (list<Tree*>::iterator lstT = (t->children).begin(); lstT != (t->children).end(); lstT++) {
			if (error == true) return;
			if (((*lstT)->tokens).front() == "dcls" || ((*lstT)->tokens).front() == "dcl") {
				recordTable(*lstT);
			}
			else if (((*lstT)->tokens).front() == "statements") {
					stmtRead(*lstT);
					if (error == true) return;
			}
			else if (((*lstT)->tokens).front() == "expr") {
				string check = getExpression(*lstT);
				if (error == true) return;
				else if (check != "int") {
					cerr << " ERROR: RETURN TYPE IS NOT RIGHT!" << endl;
				}
				else {
					code(*lstT);
					cout << "add $30, $29, $4" << endl;
					offset = 0;
					cout << "jr $31" << endl;
				// temp MIPS CODE
					break;
				}
			}
		}
		return; // after main, always return
	}
	else if ((t->tokens).front() == "procedure") {
		int offsetZero = offset;
		offset = 0;
		list<Tree*>::iterator id = (t->children).begin();
		id++;
		string name = givenName(*id);
		if (funName.count(name) != 0) {
			cerr << " ERROR: DOUBLE DEFINED THE FUNCTION!" << endl;
			error = true;
			return;
		}
		if (function != "N/A") cerr << endl;
		function = name;
		cout << "F" << name << ":" << endl;
		cout << "sub $29, $30, $4" << endl;		
		int pos_1 = push("$1");
		int pos_2 = push("$2");
		int pos_5 = push("$5");
		int pos_6 = push("$6");
		int pos_7 = push("$7");
		int pos_8 = push("$8");
		funName.insert(function);
		cerr << function;
		for (list<Tree*>::iterator lstT = (t->children).begin(); lstT != (t->children).end(); lstT++) {
			if (error == true) return;
			if (((*lstT)->tokens).front() == "dcls" || ((*lstT)->tokens).front() == "dcl")
				recordTable(*lstT);
			else if (((*lstT)->tokens).front() == "params") {
				if (((*lstT)->tokens).size() > 1) {
					readParams(*lstT);
				}
				for (vector<string>::iterator typeList = (symTable[function].first).begin();
					typeList != (symTable[function].first).end(); typeList++) {
					cerr << " " << (*typeList);
				}
				int size = paraTable[function].size();
				for (int i = 0; i < size; i++) {
					string tmpIdName = paraTable[function][i];
					int position = -4 * i;
					setTable[function][tmpIdName] = position + size * 4;
				}					
				cerr << endl;
				recordTable(*lstT);
			}
			else if (((*lstT)->tokens).front() == "statements") {
					stmtRead(*lstT);
					if (error == true) return;
			}
			else if (((*lstT)->tokens).front() == "expr") {
				string check = getExpression(*lstT);
				if (error == true) return;
				if (check != "int") {
					cerr << " ERROR: RETURN TYPE IS NOT RIGHT!" << endl;
				}
				else {
					code(*lstT);
					pop("$8", pos_8);
					pop("$7", pos_7);
					pop("$6", pos_6);
					pop("$5", pos_5);
					pop("$2", pos_2);
					pop("$1", pos_1);
					cout << "add $30, $29, $4" << endl;
					cout << "jr $31" << endl;
					offset = offsetZero;
					break;
				}
			}
		}
	}
	for(list<Tree*>::iterator it=(t->children).begin(); it != (t->children).end(); it++) {  // print all subtrees
        	print(*it);
		if (error == true) return;
    	}
}

int main(){
    readsyms(terms); // read terminals
    readsyms(nonterms); // read nonterminals
    getline(fin,start); // read start symbol
//    readsyms(prods); // read production rules
    
    Tree *parsetree = new Tree();
	readInTree(parsetree);
		cout << ".import print" << endl;
		cout << ".import init" << endl;
		cout << ".import new" << endl;
		cout << ".import delete" << endl;		
		cout << "lis $4" << endl;
		cout << ".word 4" << endl;
		cout << "lis $11" << endl;
		cout << ".word 1" << endl;
		cout << "sub $29, $30, $4" << endl;
		cout << "beq $0, $0, wain" << endl;
print(parsetree);	
//	symTable(parsetree);
    delete parsetree;
}
