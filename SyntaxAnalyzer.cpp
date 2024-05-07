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
        match(look_ahead.first);
        sub_arithmetic_expr ();
    }

    void sub_arithmetic_expr () {
        if (look_ahead.first == "+" || look_ahead.first == "-" || look_ahead.first == "*" || look_ahead.first == "/" || look_ahead.first == "%" || look_ahead.first == "&" || look_ahead.first == "|" || look_ahead.first == "^" || look_ahead.first == "<<" || look_ahead.first == ">>") {
            arithmetic_op();
            arithmetic_expr();
        }
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
        } else{
            if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){
                arithmetic_expr();
            } else{
                variable();
            }
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
        // conditional_statements -> if_expr | switch_expr
        if (look_ahead.first == "if") {
            if_expr();
        } else if (look_ahead.first == "switch") {
            switch_expr();
        }
    }

    void body() {
        //body -> single_statement sub_body
        single_statement();
        sub_body();
    }

    void sub_body() {
        // sub_body -> body | ε
        //int | float | double | char | String | long | short | signed | unsigned
        if (look_ahead.first == "if" || look_ahead.first == "while" || look_ahead.first == "for" || look_ahead.first == "switch" || look_ahead.first == "return" || look_ahead.first == "id" || look_ahead.first == "do" || look_ahead.first == "enum" || look_ahead.first == "struct" || look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "String" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned") {
            body();
        }
    }

    void if_expr() {
        // if_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
        match("if");
        match("(");
        string s = tokens[currentTokenIndex + 1].first;
        if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            boolean_expr();
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            boolean_expr();
        } else{
            variable();
        }
        match(")");
        if (look_ahead.first == "{") {
            match("{");
            body();
            match("}");
        } else {
            single_statement();
        }
        else_expr();
    }

    void else_expr() {
        // else_expr -> else {body} | else single_statement | ε
        if (look_ahead.first == "else") {
            match("else");
            if (look_ahead.first == "{") {
                match("{");
                body();
                match("}");
            } else {
                single_statement();
            }
        }
    }

    void switch_expr() {
        // switch_expr -> switch(id) { case_expr default_expr }
        match("switch");
        match("(");
        match("id");
        match(")");
        match("{");
        if (look_ahead.first == "case") {
            case_expr();
        }
        if (look_ahead.first == "default") {
            default_expr();
        }
        match("}");
    }

    void case_expr() {
        // case_expr -> case const : body break; | case const : body | case_expr | ε
        while (look_ahead.first == "case") {
            match("case");
            match(look_ahead.first);
            match(":");
            body();
            if (look_ahead.first == "break") {
                match("break");
                match(";");
            }
        }
    }

    void default_expr() {
        // default_expr -> default: body | default: body break;
        if (look_ahead.first == "default") {
            match("default");
            match(":");
            body();
            if (look_ahead.first == "break") {
                match("break");
                match(";");
            }
        }
    }

    void const_expr() {
        // const -> number | string | char
        if (look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.second == "char") {
            match(look_ahead.first);
        }
    }

};
