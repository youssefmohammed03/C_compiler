#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include "ParseTree.cpp"
using namespace std;

class SyntaxAnalyzer {

public:
vector<pair<string, string>> tokens;
int currentTokenIndex;
pair<string, string> look_ahead;
ParseTreeNode *root;
ParseTreeNode *temp;

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

    void match(string expectedToken, ParseTreeNode *parent) {
        if (look_ahead.first == expectedToken) {
            temp = new ParseTreeNode(expectedToken);
            parent->addChild(temp);
            look_ahead = getNextToken();
        } else{
            throw std::runtime_error("Unexpected token: " + look_ahead.first + ". Expected: " + expectedToken);
        }
    }

    void single_statement(ParseTreeNode *parent) {
        if (look_ahead.first == "number" || look_ahead.first == "id") {
            if(look_ahead.first == "id" && tokens[currentTokenIndex + 1].first == "(") {
                temp = new ParseTreeNode("function_call");
                parent->addChild(temp);
                function_call(temp);
            } else {
                temp = new ParseTreeNode("expression");
                parent->addChild(temp);
                expression(temp);
            }
        } else if (look_ahead.first == "if" || look_ahead.first == "switch") {
            temp = new ParseTreeNode("conditional_statement");
            parent->addChild(temp);
            conditional_statements(temp);
        } else if(look_ahead.first == "return"){
            temp = new ParseTreeNode("return_statement");
            parent->addChild(temp);
            return_statement(temp);
        } else {
            throw std::runtime_error("Unexpected token in single_statement: " + look_ahead.first);
        }
    }

    void parse() {
        root = new ParseTreeNode("main");
        while (currentTokenIndex < tokens.size()) {
            if(look_ahead.first == "Success" && look_ahead.second == "Tokens are finished"){
                cout << "Parsing completed successfully" << endl;
                root->printTree();
                return;
            }
            single_statement(root);
        }
    }

    void expression(ParseTreeNode *parent) {
        string s = tokens[currentTokenIndex + 1].first;
        if(look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||"){
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else if (s == "=" || s == "+=" || s == "-=" || s == "*=" || s == "/=" || s == "%=" || s == "&=" || s == "|=" || s == "^=" || s == "<<=" || s == ">>="){
            temp = new ParseTreeNode("assignment_expr");
            parent->addChild(temp);
            assignment_expr(temp);
        } else{
            temp = new ParseTreeNode("arithmetic_expr");
            parent->addChild(temp);
            arithmetic_expr(temp);
        }
    }

    void arithmetic_expr(ParseTreeNode *parent) {
        match(look_ahead.first, parent);
        temp = new ParseTreeNode("sub_arithmetic_expr");
        parent->addChild(temp);
        sub_arithmetic_expr (temp);
    }

    void sub_arithmetic_expr (ParseTreeNode *parent) {
        if (look_ahead.first == "+" || look_ahead.first == "-" || look_ahead.first == "*" || look_ahead.first == "/" || look_ahead.first == "%" || look_ahead.first == "&" || look_ahead.first == "|" || look_ahead.first == "^" || look_ahead.first == "<<" || look_ahead.first == ">>") {
            temp = new ParseTreeNode("arithmetic_op");
            parent->addChild(temp);
            arithmetic_op(temp);
            temp = new ParseTreeNode("arithmetic_expr");
            parent->addChild(temp);
            arithmetic_expr(temp);
        }
    }

    void arithmetic_op(ParseTreeNode *parent) {
        // Arithmetic_op -> '+' | '-' | '*' | '/' | '%' | '&' | '|' | '^' | '<<' | '>>'
        if (look_ahead.first == "+" || look_ahead.first == "-" || look_ahead.first == "*" || look_ahead.first == "/" || look_ahead.first == "%" || look_ahead.first == "&" || look_ahead.first == "|" || look_ahead.first == "^" || look_ahead.first == "<<" || look_ahead.first == ">>") {
            try{
                match(look_ahead.first, parent);
            } catch (exception e){
                cout << e.what() << endl;
            }
        }
    }

    void boolean_expr(ParseTreeNode *parent) {
        // Boolean_expr -> variable boolean_op variable | boolean_op variable
        if (look_ahead.first == "id" || look_ahead.first == "number") {
            variable(parent);
            temp = new ParseTreeNode("boolean_op");
            parent->addChild(temp);
            boolean_op(temp);
            variable(parent);
        } else {
            temp = new ParseTreeNode("boolean_op");
            parent->addChild(temp);
            boolean_op(temp);
            variable(parent);
        }
    }

    void boolean_op(ParseTreeNode *parent) {
        // Boolean_op -> '==' | '!=' | '>' | '<' | '>=' | '<=' | '!' | '&&' | '||'
        if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            try{
                match(look_ahead.first, parent);
            }catch (exception e){
                cout << e.what() << endl;
            }
        }
    }

    void assignment_expr(ParseTreeNode *parent) {
        // assignment_expr -> variable assignment_op variable | variable assignment_op arithmetic_expr | variable assignment_op boolean_expr
        variable(parent);
        temp = new ParseTreeNode("assignment_op");
        parent->addChild(temp);
        assignment_op(temp);
        string s = tokens[currentTokenIndex + 1].first;
        if(look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||"){
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else{
            if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                arithmetic_expr(temp);
            } else{
                variable(parent);
            }
        }
        match(";", parent);
    }

    void assignment_op(ParseTreeNode *parent) {
        // Assignment_op -> '='| '+=' | '-=' | '*=' | '/=' | '%='| '&='| '|='| '^='| '<<=' | '>>='
        if (look_ahead.first == "=" || look_ahead.first == "+=" || look_ahead.first == "-=" || look_ahead.first == "*=" || look_ahead.first == "/=" || look_ahead.first == "%=" || look_ahead.first == "&=" || look_ahead.first == "|=" || look_ahead.first == "^=" || look_ahead.first == "<<=" || look_ahead.first == ">>=") {
            try{
                match(look_ahead.first, parent);
            } catch (exception e){
                cout << e.what() << endl;
            }
        }
    }

    void variable(ParseTreeNode *parent) {
        // Variable -> number | id
        if (look_ahead.first == "number") {
            try{
                match("number", parent);
            } catch (exception e){
                cout << e.what() << endl;
            }
        } else if (look_ahead.first == "id") {
            try{
                match("id", parent);
            } catch (exception e){
                cout << e.what() << endl;
            }
        }
    }

    void function_call(ParseTreeNode *parent) {
        // function_call -> id ( arguments ) ;
        if (look_ahead.first == "id") {
            try {
                match("id", parent);
                match("(", parent);
                temp = new ParseTreeNode("arguments");
                parent->addChild(temp);
                arguments(temp);
                match(")", parent);
                match(";", parent);
            } catch (exception e) {
                cout << e.what() << endl;
            }
        }
    }

    void arguments(ParseTreeNode *parent) {
        // arguments -> arg_expression | arg_expression , arguments | ε
        if (look_ahead.first == "id" || look_ahead.first == "number" || look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            temp = new ParseTreeNode("arg_expression");
            parent->addChild(temp);
            arg_expression(temp);
            if (look_ahead.first == ",") {
                match(",", parent);
                temp = new ParseTreeNode("arguments");
                parent->addChild(temp);
                arguments(temp);
            }
        } else {
            return;
        }
    }

    void arg_expression(ParseTreeNode *parent) {
        // arg_expression -> arithmetic_expr | boolean_expr
        string s = tokens[currentTokenIndex + 1].first;
        if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){ 
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            arithmetic_expr(temp);
        } else{
            variable(parent);
        }
    }

    void return_statement(ParseTreeNode *parent) {
        // return_statement  -> return return_expr ;
        match("return", parent);
        temp = new ParseTreeNode("return_expr");
        parent->addChild(temp);
        return_expr(temp);
        match(";", temp);
    }

    void return_expr(ParseTreeNode *parent) {
        // return_expr -> arithmetic_expr | boolean_expr | 1 | 0 | ε
        string s = tokens[currentTokenIndex + 1].first;
        if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){ 
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            arithmetic_expr(temp);
        } else if(look_ahead.first == "1" || look_ahead.first == "0"){
            match(look_ahead.first, parent);
        } else{
            variable(parent);
        }
    }

    void conditional_statements(ParseTreeNode *parent) {
        // conditional_statements -> if_expr | switch_expr
        if (look_ahead.first == "if") {
            temp = new ParseTreeNode("if_expr");
            parent->addChild(temp);
            if_expr(temp);
        } else if (look_ahead.first == "switch") {
            temp = new ParseTreeNode("switch_expr");
            parent->addChild(temp);
            switch_expr(temp);
        }
    }

    void body(ParseTreeNode *parent) {
        //body -> single_statement sub_body
        temp = new ParseTreeNode("single_statement");
        parent->addChild(temp);
        single_statement(temp);
        temp = new ParseTreeNode("sub_body");
        parent->addChild(temp);
        sub_body(temp);
    }

    void sub_body(ParseTreeNode *parent) {
        // sub_body -> body | ε
        //int | float | double | char | String | long | short | signed | unsigned
        if (look_ahead.first == "if" || look_ahead.first == "while" || look_ahead.first == "for" || look_ahead.first == "switch" || look_ahead.first == "return" || look_ahead.first == "id" || look_ahead.first == "do" || look_ahead.first == "enum" || look_ahead.first == "struct" || look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "String" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned") {
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
        }
    }

    void if_expr(ParseTreeNode *parent) {
        // if_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
        match("if", parent);
        match("(", parent);
        string s = tokens[currentTokenIndex + 1].first;
        if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else if(s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||"){
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else{
            variable(parent);
        }
        match(")", parent);
        if (look_ahead.first == "{") {
            match("{", parent);
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
            match("}", parent);
        } else {
            temp = new ParseTreeNode("single_statement");
            parent->addChild(temp);
            single_statement(temp);
        }
        temp = new ParseTreeNode("else_expr");
        parent->addChild(temp);
        else_expr(temp);
    }

    void else_expr(ParseTreeNode *parent) {
        // else_expr -> else {body} | else single_statement | ε
        if (look_ahead.first == "else") {
            match("else", parent);
            if (look_ahead.first == "{") {
                match("{", parent);
                temp = new ParseTreeNode("body");
                parent->addChild(temp);
                body(temp);
                match("}", parent);
            } else {
                temp = new ParseTreeNode("single_statement");
                parent->addChild(temp);
                single_statement(temp);
            }
        }
    }

    void switch_expr(ParseTreeNode *parent) {
        // switch_expr -> switch(id) { case_expr default_expr }
        match("switch", parent);
        match("(", parent);
        match("id", parent);
        match(")", parent);
        match("{", parent);
        if (look_ahead.first == "case") {
            temp = new ParseTreeNode("case_expr");
            parent->addChild(temp);
            case_expr(temp);
        }
        if (look_ahead.first == "default") {
            temp = new ParseTreeNode("default_expr");
            parent->addChild(temp);
            default_expr(temp);
        }
        match("}", parent);
    }

    void case_expr(ParseTreeNode *parent) {
        // case_expr -> case const : body break; | case const : body | case_expr | ε
        while (look_ahead.first == "case") {
            match("case", parent);
            match(look_ahead.first, parent);
            match(":", parent);
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
            if (look_ahead.first == "break") {
                match("break", parent);
                match(";", parent);
            }
        }
    }

    void default_expr(ParseTreeNode *parent) {
        // default_expr -> default: body | default: body break;
        if (look_ahead.first == "default") {
            match("default", parent);
            match(":", parent);
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
            if (look_ahead.first == "break") {
                match("break", parent);
                match(";", parent);
            }
        }
    }

    void const_expr(ParseTreeNode *parent) {
        // const -> number | string | char
        if (look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.second == "char") {
            match(look_ahead.first, parent);
        }
    }

};
