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

    void match(string expectedToken) {
        pair<string, string> currentToken = getNextToken();
        if (currentToken.first != expectedToken) {
            throw std::runtime_error("Unexpected token: " + currentToken.first + ". Expected: " + expectedToken);
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
        pair<string, string> token = getNextToken();
        if (token.first == "if") {
            if_expr();
        } else if (token.first == "switch") {
            switch_expr();
        } else {
            // Error handling
            cout << "Error: Expected 'if' or 'switch', got " << token.first << endl;
        }
    }

    void if_expr() {
        // If_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
        match("IF");
        match("(");
        boolean_expression();
        match(")");
        pair<string, string> token = getNextToken();
        if (token.first == "{") {
            //body();
            match("}");
            else_expr();
        } else {
//            single_statement();
            else_expr();
        }
    }

    void else_expr() {
        // Else_expr -> else {body} | else if (boolean_expr) {body} | else_expr |else single_ statement | else if (boolean_expr) single_statement | ε
        pair<string, string> token = getNextToken();
        if (token.first == "ELSE") {
            token = getNextToken();
            if (token.first == "IF") {
                match("(");
                boolean_expression();
                match(")");
                token = getNextToken();
                if (token.first == "{") {
//                    body();
                    match("}");
                } else {
//                    single_statement();
                }
            } else if (token.first == "{") {
//                body();
                match("}");
            } else {
//                single_statement();
            }
        }
    }

    void switch_expr() {
        // Switch_expr -> switch(ID) { case_expr default_expr } | switch(ID) {body case_expr default_expr}
        match("SWITCH");
        match("(");
        match("ID");
        match(")");
        match("{");
        case_expr();
        default_expr();
        match("}");
    }

    void case_expr() {
        // Case_expr -> case const : body break; | case const : body | case_expr | ε
        pair<string, string> token = getNextToken();
        while (token.first == "CASE") {
            match("CONST");
            match(":");
//            body();
            token = getNextToken();
            if (token.first == "BREAK") {
                match(";");
            }
            token = getNextToken();
        }
    }

    void default_expr() {
        // Default_expr -> default: body | default: body break;
        match("DEFAULT");
        match(":");
//        body();
        pair<string, string> token = getNextToken();
        if (token.first == "BREAK") {
            match(";");
        }
    }


};
