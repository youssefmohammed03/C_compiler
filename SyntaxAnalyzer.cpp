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
bool isError = false;
vector<pair<string, string>> symbolTableVector;

public:
    SyntaxAnalyzer(vector<pair<string, string>> tokens, vector<pair<string, string>> symbolTableVector) {
        this->tokens = tokens;
        this->currentTokenIndex = 0;
        this->look_ahead = tokens[currentTokenIndex];
        this->symbolTableVector = symbolTableVector;
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
            temp->isTerminal = true;
            parent->addChild(temp);
            if(look_ahead.first == "id"){
                for(int i = 0; i < symbolTableVector.size(); i++){
                    if(symbolTableVector[i].second == look_ahead.second){
                        temp->isTerminal = false;
                        temp->addChild(new ParseTreeNode(symbolTableVector[i].first));
                        temp->children[0]->isTerminal = true;
                        break;
                    }
                }
            }
            if(look_ahead.first == "number"){
                temp->isTerminal = false;
                temp->addChild(new ParseTreeNode(look_ahead.second));
                temp->children[0]->isTerminal = true;
            }
            look_ahead = getNextToken();
        } else{
            throw std::runtime_error("Unexpected token: " + look_ahead.first + ". Expected: " + expectedToken);
        }
    }

    void single_statement(ParseTreeNode *parent) {
        // single_statement -> expression | conditional_statements | iterative_statements | function_calls | return_statement | variable_declaration | array_declaration | pointer_declaration
        if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "void" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
                if(tokens[currentTokenIndex + 2].first == "[" || tokens[currentTokenIndex + 3].first == "["){
                    temp = new ParseTreeNode("array_declaration");
                    parent->addChild(temp);
                    array_declaration(temp);
                } else if (tokens[currentTokenIndex + 1].first == "*" || tokens[currentTokenIndex + 2].first == "*" || tokens[currentTokenIndex + 3].first == "*"){
                    temp = new ParseTreeNode("pointer_declaration");
                    parent->addChild(temp);
                    pointer_declaration(temp);
                } else{
                    temp = new ParseTreeNode("variable_declaration");
                    parent->addChild(temp);
                    variable_declaration(temp);
                }
        } else if (look_ahead.first == "if" || look_ahead.first == "switch") {
            temp = new ParseTreeNode("conditional_statement");
            parent->addChild(temp);
            conditional_statements(temp);
        } else if(look_ahead.first == "return"){
            temp = new ParseTreeNode("return_statement");
            parent->addChild(temp);
            return_statement(temp);
        } else if(look_ahead.first == "number" || look_ahead.first == "id"){
            if(look_ahead.first == "id" && tokens[currentTokenIndex + 1].first == "(") {
                temp = new ParseTreeNode("function_call");
                parent->addChild(temp);
                function_call(temp);
            } else {
                temp = new ParseTreeNode("expression");
                parent->addChild(temp);
                expression(temp);
            }
        }else if(look_ahead.first == "do" || look_ahead.first == "while" || look_ahead.first == "for"){
            temp = new ParseTreeNode("Iterative_statement");
            parent->addChild(temp);
            Iterative_statement(temp);
        } else {
            throw std::runtime_error("Unexpected token in single_statement: " + look_ahead.first);
        }
    }

    void parse() {
        root = new ParseTreeNode("program");
        while (currentTokenIndex < tokens.size()) {
            if(this->isError){
                cout << "Parsing failed" << endl;
                return;
            }
            if(look_ahead.first == "Success" && look_ahead.second == "Tokens are finished"){
                cout << "Parsing completed successfully" << endl;
                root->printTree();
                return;
            }
            program(root);
        }
    }

    void program(ParseTreeNode *parent){
        //program -> declarations function_definitions
        temp = new ParseTreeNode("declarations");
        parent->addChild(temp);
        declarations(temp);

        temp = new ParseTreeNode("function_definitions");
        parent->addChild(temp);
        function_definitions(temp);
    }


    void function_definitions(ParseTreeNode *parent) {
        // function_definitions -> function_definition function_definitions | ε
        if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "void" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
            if(look_ahead.first == "void" || tokens[currentTokenIndex + 2].first == "(" || tokens[currentTokenIndex + 3].first == "("){
                temp = new ParseTreeNode("function_definition");
                parent->addChild(temp);
                function_definition(temp);
                temp = new ParseTreeNode("function_definitions");
                parent->addChild(temp);
                function_definitions(temp);
            }
        }
    }

    void declarations(ParseTreeNode *parent) {
        // enum_declaration | structure_declaration | array_declaration | pointer_declaration | variable_declaration | ε
        if(look_ahead.first == "enum"){
            temp = new ParseTreeNode("enum_declaration");
            parent->addChild(temp);
            enum_declaration(temp);
            return;
        }
        if(look_ahead.first == "struct"){
            temp = new ParseTreeNode("structure_declaration");
            parent->addChild(temp);
            structure_declaration(temp);
            return;
        }
        if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "void" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
            if(look_ahead.first == "void" || tokens[currentTokenIndex + 2].first == "(" || tokens[currentTokenIndex + 3].first == "("){
                return;
            } else if(tokens[currentTokenIndex + 2].first == "[" || tokens[currentTokenIndex + 3].first == "["){
                temp = new ParseTreeNode("array_declaration");
                parent->addChild(temp);
                array_declaration(temp);
            } else if (tokens[currentTokenIndex + 1].first == "*" || tokens[currentTokenIndex + 2].first == "*"){
                temp = new ParseTreeNode("pointer_declaration");
                parent->addChild(temp);
                pointer_declaration(temp);
            } else {
                temp = new ParseTreeNode("variable_declaration");
                parent->addChild(temp);
                variable_declaration(temp);
            }
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
        variable(parent);
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
            } catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;
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
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        }
    }

    void assignment_expr(ParseTreeNode *parent) {
        // assignment_expr -> variable assignment_op variable | variable assignment_op arithmetic_expr
        variable(parent);
        temp = new ParseTreeNode("assignment_op");
        parent->addChild(temp);
        assignment_op(temp);
        string s = tokens[currentTokenIndex + 1].first;
        if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){
            temp = new ParseTreeNode("arithmetic_expr");
            parent->addChild(temp);
            arithmetic_expr(temp);
        } else{
            variable(parent);
        }
        try{
            match(";", parent);
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;
            }
    }

    void assignment_op(ParseTreeNode *parent) {
        // Assignment_op -> '='| '+=' | '-=' | '*=' | '/=' | '%='| '&='| '|='| '^='| '<<=' | '>>='
        if (look_ahead.first == "=" || look_ahead.first == "+=" || look_ahead.first == "-=" || look_ahead.first == "*=" || look_ahead.first == "/=" || look_ahead.first == "%=" || look_ahead.first == "&=" || look_ahead.first == "|=" || look_ahead.first == "^=" || look_ahead.first == "<<=" || look_ahead.first == ">>=") {
            try{
                match(look_ahead.first, parent);
            } catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        }
    }

    void variable(ParseTreeNode *parent) {
        // Variable -> number | id
        if (look_ahead.first == "number") {
            try{
                match("number", parent);
            } catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        } else{
            try{
                match("id", parent);
            } catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

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
            } catch (exception& e) {
                cout << e.what() << endl;
                this->isError = true;

            }
        }
    }

    void arguments(ParseTreeNode *parent) {
        // arguments -> arg_expression | arg_expression , arguments | ε
        if (look_ahead.first == "id" || look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
            temp = new ParseTreeNode("arg_expression");
            parent->addChild(temp);
            arg_expression(temp);
            if (look_ahead.first == ",") {
                try{
                    match(",", parent);
                }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

                }
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
        } else if(look_ahead.second == "string"){
            try{
                match(look_ahead.first, parent);
            } catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;
            }
        } else{
            variable(parent);
        }
    }

    void return_statement(ParseTreeNode *parent) {
        // return_statement  -> return return_expr ;
        try{
            match("return", parent);
            temp = new ParseTreeNode("return_expr");
            parent->addChild(temp);
            return_expr(temp);
            match(";", parent);
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
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
            try{
                match(look_ahead.first, parent);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        } else if(look_ahead.first == "id" || look_ahead.first == "number"){
            variable(parent);
        }
    }

    void Iterative_statement(ParseTreeNode *parent) {
        //Iterative_Statements-> for_loop | while_loop | do_while_loop
        if(look_ahead.first=="for"){
            temp = new ParseTreeNode("for_loop");
            parent->addChild(temp);
            for_loop(temp);
        }
        else if(look_ahead.first=="while"){
            temp = new ParseTreeNode("while_loop");
            parent->addChild(temp);
            while_loop(temp);
        }
        else if(look_ahead.first=="do"){
            temp = new ParseTreeNode("do_while_loop");
            parent->addChild(temp);
            do_while_loop(temp);
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
        try{
            single_statement(temp);
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

        }
        temp = new ParseTreeNode("sub_body");
        parent->addChild(temp);
        sub_body(temp);
    }

    void sub_body(ParseTreeNode *parent) {
        // sub_body -> body | ε
        if (look_ahead.first == "if" || look_ahead.first == "while" || look_ahead.first == "for" || look_ahead.first == "switch" || look_ahead.first == "return" || look_ahead.first == "id" || look_ahead.first == "do" || look_ahead.first == "enum" || look_ahead.first == "struct" || look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned") {
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
        }
    }

    void for_loop(ParseTreeNode *parent) {
        // for_loop -> for (init_expr condition_expr; update_expr) { body}
        try{
            match("for", parent);                       
            match("(", parent);
            temp = new ParseTreeNode("init_expr");
            parent->addChild(temp);
            init_expr(temp);
            temp = new ParseTreeNode("condition_expr");
            parent->addChild(temp);
            condition_expr(temp);
            match(";", parent);
            temp = new ParseTreeNode("update_expr");
            parent->addChild(temp);
            update_expr(temp);
            match(")", parent);
            match("{", parent);
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
            match("}", parent);
        }catch (exception& e){
            cout << e.what() << endl;
            this->isError = true;
        }
    }
    
    void init_expr(ParseTreeNode *parent) {
        // init_expr -> assignment_expr | variable_declaration  | ε
        if (look_ahead.first == "id") {
            temp = new ParseTreeNode("assignment_expr");
            parent->addChild(temp);
            assignment_expr(temp);
        } else if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "void" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
            temp = new ParseTreeNode("variable_declaration");
            parent->addChild(temp);
            variable_declaration(temp);
        } else{
            try{
                match(";", parent);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;
            }
            return;
        }
    }

    void condition_expr(ParseTreeNode *parent) {
        // condition_expr -> boolean_expr | ε
        string s = tokens[currentTokenIndex + 1].first;
        if (s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||") {
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        }
    }

    void update_expr(ParseTreeNode *parent) {
        // update_expr -> variable  assignment_op variable | variable assignment_op arithmetic_expr | ε
        try{
            variable(parent);
            temp = new ParseTreeNode("assignment_op");
            parent->addChild(temp);
            assignment_op(temp);
            string s = tokens[currentTokenIndex + 1].first;
            if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){
                temp = new ParseTreeNode("arithmetic_expr");
                parent->addChild(temp);
                arithmetic_expr(temp);
            } else if(look_ahead.first == "id" || look_ahead.first == "number"){
                variable(parent);
            } else{
                return;
            }
        }catch (exception& e){
            cout << e.what() << endl;
            this->isError = true;
        }
    }

    void while_loop(ParseTreeNode *parent) {
        // while_loop -> while (Condition_expr_while ) { body }
        try{
            match("while", parent);
            match("(", parent);
            temp = new ParseTreeNode("Condition_expr_while");
            parent->addChild(temp);
            Condition_expr_while(temp);
            match(")", parent);
            match("{", parent);
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
            match("}", parent);
        }catch (exception& e){
            cout << e.what() << endl;
            this->isError = true;
        }
    }

    void Condition_expr_while(ParseTreeNode *parent) {
        // Condition_expr_while -> boolean_expr | variable
        string s = tokens[currentTokenIndex + 1].first;
        if (s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||") {
            temp = new ParseTreeNode("boolean_expr");
            parent->addChild(temp);
            boolean_expr(temp);
        } else {
            try{
                variable(parent);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;
            }
        }
    }

    void do_while_loop(ParseTreeNode *parent) {
        // do_while_loop -> do { body } while (Condition_expr_while );
        try{
            match("do", parent);
            match("{", parent);
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
            match("}", parent);
            match("while", parent);
            match("(", parent);
            temp = new ParseTreeNode("Condition_expr_while");
            parent->addChild(temp);
            Condition_expr_while(temp);
            match(")", parent);
            match(";", parent);
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;
        }
    }
    
    void if_expr(ParseTreeNode *parent) {
        // if_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
        try{
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
                try{
                    single_statement(temp);
                }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

                }
            }
            temp = new ParseTreeNode("else_expr");
            parent->addChild(temp);
            else_expr(temp);
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
    }

    void else_expr(ParseTreeNode *parent) {
        // else_expr -> else {body} | else single_statement | ε
        try{
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
                    try{
                        single_statement(temp);
                    }catch (exception& e){
                        cout << e.what() << endl;
                this->isError = true;

                    }
                }
            }
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
    }

    void switch_expr(ParseTreeNode *parent) {
        // switch_expr -> switch(id) { case_expr default_expr }
        try{
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
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

        }
    }

    void case_expr(ParseTreeNode *parent) {
        // case_expr -> case const : body break; | case const : body | case_expr | ε
        while (look_ahead.first == "case") {
            try{
                match("case", parent);
                temp = new ParseTreeNode("const_expr");
                parent->addChild(temp);
                try{
                    const_expr(temp);
                }catch (exception& e){
                    cout << e.what() << endl;
                this->isError = true;

                }
                match(":", parent);
                temp = new ParseTreeNode("body");
                parent->addChild(temp);
                body(temp);
                if (look_ahead.first == "break") {
                    match("break", parent);
                    match(";", parent);
                }
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        }
    }

    void default_expr(ParseTreeNode *parent) {
        // default_expr -> default: body | default: body break;
        if (look_ahead.first == "default") {
            try{
                match("default", parent);
                match(":", parent);
                temp = new ParseTreeNode("body");
                parent->addChild(temp);
                body(temp);
                if (look_ahead.first == "break") {
                    match("break", parent);
                    match(";", parent);
                }
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        }
    }

    void const_expr(ParseTreeNode *parent) {
        // const -> number | string | char
        if (look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.second == "char") {
            try{
                match(look_ahead.first, parent);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        } else{
            throw std::runtime_error("Unexpected token in const_expr: " + look_ahead.first);
        }
    }

    void variable_declaration(ParseTreeNode *parent) {
        // variable_declaration -> data_type variable_list ;
        ParseTreeNode* temp = new ParseTreeNode("data_type");
        parent->addChild(temp);
        data_type(temp);

        temp = new ParseTreeNode("variable_list");
        parent->addChild(temp);
        variable_list(temp);
        try{
            match(";", parent);
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
    }

    void data_type(ParseTreeNode *parent) {
        // data_type -> type_modifier type
        temp = new ParseTreeNode("type_modifier");
        parent->addChild(temp);
        type_modifier(temp);

        temp = new ParseTreeNode("type");
        parent->addChild(temp);
        try{
            type(temp);
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;

        }
    }

    void type(ParseTreeNode *parent) {
        // type -> int | float | double | char | string | long | short | signed | unsigned
        if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" ||
            look_ahead.first == "char" || look_ahead.first == "string") {
            try{
                match(look_ahead.first, parent);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        } else {
            throw std::runtime_error("Unexpected token in type: " + look_ahead.first);
        }
    }

    void type_modifier(ParseTreeNode *parent) {
        // type_modifier -> const | volatile | restrict | ε
        if (look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict" || look_ahead.first == "long" ||
            look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned") {
            try{
                match(look_ahead.first, parent);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }    
        }
    }

    void variable_list(ParseTreeNode *parent) {
        // variable_list -> id equal_assign | id equal_assign, variable_list
        try{
            match("id", parent);
            temp = new ParseTreeNode("equal_assign");
            parent->addChild(temp);
            equal_assign(temp);
            if (look_ahead.first == ",") {
                match(",", parent);
                temp = new ParseTreeNode("variable_list");
                parent->addChild(temp);
                variable_list(temp);
            }
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
    }

    void equal_assign(ParseTreeNode *parent) {
        // equal_assign -> = const | = id | = arithmetic_expr |ε
        try{
            if (look_ahead.first == "=") {
                match("=" , parent);
                string s = tokens[currentTokenIndex + 1].first;
                if(s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>"){
                    temp = new ParseTreeNode("arithmetic_expr");
                    parent->addChild(temp);
                    arithmetic_expr(temp);
                } else if(look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.second == "char"){
                    temp = new ParseTreeNode("const_expr");
                    parent->addChild(temp);
                    const_expr(temp);
                } else{
                    match("id", parent);
                }
            }
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
    }

    void function_definition(ParseTreeNode *parent) {
        // function_definition -> function_header { body }
        temp = new ParseTreeNode("function_header");
        parent->addChild(temp);
        function_header(temp);

        try{
            match("{", parent);
            temp = new ParseTreeNode("body");
            parent->addChild(temp);
            body(temp);
            match("}", parent);
        }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
    }

    void function_header(ParseTreeNode *parent) {
        // function_header -> function_datatype id ( parameter_list )
        temp = new ParseTreeNode("function_datatype");
        parent->addChild(temp);
        function_datatype(temp);

        try{
            match("id", parent);
            match("(", parent);

            temp = new ParseTreeNode("parameter_list");
            parent->addChild(temp);
            parameter_list(temp);

            match(")", parent);
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;

        }
    }

    void function_datatype(ParseTreeNode *parent) {
        // function_datatype -> data_type | void
        if (look_ahead.first == "void") {
            try{    
                match("void", parent);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        } else {
            temp = new ParseTreeNode("data_type");
            parent->addChild(temp);
            data_type(temp);
        }
    }

    void parameter_list(ParseTreeNode *parent) {
        // parameter_list -> parameter sub_parameter_list
        if(look_ahead.first == ")"){
            return;
        }
        temp = new ParseTreeNode("parameter");
        parent->addChild(temp);
        parameter(temp);

        if(look_ahead.first == ")"){
            return;
        }

        temp = new ParseTreeNode("sub_parameter_list");
        parent->addChild(temp);
        sub_parameter_list(temp);
    }

    void sub_parameter_list(ParseTreeNode *parent) {
        // sub_parameter_list -> , parameter sub_parameter_list | ε
        if (look_ahead.first == ",") {
            try{
                match(",", parent);
                temp = new ParseTreeNode("parameter");
                parent->addChild(temp);
                parameter(temp);

                temp = new ParseTreeNode("sub_parameter_list");
                parent->addChild(temp);
                sub_parameter_list(temp);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        }
    }

    void parameter(ParseTreeNode *parent) {
        // parameter -> data_type id | ε
        if(look_ahead.first == ")"){
            return;
        } else{
            temp = new ParseTreeNode("data_type");
            parent->addChild(temp);
            data_type(temp);
            try{
                match("id", parent);
            }catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }
        }
    }

    void array_declaration(ParseTreeNode *parent) {
        // array_declaration -> data_type id array_dimensions
        temp = new ParseTreeNode("data_type");
        parent->addChild(temp);
        data_type(temp);
        try{
            match("id", parent);
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;

        }
        temp = new ParseTreeNode("array_dimensions");
        parent->addChild(temp);
        array_dimensions(temp);
        try{
            match(";", parent);
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;

        }
    }

    void array_dimensions(ParseTreeNode *parent) {
        // array_dimensions -> [ variable ] | [ variable ] array_dimensions
        try{
            match("[", parent);
            variable(parent);
            match("]", parent);
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;

        }

        if (look_ahead.first == "[") {
            temp = new ParseTreeNode("array_dimensions");
            parent->addChild(temp);
            array_dimensions(temp);
        }
    }

    void pointer_declaration(ParseTreeNode *parent) {
        // pointer_declaration -> data_type variable_list_point ;
        temp = new ParseTreeNode("data_type");
        parent->addChild(temp);
        data_type(temp);
        temp = new ParseTreeNode("variable_list_point");
        parent->addChild(temp);
        variable_list_point(temp);
        try{
            match(";", parent);
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;
        }
    }

    void variable_list_point(ParseTreeNode *parent) {
        // variable_list_point -> *variable_point | *variable_point, variable_list_point
        try{
            match("*", parent);
            temp = new ParseTreeNode("variable_point");
            parent->addChild(temp);
            variable_point(temp);

            if (look_ahead.first == ",") {
                    match(",", parent);
                temp = new ParseTreeNode("variable_list_point");
                parent->addChild(temp);
                variable_list_point(temp);
            }
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;
        }
    }

    void variable_point(ParseTreeNode *parent) {
        // variable_point -> id | id = &id
        try{
            if (look_ahead.first == "id") {
                match("id", parent);
                if (look_ahead.first == "=") {
                    match("=", parent);
                    match("&", parent);
                    match("id", parent);
                }
            }
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;
        }
    }

    void enum_declaration(ParseTreeNode *parent) {
        // enum_declaration -> enum id { enum_constants } ;
        try{
            match("enum", parent);

            match("id", parent);

            match("{", parent);

            temp = new ParseTreeNode("enum_constants");
            parent->addChild(temp);
            enum_constants(temp);

            match("}", parent);
            match(";", parent);
        } catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;

        }
    }

    void enum_constants(ParseTreeNode *parent) {
        // enum_constants -> enum_constant | enum_constant , enum_constants
        temp = new ParseTreeNode("enum_constant");
        parent->addChild(temp);
        enum_constant(temp);

        if (look_ahead.first == ",") {
            try{
                match(",", parent);
            } catch (exception& e){
                cout << e.what() << endl;
                this->isError = true;

            }

            temp = new ParseTreeNode("enum_constants");
            parent->addChild(temp);
            enum_constants(temp);
        }
    }

    void enum_constant(ParseTreeNode *parent) {
        // enum_constant -> id
        try{
            match("id", parent);
        } catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;

        }
    }

    void structure_declaration(ParseTreeNode *parent) {
        // structure_declaration -> struct id { member_list } ;
        try{
            match("struct", parent);
            match("id", parent);
            match("{", parent);

            temp = new ParseTreeNode("member_list");
            parent->addChild(temp);
            member_list(temp);

            match("}", parent);
            match(";", parent);
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;
        }
    }

    void member_list(ParseTreeNode *parent) {
        // member_list -> member | member ; member_list | ε
        if(look_ahead.first == "}") return;
        temp = new ParseTreeNode("member");
        parent->addChild(temp);
        member(temp);

        if (look_ahead.first == ";") {
            try{
                match(";", parent);
            }catch (exception& e){
                cout << e.what() << endl;
                    this->isError = true;
            }
            temp = new ParseTreeNode("member_list");
            parent->addChild(temp);
            member_list(temp);
        }
    }

    void member(ParseTreeNode *parent) {
        // member -> data_type id equal_assign
        temp = new ParseTreeNode("data_type");
        parent->addChild(temp);
        data_type(temp);
        try{
            match("id", parent);
        }catch (exception& e){
            cout << e.what() << endl;
                this->isError = true;
        }
        temp = new ParseTreeNode("equal_assign");
        parent->addChild(temp);
        equal_assign(temp);
    }

};
