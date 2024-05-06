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
pair<string, string> look_ahead;

public:
    SyntaxAnalyzer(vector<pair<string, string>> tokens) {
        this->tokens = tokens;
        this->currentTokenIndex = 0;
        this->look_ahead = tokens[currentTokenIndex];
    }

    pair<string, string> getNextToken() {
        if (currentTokenIndex < tokens.size() - 1) {
            return tokens[++currentTokenIndex];
        } else {
            return make_pair("Success", "Tokens are finished");
        }
    }

    void match(string expectedToken) {
        if (look_ahead.first == expectedToken) {
            look_ahead = getNextToken();
        } else{
            throw std::runtime_error("Unexpected token: " + look_ahead.first + ". Expected: " + expectedToken);
        }
    }

    void parse() {
        while (currentTokenIndex < tokens.size()) {
            if(look_ahead.first == "Success" && look_ahead.second == "Tokens are finished"){
            cout << "Parsing completed successfully" << endl;
            return;
            }
            arithmetic_expr();
            assignment_expr();
            boolean_expr();
        }
    }

    void arithmetic_expr() {
        if (look_ahead.first == "id") {
            variable();
        } else {
            arithmetic_expr();
            arithmetic_op();
            arithmetic_expr();
        }
    }

    void arithmetic_op() {
        // Arithmetic_op -> '+' | '-' | '*' | '/' | '%' | '&' | '|' | '^' | '<<' | '>>'
        if (look_ahead.first == "+" || look_ahead.first == "-" || look_ahead.first == "*" || look_ahead.first == "/" || look_ahead.first == "%" || look_ahead.first == "&" || look_ahead.first == "|" || look_ahead.first == "^" || look_ahead.first == "<<" || look_ahead.first == ">>") {
            look_ahead = getNextToken();
        }
    }

    void boolean_expr() {
        // Boolean_expr -> variable boolean_op variable | boolean_op variable
        if (isalpha(look_ahead.first[0]) || look_ahead.first[0] == '_' || look_ahead.first == "number") {
            variable();
            boolean_op();
            variable();
        } else {
            boolean_op();
            variable();
        }
    }

    void boolean_op() {
        // Boolean_op -> '==' | '!=' | '>' | '<' | '>=' | '<=' | '!' | '&&' | '||'
        if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            look_ahead = getNextToken();
        }
    }

    void assignment_expr() {
        // assignment_expr -> variable assignment_op variable | variable assignment_op arithmetic_expr | variable assignment_op boolean_expr
        variable();
        assignment_op();
        if (look_ahead.first == "number" || look_ahead.first == "id") {
            variable();
        } else if (look_ahead.first == "arithmetic_expr") {
            arithmetic_expr();
        } else if (look_ahead.first == "boolean_expr") {
            boolean_expr();
        }
    }

    void assignment_op() {
        // Assignment_op -> '='| '+=' | '-=' | '*=' | '/=' | '%='| '&='| '|='| '^='| '<<=' | '>>='
        if (look_ahead.first == "=" || look_ahead.first == "+=" || look_ahead.first == "-=" || look_ahead.first == "*=" || look_ahead.first == "/=" || look_ahead.first == "%=" || look_ahead.first == "&=" || look_ahead.first == "|=" || look_ahead.first == "^=" || look_ahead.first == "<<=" || look_ahead.first == ">>=") {
            look_ahead = getNextToken();
        }
    }

    void variable() {
        // Variable -> number | id
        if (look_ahead.first == "number") {
            look_ahead = getNextToken();
        } else if (look_ahead.first == "id") {
            look_ahead = getNextToken();
        }
    }

    void conditional_statements() {
        // Implement according to your grammar
        // Conditional_statements -> if_expr | switch_expr
        if (look_ahead.first == "if") {
            if_expr();
        } else if (look_ahead.first == "switch") {
            switch_expr();
        }
    }

    void if_expr() {
        // If_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
        match("if");
        match("(");
        boolean_expr();
        match(")");
        match("{");
        match("}");
    }

    void else_expr() {
        // Else_expr -> else {body} | else if (boolean_expr) {body} | else_expr |else single_ statement | else if (boolean_expr) single_statement | ε
        if (look_ahead.first == "else") {
            look_ahead = getNextToken();
            if (look_ahead.first == "if") {
                match("(");
                boolean_expr();
                match(")");
                look_ahead = getNextToken();
                if (look_ahead.first == "{") {
//                    body();
                    match("}");
                } else {
//                    single_statement();
                }
            } else if (look_ahead.first == "{") {
//                body();
                match("}");
            } else {
//                single_statement();
            }
        }
    }

    void switch_expr() {
        // Switch_expr -> switch(ID) { case_expr default_expr } | switch(ID) {body case_expr default_expr}
        match("switch");
        match("(");
        match("id");
        match(")");
        match("{");
        case_expr();
        default_expr();
        match("}");
    }

    void case_expr() {
        // Case_expr -> case const : body break; | case const : body | case_expr | ε
        while (look_ahead.first == "CASE") {
            match("CONST");
            match(":");
//            body();
            look_ahead = getNextToken();
            if (look_ahead.first == "BREAK") {
                match(";");
            }
            look_ahead = getNextToken();
        }
    }

    void default_expr() {
        // Default_expr -> default: body | default: body break;
        match("DEFAULT");
        match(":");
//        body();
        look_ahead = getNextToken();
        if (look_ahead.first == "BREAK") {
            match(";");
        }
    }


};
