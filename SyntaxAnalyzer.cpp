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

    void single_statement() {
        if (look_ahead.first == "number" || look_ahead.first == "id") {
            if(look_ahead.first == "id" && tokens[currentTokenIndex + 1].first == "(") {
                function_call();
            } else {
                expression();
            }
        } else if (look_ahead.first == "if" || look_ahead.first == "switch") {
            conditional_statements();
        } else if(look_ahead.first == "return"){
            return_statement();
        } else {
            throw std::runtime_error("Unexpected token in single_statement: " + look_ahead.first);
        }
    }

    void parse() {
        while (currentTokenIndex < tokens.size()) {
            if(look_ahead.first == "Success" && look_ahead.second == "Tokens are finished"){
            cout << "Parsing completed successfully" << endl;
            return;
            }
            single_statement();
        }
    }

    void expression() {
        string s = tokens[currentTokenIndex + 1].first;
        if(look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||"){
            boolean_expr();
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            boolean_expr();
        } else if (s == "=" || s == "+=" || s == "-=" || s == "*=" || s == "/=" || s == "%=" || s == "&=" || s == "|=" || s == "^=" || s == "<<=" || s == ">>="){
            assignment_expr();
        } else{
            arithmetic_expr();
        }
    }

    void arithmetic_expr() {
        //arithmetic_expr -> arithmetic_expr arithmetic_op arithmetic_expr; | variable
        variable();
        if (look_ahead.first == "id") {
            variable();
        }
        arithmetic_op();
        arithmetic_expr();
        match(";");
        
    }

    void arithmetic_op() {
        // Arithmetic_op -> '+' | '-' | '*' | '/' | '%' | '&' | '|' | '^' | '<<' | '>>'
        if (look_ahead.first == "+" || look_ahead.first == "-" || look_ahead.first == "*" || look_ahead.first == "/" || look_ahead.first == "%" || look_ahead.first == "&" || look_ahead.first == "|" || look_ahead.first == "^" || look_ahead.first == "<<" || look_ahead.first == ">>") {
            try{
                match(look_ahead.first);
            } catch (exception e){
                cout << e.what() << endl;
            }
        }
    }

    void boolean_expr() {
        // Boolean_expr -> variable boolean_op variable | boolean_op variable
        if (look_ahead.first == "id" || look_ahead.first == "number") {
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
            try{
                match(look_ahead.first);
            }catch (exception e){
                cout << e.what() << endl;
            }
        }
    }

    void assignment_expr() {
        // assignment_expr -> variable assignment_op variable | variable assignment_op arithmetic_expr | variable assignment_op boolean_expr
        variable();
        assignment_op();
        string s = tokens[currentTokenIndex + 1].first;
        if(look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||"){
            boolean_expr();
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            boolean_expr();
        } else if(look_ahead.first == "id" || look_ahead.first == "number"){
            variable();
        } else{
            arithmetic_expr();
        }
        match(";");
    }

    void assignment_op() {
        // Assignment_op -> '='| '+=' | '-=' | '*=' | '/=' | '%='| '&='| '|='| '^='| '<<=' | '>>='
        if (look_ahead.first == "=" || look_ahead.first == "+=" || look_ahead.first == "-=" || look_ahead.first == "*=" || look_ahead.first == "/=" || look_ahead.first == "%=" || look_ahead.first == "&=" || look_ahead.first == "|=" || look_ahead.first == "^=" || look_ahead.first == "<<=" || look_ahead.first == ">>=") {
            try{
                match(look_ahead.first);
            } catch (exception e){
                cout << e.what() << endl;
            }
        }
    }

    void variable() {
        // Variable -> number | id
        if (look_ahead.first == "number") {
            try{
                match("number");
            } catch (exception e){
                cout << e.what() << endl;
            }
        } else if (look_ahead.first == "id") {
            try{
                match("id");
            } catch (exception e){
                cout << e.what() << endl;
            }
        }
    }

    void function_call() {
        // function_call -> id ( arguments ) ;
        if (look_ahead.first == "id") {
            try {
                match("id");
                match("(");
                arguments();
                match(")");
                match(";");
            } catch (exception e) {
                cout << e.what() << endl;
            }
        }
    }

    void arguments() {
        // arguments -> arg_expression | arg_expression , arguments | ε
        if (look_ahead.first == "id" || look_ahead.first == "number" || look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            arg_expression();
            if (look_ahead.first == ",") {
                match(",");
                arguments();
            }
        } else {
            return;
        }
    }

    void arg_expression() {
        // arg_expression -> arithmetic_expr | boolean_expr
        string s = tokens[currentTokenIndex + 1].first;
        if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            boolean_expr();
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            boolean_expr();
        } else if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){ 
            arithmetic_expr();
        } else{
            variable();
        }
    }

    void return_statement() {
        // return_statement  -> return return_expr ;
        match("return");
        return_expr();
        match(";");
    }

    void return_expr() {
        // return_expr -> arithmetic_expr | boolean_expr | 1 | 0 | ε
        string s = tokens[currentTokenIndex + 1].first;
        if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            boolean_expr();
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            boolean_expr();
        } else if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){ 
            arithmetic_expr();
        } else if(look_ahead.first == "1" || look_ahead.first == "0"){
            match(look_ahead.first);
        } else{
            variable();
        }
    }

    void conditional_statements() {
        // Conditional_statements -> if_expr | switch_expr
        if (look_ahead.first == "if") {
            if_expr();
        } else if (look_ahead.first == "switch") {
            switch_expr();
        }
    }

    void body() {
        // body -> single_statement body | ε
        if (look_ahead.first == "single_statement") {
            single_statement();
            body();
        } else{
            return;
        }
    }

    void if_expr() {
        // if_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
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
