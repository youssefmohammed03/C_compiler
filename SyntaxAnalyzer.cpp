#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
using namespace std;

class SyntaxAnalyzer {

public:
vector<pair<string, string>> tokens;
int currentTokenIndex;

public:
    SyntaxAnalyzer(vector<pair<string, string>> tokens) {
        this->tokens = tokens;
        this->currentTokenIndex = 0;
    }

    pair<string, string> getNextToken() {
        if (currentTokenIndex < tokens.size()) {
            return tokens[currentTokenIndex++];
        } else {
            return make_pair("ERROR", "Tokens are finished");
        }
    }

    void parse() {
        program();
    }

    void program() {
        while (currentTokenIndex < tokens.size()) {
            statement();
        }
    }

    void statement() {
        // Implement according to your grammar
        // This is just a placeholder
        // You can replace this with your actual implementation
        // For example, if your grammar is: statement -> assignment_expression | arithmetic_expression | boolean_expression | function_call | conditional_statements
        assignment_expression();
        arithmetic_expression();
        boolean_expression();
        function_call();
        conditional_statements();
    }

    void assignment_expression() {                              
        // Implement according to your grammar
        // ASSIGNMENT EXPRESSION -> variable  AssignmentOp variable
        // AssignmentOp -> '='| '+=' | '-=' | '*=' | '/=' | '%='| '&='| '|='| '^='| '<<=' | '>>='|
        // variable -> NUMBER | ID | ε
        // NUMBER -> DIGIT+ 
        // DIGIT -> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
        // ID -> [a-zA-Z_][a-zA-Z0-9_]*
    }

    void arithmetic_expression() {
        // Implement according to your grammar
        // ARITHMETIC EXPRESSION -> ARITHMETIC EXPRESSION  ArithmeticOp ARITHMETIC EXPRESSION | variable
        // ArithmeticOp ->  '+' | '-' | '*' | '/' | '%' | '&' | '|' | '^' | '<<' | '>>’
    }

    void boolean_expression() {
        // Implement according to your grammar
        // BOOLEAN EXPRESSION -> variable BooleanOp variable
        // BooleanOp ->   '==' | '!=' | '>' | '<' | '>=' | '<=' | '!' | '&&' | '||'
    }

    void function_call() {
        // Implement according to your grammar
        // FUNCTION CALL -> ID ‘(‘ ARGUMENTS ‘)’ ‘;’
        // ARGUMENTS -> ARGEXPRESSION | ARGEXPRESSION ‘,’ ARGUMENTS | ε
        // ARGEXPRESSION -> ARITHMETIC EXPRESSION | BOOLEAN EXPRESSION
    }

    void conditional_statements() {
        // Implement according to your grammar
        // Conditional_statements -> if_expr | switch_expr
        // If_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
        // Else_expr -> else {body} | else if (boolean_expr) {body} | else_expr |else single_ statement | else if (boolean_expr) single_statement | ε
        // Switch_expr -> switch(ID) { case_expr default_expr } | switch(ID) {body case_expr default_expr}
        // Case_expr -> case const : body break; | case const : body | case_expr | ε
        // Default_expr -> default: body | default: body break;
        // Const -> NUMBER | string | char
        // Expression -> boolean_expr | arth_expr | assignment_expr
        // Body -> single_statement | body
        // Single_statement -> expression | conditional_statements | iterative_statements | function_calls
    }


};