#pragma once

#include <string>
#include <msclr/marshal_cppstd.h> 
#include <iostream>
#include <regex>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
//#include "ParserTreeNode.h"

using namespace std;

class ParseTreeNode {
public:
    string type;
    vector<ParseTreeNode*> children;
    bool isTerminal = false;

    ParseTreeNode(string type) : type(type) {}

    void addChild(ParseTreeNode* child) {
        children.push_back(child);
    }

    vector<ParseTreeNode*> getChildren() {
        return children;
    }

    string getType() {
        return type;
    }

    std::string buildTreeString(int level = 0) {
        std::string result;

        // Add indentation
        for (int i = 0; i < level; i++) {
            result += "  ";
        }

        // Add node type
        result += "\"" + type + "\"";

        // Add children
        if (!children.empty()) {
            result += " -> {";
            for (size_t i = 0; i < children.size(); i++) {
                result += "\"" + children[i]->type + "\"";
                if (!children[i]->isTerminal && children[i]->children.empty()) {
                    continue;
                }
                if (i != children.size() - 1) {
                    result += ", ";
                }
            }
            result += "}";
        }

        result += "\n";

        // Recursively build children's trees
        for (ParseTreeNode* child : children) {
            result += child->buildTreeString(level + 1);
        }

        return result;
    }

    void generateGraphvizDotFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }

        file << "digraph ParseTree {\n";

        // Generate the DOT representation of the tree
        generateDotRepresentation(file, 0);

        file << "}\n";

        file.close();
        std::cout << "Graphviz DOT file generated: " << filename << std::endl;
    }

    int generateDotRepresentation(std::ofstream& file, int parentId = 0) {
        int currentId = parentId;

        // For each child of the current node
        for (size_t i = 0; i < children.size(); i++) {
            // Generate a unique ID for the child
            int childId = ++currentId;

            // Write the relationship to the file
            file << "\"" << parentId << "\" -> \"" << childId << "\";\n";

            // Set the labels for the nodes
            file << "\"" << parentId << "\" [label=\"" << type << "\"];\n";
            file << "\"" << childId << "\" [label=\"" << children[i]->type << "\"];\n";

            // Recurse on the child
            currentId = children[i]->generateDotRepresentation(file, childId);
        }

        // Return the last used ID
        return currentId;
    }
};

namespace ScannerGUI2 {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;



    vector<string> errors;
    vector<string> lexemes;
    vector<pair<string, string>> symbolTableVector;
    vector<pair<string, string>> tokens;
    ParseTreeNode* root;
    ParseTreeNode* temp;
    int currentTokenIndex;
    pair<string, string> look_ahead;
    bool isError = false;
    int counter = 0;

    regex keywordPattern("\\b(alignas|alignof|auto|bool|break|case|const|constexpr|continue|default|do|else|enum|extern|false|for|goto|if|inline|register|restrict|return|signed|sizeof|static|static_assert|struct|switch|thread_local|true|typedef|typeof|typeof_unqual|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Decimal128|_Decimal32|_Decimal64|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local)\\b");
    regex dataTypePattern("\\b(char|double|float|int|long|short|signed|unsigned|void|string)\\b");
    regex arithPattern("(\\+\\+|\\-\\-|\\+|\\-|\\*|\\/|\\%|\\~|\\<\\<|\\>\\>|\\^|([^\\&]|^)\\&([^\\&]|$)|([^\\|]|^)\\|([^\\|]|$))");
    regex boolPattern("(\\=\\=|\\!\\=|\\<\\=|\\>\\=|([^\\>]|^)\\>([^\\>]|$)|([^\\<]|^)\\<([^\\<]|$)|\\!|\\&\\&|\\|\\|)");
    regex assignmentPattern("((\\+=)|(-=)|(\\*=)|(\\/=)|(\\%=)|(&=)|(\\|=)|(\\^=)|(<<=)|(>>=)|(=))");
    regex punctuationPattern("(\\?|\\-\\>|\\:\\:|\\{|\\}|\\(|\\)|\\[|\\]|\\;|\\,|\\.|\\:)");
    regex identifierPattern("^[_a-zA-Z][_a-zA-Z0-9]*$");
    regex decimal_regex("^[-+]?[1-9][0-9]*\\.?[0-9]*$");
    regex binary_regex("^0b[01]+$");
    regex octal_regex("^0[0-7]*$");
    regex hex_regex("^0x[a-fA-F0-9]+$");
    regex string_regex("\"(\\\\.|[^\"])*\"");
    regex char_regex("'(\\\\.|[^'])*'");

    void printTokens(const vector<pair<string, string>>& tokens) {
        cout << "Tokens\n";
        for (const auto& token : tokens) {
            if (token.second != "") {
                cout << "<" << token.first << ", " << token.second << ">" << "\n";
            }
            else {
                cout << "<" << token.first << ">" << "\n";
            }
        }
    }

    void printErrors() {
        cout << "Errors\n";
        for (const auto& element : errors) {
            cout << element << endl;
        }
    }

    void printLexemes() {
        cout << "Lexemes\n";
        for (const string& lex : lexemes) {
            cout << lex << endl;
        }
    }

    string removeComments(string code) {
        regex commentPattern("(\\/\\*([^*]|[\r\n]|(\\*+([^*/]|[\r\n])))*\\*\\/)|(\\/\\/.*)|#[^\\n]*");
        return regex_replace(code, commentPattern, "");
    }

    string removeExtraSpaces(string code) {
        regex spacePattern("\\s+");
        return regex_replace(code, spacePattern, " ");
    }

    string extractPreprocessors(string code) {
        regex preprocessorPattern("#[^\\n]*");
        string result;
        smatch match;

        while (regex_search(code, match, preprocessorPattern)) {
            result += match.str() + "\n";
            code = match.suffix().str();
        }

        return result;
    }

    bool isValidIdentifier(const string& str) {
        return regex_match(str, identifierPattern);
    }

    void processToken(const string& temp, vector<pair<string, string>>& tokens) {
        if (regex_match(temp, keywordPattern)) {
            tokens.push_back(make_pair(temp, ""));
            lexemes.push_back(temp);
        }
        else if (regex_match(temp, dataTypePattern)) {
            tokens.push_back(make_pair(temp, ""));
            lexemes.push_back(temp);
        }
        else if (regex_match(temp, arithPattern)) {
            tokens.push_back(make_pair(temp, ""));
            lexemes.push_back(temp);
        }
        else if (regex_match(temp, boolPattern)) {
            tokens.push_back(make_pair(temp, ""));
            lexemes.push_back(temp);
        }
        else if (regex_match(temp, assignmentPattern)) {
            tokens.push_back(make_pair(temp, ""));
            lexemes.push_back(temp);
        }
        else if (regex_match(temp, punctuationPattern)) {
            tokens.push_back(make_pair(temp, ""));
            lexemes.push_back(temp);
        }
        else {
            if (isValidIdentifier(temp)) {
                auto it = find_if(symbolTableVector.begin(), symbolTableVector.end(),
                    [&](const pair<string, string>& entry) { return entry.first == temp; });
                if (it == symbolTableVector.end()) {
                    symbolTableVector.push_back(make_pair(temp, to_string(counter++)));
                    tokens.push_back(make_pair("id", symbolTableVector.back().second));
                    lexemes.push_back(temp);
                }
                else {
                    tokens.push_back(make_pair("id", it->second));
                    lexemes.push_back(temp);
                }
            }
            else {
                errors.push_back(temp);
                lexemes.push_back(temp);
            }
        }
    }

    void twoCharOps(string& temp, const string& code, int& i) {
        string multiCharOp;
        string twoCharOps[] = { "||", "&&", "<=", ">=", "==", "!=", "<<", ">>", "++", "--", "-=", "+=", "*=", "/=", "%=", "&=", "|=", "^=", "->", "::" };
        string threeCharOps[] = { "<<=", ">>=" };

        if (i + 1 < code.size()) {
            multiCharOp = temp + code[i + 1];
            if (find(begin(twoCharOps), end(twoCharOps), multiCharOp) != end(twoCharOps)) {
                temp = multiCharOp;
                ++i;
                if (i + 1 < code.size()) {
                    multiCharOp = temp + code[i + 1];
                    if (find(begin(threeCharOps), end(threeCharOps), multiCharOp) != end(threeCharOps)) {
                        temp = multiCharOp;
                        ++i;
                    }
                }
            }
        }
    }

    void numbersDetector(string& temp, const string& code, int& i, vector<pair<string, string>>& tokens) {
        string number = temp;
        bool isInvalid = false;

        while (i + 1 < code.size() && (isdigit(code[i + 1]) || code[i + 1] == '.' || isalpha(code[i + 1]))) {
            number += code[++i];
        }

        if (regex_match(number, decimal_regex)) {
            tokens.push_back(make_pair("number", number));
            lexemes.push_back(number);
        }
        else if (regex_match(number, binary_regex)) {
            tokens.push_back(make_pair("number", number));
            lexemes.push_back(number);
        }
        else if (regex_match(number, octal_regex)) {
            tokens.push_back(make_pair("number", number));
            lexemes.push_back(number);
        }
        else if (regex_match(number, hex_regex)) {
            tokens.push_back(make_pair("number", number));
            lexemes.push_back(number);
        }
        else {
            errors.push_back(number);
            lexemes.push_back(number);
        }
    }

    vector<pair<string, string>> analyzeCode(const string& code) {
        vector<pair<string, string>> tokens;
        string separators = "(){}[]?.,;+-*/%~<>^&|!=:\"\'";
        string temp;
        for (int i = 0; i < code.size(); ++i) {
            char c = code[i];
            if (separators.find(c) != string::npos) {
                if (!temp.empty()) {
                    processToken(temp, tokens);
                    temp.clear();
                }
                temp += c;
                if (c == '\"') {
                    int j = i + 1;
                    while (j < code.size()) {
                        if (code[j] == '\"' && code[j - 1] != '\\') {
                            break;
                        }
                        temp += code[j++];
                    }
                    if (j < code.size()) {
                        temp += code[j++];
                    }
                    i = j - 1;
                    tokens.push_back(make_pair(temp, "string"));
                    lexemes.push_back(temp);
                }
                else if (c == '\'') {
                    int j = i + 1;
                    while (j < code.size() && code[j] != '\'') {
                        temp += code[j++];
                    }
                    if (j < code.size()) {
                        temp += code[j++];
                    }
                    i = j - 1;
                    tokens.push_back(make_pair(temp, "char"));
                    lexemes.push_back(temp);
                }
                else if ((c == '-' || c == '+') && i + 1 < code.size() && isdigit(code[i + 1])) {
                    numbersDetector(temp, code, i, tokens);
                }
                else {
                    twoCharOps(temp, code, i);
                    processToken(temp, tokens);
                }
                temp.clear();
            }
            else if (c == ' ') {
                if (!temp.empty()) {
                    processToken(temp, tokens);
                    temp.clear();
                }
            }
            else if (isdigit(c)) {
                if (temp.empty() || isdigit(temp[0])) {
                    temp += c;
                    numbersDetector(temp, code, i, tokens);
                    temp.clear();
                }
            }
            else {
                temp += c;
            }
        }

        if (!temp.empty()) {
            processToken(temp, tokens);
        }

        return tokens;
    }


    // Define a function to perform the conversion
 

    /// <summary>
    /// Summary for MyForm
    /// </summary>
    public ref class MyForm : public System::Windows::Forms::Form
    {
    public:
        MyForm(void)
        {
            InitializeComponent();
            //
            //TODO: Add the constructor code here
            //
        }

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~MyForm()
        {
            if (components)
            {
                delete components;
            }
        }
    private: System::Windows::Forms::Panel^ panel1;
    protected:
    private: System::Windows::Forms::Button^ button1;
    private: System::Windows::Forms::TextBox^ textBox1;
    private: System::Windows::Forms::Label^ label2;
    private: System::Windows::Forms::Button^ updateDictionbutton;
    private: System::Windows::Forms::Label^ label3;
    private: System::Windows::Forms::Label^ output;
    private: System::Windows::Forms::Label^ error;

    protected:

    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
        System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->panel1 = (gcnew System::Windows::Forms::Panel());
            this->error = (gcnew System::Windows::Forms::Label());
            this->output = (gcnew System::Windows::Forms::Label());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->updateDictionbutton = (gcnew System::Windows::Forms::Button());
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->textBox1 = (gcnew System::Windows::Forms::TextBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->panel1->SuspendLayout();
            this->SuspendLayout();
            // 
            // panel1
            // 
            this->panel1->Controls->Add(this->error);
            this->panel1->Controls->Add(this->output);
            this->panel1->Controls->Add(this->label3);
            this->panel1->Controls->Add(this->updateDictionbutton);
            this->panel1->Controls->Add(this->button1);
            this->panel1->Controls->Add(this->textBox1);
            this->panel1->Controls->Add(this->label2);
            this->panel1->Location = System::Drawing::Point(12, 12);
            this->panel1->Name = L"panel1";
            this->panel1->Size = System::Drawing::Size(620, 426);
            this->panel1->TabIndex = 0;
            // 
            // error
            // 
            this->error->AutoSize = true;
            this->error->ForeColor = System::Drawing::Color::Red;
            this->error->Location = System::Drawing::Point(34, 329);
            this->error->Name = L"error";
            this->error->Size = System::Drawing::Size(0, 16);
            this->error->TabIndex = 5;
            // 
            // output
            // 
            this->output->AutoSize = true;
            this->output->Location = System::Drawing::Point(34, 300);
            this->output->Name = L"output";
            this->output->Size = System::Drawing::Size(0, 16);
            this->output->TabIndex = 5;
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(26, 366);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(0, 16);
            this->label3->TabIndex = 4;
            // 
            // updateDictionbutton
            // 
            this->updateDictionbutton->Location = System::Drawing::Point(156, 377);
            this->updateDictionbutton->Name = L"updateDictionbutton";
            this->updateDictionbutton->Size = System::Drawing::Size(155, 31);
            this->updateDictionbutton->TabIndex = 3;
            this->updateDictionbutton->Text = L"Update Dictionary";
            this->updateDictionbutton->UseVisualStyleBackColor = true;
            this->updateDictionbutton->Click += gcnew System::EventHandler(this, &MyForm::updateDictionbutton_Click);
            // 
            // button1
            // 
            this->button1->Location = System::Drawing::Point(26, 377);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(105, 31);
            this->button1->TabIndex = 2;
            this->button1->Text = L"Analyze Text";
            this->button1->UseVisualStyleBackColor = true;
            this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
            // 
            // textBox1
            // 
            this->textBox1->Location = System::Drawing::Point(26, 69);
            this->textBox1->Multiline = true;
            this->textBox1->Name = L"textBox1";
            this->textBox1->Size = System::Drawing::Size(566, 212);
            this->textBox1->TabIndex = 1;
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(23, 28);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(264, 16);
            this->label2->TabIndex = 0;
            this->label2->Text = L"Enter code in the below box to check syntax";
            // 
            // MyForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(644, 450);
            this->Controls->Add(this->panel1);
            this->Name = L"MyForm";
            this->Text = L"Lexical Analyzer";
            this->panel1->ResumeLayout(false);
            this->panel1->PerformLayout();
            this->ResumeLayout(false);

        }
#pragma endregion
#include <Windows.h> // Required for MessageBox


        pair<string, string> getNextToken() {
            if (currentTokenIndex < tokens.size() - 1) {
                return tokens[++currentTokenIndex];
            }
            else {
                return make_pair("Success", "Tokens are finished");
            }
        }

        void match(string expectedToken, ParseTreeNode* parent) {
            if (look_ahead.first == expectedToken) {
                temp = new ParseTreeNode(expectedToken);
                temp->isTerminal = true;
                parent->addChild(temp);
                if (look_ahead.first == "id") {
                    for (int i = 0; i < symbolTableVector.size(); i++) {
                        if (symbolTableVector[i].second == look_ahead.second) {
                            temp->isTerminal = false;
                            temp->addChild(new ParseTreeNode(symbolTableVector[i].first));
                            temp->children[0]->isTerminal = true;
                            break;
                        }
                    }
                }
                if (look_ahead.first == "number") {
                    temp->isTerminal = false;
                    temp->addChild(new ParseTreeNode(look_ahead.second));
                    temp->children[0]->isTerminal = true;
                }
                look_ahead = getNextToken();
            }
            else {
                throw std::runtime_error("Unexpected token: " + look_ahead.first + ". Expected: " + expectedToken);
            }
        }

        void single_statement(ParseTreeNode* parent) {
            if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "void" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
                if (tokens[currentTokenIndex + 2].first == "[" || tokens[currentTokenIndex + 3].first == "[") {
                    temp = new ParseTreeNode("array_declaration");
                    parent->addChild(temp);
                    array_declaration(temp);
                }
                else if (tokens[currentTokenIndex + 1].first == "*" || tokens[currentTokenIndex + 2].first == "*" || tokens[currentTokenIndex + 3].first == "*") {
                    temp = new ParseTreeNode("pointer_declaration");
                    parent->addChild(temp);
                    pointer_declaration(temp);
                }
                else {
                    temp = new ParseTreeNode("variable_declaration");
                    parent->addChild(temp);
                    variable_declaration(temp);
                }
            }
            else if (look_ahead.first == "if" || look_ahead.first == "switch") {
                temp = new ParseTreeNode("conditional_statement");
                parent->addChild(temp);
                conditional_statements(temp);
            }
            else if (look_ahead.first == "return") {
                temp = new ParseTreeNode("return_statement");
                parent->addChild(temp);
                return_statement(temp);
            }
            else if (look_ahead.first == "number" || look_ahead.first == "id") {
                if (look_ahead.first == "id" && tokens[currentTokenIndex + 1].first == "(") {
                    temp = new ParseTreeNode("function_call");
                    parent->addChild(temp);
                    function_call(temp);
                }
                else {
                    temp = new ParseTreeNode("expression");
                    parent->addChild(temp);
                    expression(temp);
                }
            }
            else if (look_ahead.first == "do" || look_ahead.first == "while" || look_ahead.first == "for") {
                temp = new ParseTreeNode("Iterative_statement");
                parent->addChild(temp);
                Iterative_statement(temp);
            }
            else {
                throw std::runtime_error("Unexpected token in single_statement: " + look_ahead.first);
            }
        }

        string parse() {
            root = new ParseTreeNode("program");
            while (currentTokenIndex < tokens.size()) {
                if (isError) {
                    output->Text = "Parsing failed";
                    return "Empty Tree";
                }
                if (look_ahead.first == "Success" && look_ahead.second == "Tokens are finished") {
                    output->Text = "Parsing completed successfully";
                    string tree = root->buildTreeString();
                    root->generateGraphvizDotFile("parse_tree.dot");
                    return tree;
                }
                program(root);
            }
            }

        void program(ParseTreeNode* parent) {
            //program -> declarations function_definitions
            temp = new ParseTreeNode("declarations");
            parent->addChild(temp);
            declarations(temp);

            temp = new ParseTreeNode("function_definitions");
            parent->addChild(temp);
            function_definitions(temp);
        }


        void function_definitions(ParseTreeNode* parent) {
            // function_definitions -> function_definition function_definitions | ε
            if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "void" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
                if (look_ahead.first == "void" || tokens[currentTokenIndex + 2].first == "(" || tokens[currentTokenIndex + 3].first == "(") {
                    temp = new ParseTreeNode("function_definition");
                    parent->addChild(temp);
                    function_definition(temp);
                    temp = new ParseTreeNode("function_definitions");
                    parent->addChild(temp);
                    function_definitions(temp);
                }
            }
        }

        void declarations(ParseTreeNode* parent) {
            // enum_declaration | structure_declaration | array_declaration | pointer_declaration | variable_declaration | ε
            if (look_ahead.first == "enum") {
                temp = new ParseTreeNode("enum_declaration");
                parent->addChild(temp);
                enum_declaration(temp);
                return;
            }
            if (look_ahead.first == "struct") {
                temp = new ParseTreeNode("structure_declaration");
                parent->addChild(temp);
                structure_declaration(temp);
                return;
            }
            if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "void" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
                if (look_ahead.first == "void" || tokens[currentTokenIndex + 2].first == "(" || tokens[currentTokenIndex + 3].first == "(") {
                    return;
                }
                else if (tokens[currentTokenIndex + 2].first == "[" || tokens[currentTokenIndex + 3].first == "[") {
                    temp = new ParseTreeNode("array_declaration");
                    parent->addChild(temp);
                    array_declaration(temp);
                }
                else if (tokens[currentTokenIndex + 1].first == "*" || tokens[currentTokenIndex + 2].first == "*") {
                    temp = new ParseTreeNode("pointer_declaration");
                    parent->addChild(temp);
                    pointer_declaration(temp);
                }
                else {
                    temp = new ParseTreeNode("variable_declaration");
                    parent->addChild(temp);
                    variable_declaration(temp);
                }
            }

        }

        void expression(ParseTreeNode* parent) {
            string s = tokens[currentTokenIndex + 1].first;
            if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                boolean_expr(temp);
            }
            else if (s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                boolean_expr(temp);
            }
            else if (s == "=" || s == "+=" || s == "-=" || s == "*=" || s == "/=" || s == "%=" || s == "&=" || s == "|=" || s == "^=" || s == "<<=" || s == ">>=") {
                temp = new ParseTreeNode("assignment_expr");
                parent->addChild(temp);
                assignment_expr(temp);
            }
            else {
                temp = new ParseTreeNode("arithmetic_expr");
                parent->addChild(temp);
                arithmetic_expr(temp);
            }
        }

        void arithmetic_expr(ParseTreeNode* parent) {
            variable(parent);
            temp = new ParseTreeNode("sub_arithmetic_expr");
            parent->addChild(temp);
            sub_arithmetic_expr(temp);
        }

        void sub_arithmetic_expr(ParseTreeNode* parent) {
            if (look_ahead.first == "+" || look_ahead.first == "-" || look_ahead.first == "*" || look_ahead.first == "/" || look_ahead.first == "%" || look_ahead.first == "&" || look_ahead.first == "|" || look_ahead.first == "^" || look_ahead.first == "<<" || look_ahead.first == ">>") {
                temp = new ParseTreeNode("arithmetic_op");
                parent->addChild(temp);
                arithmetic_op(temp);
                temp = new ParseTreeNode("arithmetic_expr");
                parent->addChild(temp);
                arithmetic_expr(temp);
            }
        }

        void arithmetic_op(ParseTreeNode* parent) {
            // Arithmetic_op -> '+' | '-' | '*' | '/' | '%' | '&' | '|' | '^' | '<<' | '>>'
            if (look_ahead.first == "+" || look_ahead.first == "-" || look_ahead.first == "*" || look_ahead.first == "/" || look_ahead.first == "%" || look_ahead.first == "&" || look_ahead.first == "|" || look_ahead.first == "^" || look_ahead.first == "<<" || look_ahead.first == ">>") {
                try {
                    match(look_ahead.first, parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
                        String^ str = gcnew String(e.what());
                        error->Text = str;
                    isError = true;
                }
            }
        }

        void boolean_expr(ParseTreeNode* parent) {
            // Boolean_expr -> variable boolean_op variable | boolean_op variable
            if (look_ahead.first == "id" || look_ahead.first == "number") {
                variable(parent);
                temp = new ParseTreeNode("boolean_op");
                parent->addChild(temp);
                boolean_op(temp);
                variable(parent);
            }
            else {
                temp = new ParseTreeNode("boolean_op");
                parent->addChild(temp);
                boolean_op(temp);
                variable(parent);
            }
        }

        void boolean_op(ParseTreeNode* parent) {
            // Boolean_op -> '==' | '!=' | '>' | '<' | '>=' | '<=' | '!' | '&&' | '||'
            if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
                try {
                    match(look_ahead.first, parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void assignment_expr(ParseTreeNode* parent) {
            // assignment_expr -> variable assignment_op variable | variable assignment_op arithmetic_expr
            variable(parent);
            temp = new ParseTreeNode("assignment_op");
            parent->addChild(temp);
            assignment_op(temp);
            string s = tokens[currentTokenIndex + 1].first;
            if (s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                arithmetic_expr(temp);
            }
            else {
                variable(parent);
            }
            try {
                match(";", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void assignment_op(ParseTreeNode* parent) {
            // Assignment_op -> '='| '+=' | '-=' | '*=' | '/=' | '%='| '&='| '|='| '^='| '<<=' | '>>='
            if (look_ahead.first == "=" || look_ahead.first == "+=" || look_ahead.first == "-=" || look_ahead.first == "*=" || look_ahead.first == "/=" || look_ahead.first == "%=" || look_ahead.first == "&=" || look_ahead.first == "|=" || look_ahead.first == "^=" || look_ahead.first == "<<=" || look_ahead.first == ">>=") {
                try {
                    match(look_ahead.first, parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void variable(ParseTreeNode* parent) {
            // Variable -> number | id
            if (look_ahead.first == "number") {
                try {
                    match("number", parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
            else {
                try {
                    match("id", parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void function_call(ParseTreeNode* parent) {
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
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void arguments(ParseTreeNode* parent) {
            // arguments -> arg_expression | arg_expression , arguments | ε
            if (look_ahead.first == "id" || look_ahead.first == "number" || look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
                temp = new ParseTreeNode("arg_expression");
                parent->addChild(temp);
                arg_expression(temp);
                if (look_ahead.first == ",") {
                    try {
                        match(",", parent);
                    }
                    catch (exception& e) {
                         cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                        isError = true;

                    }
                    temp = new ParseTreeNode("arguments");
                    parent->addChild(temp);
                    arguments(temp);
                }
            }
            else {
                return;
            }
        }

        void arg_expression(ParseTreeNode* parent) {
            // arg_expression -> arithmetic_expr | boolean_expr
            string s = tokens[currentTokenIndex + 1].first;
            if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                boolean_expr(temp);
            }
            else if (s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                boolean_expr(temp);
            }
            else if (s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                arithmetic_expr(temp);
            }
            else {
                variable(parent);
            }
        }

        void return_statement(ParseTreeNode* parent) {
            // return_statement  -> return return_expr ;
            try {
                match("return", parent);
                temp = new ParseTreeNode("return_expr");
                parent->addChild(temp);
                return_expr(temp);
                match(";", temp);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void return_expr(ParseTreeNode* parent) {
            // return_expr -> arithmetic_expr | boolean_expr | 1 | 0 | ε
            string s = tokens[currentTokenIndex + 1].first;
            if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                boolean_expr(temp);
            }
            else if (s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                boolean_expr(temp);
            }
            else if (s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                arithmetic_expr(temp);
            }
            else if (look_ahead.first == "1" || look_ahead.first == "0") {
                try {
                    match(look_ahead.first, parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
            else {
                variable(parent);
            }
        }

        void Iterative_statement(ParseTreeNode* parent) {
            //Iterative_Statements-> for_loop | while_loop | do_while_loop
            if (look_ahead.first == "for") {
                temp = new ParseTreeNode("for_loop");
                parent->addChild(temp);
                for_loop(temp);
            }
            else if (look_ahead.first == "while") {
                temp = new ParseTreeNode("while_loop");
                parent->addChild(temp);
                while_loop(temp);
            }
            else if (look_ahead.first == "do") {
                temp = new ParseTreeNode("do_while_loop");
                parent->addChild(temp);
                do_while_loop(temp);
            }
        }

        void conditional_statements(ParseTreeNode* parent) {
            // conditional_statements -> if_expr | switch_expr
            if (look_ahead.first == "if") {
                temp = new ParseTreeNode("if_expr");
                parent->addChild(temp);
                if_expr(temp);
            }
            else if (look_ahead.first == "switch") {
                temp = new ParseTreeNode("switch_expr");
                parent->addChild(temp);
                switch_expr(temp);
            }
        }

        void body(ParseTreeNode* parent) {
            //body -> single_statement sub_body
            temp = new ParseTreeNode("single_statement");
            parent->addChild(temp);
            try {
                single_statement(temp);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
            temp = new ParseTreeNode("sub_body");
            parent->addChild(temp);
            sub_body(temp);
        }

        void sub_body(ParseTreeNode* parent) {
            // sub_body -> body | ε
            if (look_ahead.first == "if" || look_ahead.first == "while" || look_ahead.first == "for" || look_ahead.first == "switch" || look_ahead.first == "return" || look_ahead.first == "id" || look_ahead.first == "do" || look_ahead.first == "enum" || look_ahead.first == "struct" || look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned") {
                temp = new ParseTreeNode("body");
                parent->addChild(temp);
                body(temp);
            }
        }

        void for_loop(ParseTreeNode* parent) {
            // for_loop -> for (init_expr condition_expr; update_expr) { body}
            try {
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
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void init_expr(ParseTreeNode* parent) {
            // init_expr -> assignment_expr | variable_declaration  | ε
            if (look_ahead.first == "id") {
                temp = new ParseTreeNode("assignment_expr");
                parent->addChild(temp);
                assignment_expr(temp);
            }
            else if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "void" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
                temp = new ParseTreeNode("variable_declaration");
                parent->addChild(temp);
                variable_declaration(temp);
            }
            else {
                try {
                    match(";", parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;
                }
                return;
            }
        }

        void condition_expr(ParseTreeNode* parent) {
            // condition_expr -> boolean_expr | ε
            string s = tokens[currentTokenIndex + 1].first;
            if (s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                boolean_expr(temp);
            }
        }

        void update_expr(ParseTreeNode* parent) {
            // update_expr -> variable  assignment_op variable | variable assignment_op arithmetic_expr | ε
            try {
                variable(parent);
                temp = new ParseTreeNode("assignment_op");
                parent->addChild(temp);
                assignment_op(temp);
                string s = tokens[currentTokenIndex + 1].first;
                if (s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>") {
                    temp = new ParseTreeNode("arithmetic_expr");
                    parent->addChild(temp);
                    arithmetic_expr(temp);
                }
                else if (look_ahead.first == "id" || look_ahead.first == "number") {
                    variable(parent);
                }
                else {
                    return;
                }
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void while_loop(ParseTreeNode* parent) {
            // while_loop -> while (Condition_expr_while ) { body }
            try {
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
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void Condition_expr_while(ParseTreeNode* parent) {
            // Condition_expr_while -> boolean_expr | variable
            string s = tokens[currentTokenIndex + 1].first;
            if (s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||") {
                temp = new ParseTreeNode("boolean_expr");
                parent->addChild(temp);
                boolean_expr(temp);
            }
            else {
                try {
                    variable(parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;
                }
            }
        }

        void do_while_loop(ParseTreeNode* parent) {
            // do_while_loop -> do { body } while (Condition_expr_while );
            try {
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
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void if_expr(ParseTreeNode* parent) {
            // if_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
            try {
                match("if", parent);
                match("(", parent);
                string s = tokens[currentTokenIndex + 1].first;
                if (look_ahead.first == "==" || look_ahead.first == "!=" || look_ahead.first == ">" || look_ahead.first == "<" || look_ahead.first == ">=" || look_ahead.first == "<=" || look_ahead.first == "!" || look_ahead.first == "&&" || look_ahead.first == "||") {
                    temp = new ParseTreeNode("boolean_expr");
                    parent->addChild(temp);
                    boolean_expr(temp);
                }
                else if (s == "==" || s == "!=" || s == ">" || s == "<" || s == ">=" || s == "<=" || s == "!" || s == "&&" || s == "||") {
                    temp = new ParseTreeNode("boolean_expr");
                    parent->addChild(temp);
                    boolean_expr(temp);
                }
                else {
                    variable(parent);
                }
                match(")", parent);
                if (look_ahead.first == "{") {
                    match("{", parent);
                    temp = new ParseTreeNode("body");
                    parent->addChild(temp);
                    body(temp);
                    match("}", parent);
                }
                else {
                    temp = new ParseTreeNode("single_statement");
                    parent->addChild(temp);
                    try {
                        single_statement(temp);
                    }
                    catch (exception& e) {
                         cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                        isError = true;

                    }
                }
                temp = new ParseTreeNode("else_expr");
                parent->addChild(temp);
                else_expr(temp);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void else_expr(ParseTreeNode* parent) {
            // else_expr -> else {body} | else single_statement | ε
            try {
                if (look_ahead.first == "else") {
                    match("else", parent);
                    if (look_ahead.first == "{") {
                        match("{", parent);
                        temp = new ParseTreeNode("body");
                        parent->addChild(temp);
                        body(temp);
                        match("}", parent);
                    }
                    else {
                        temp = new ParseTreeNode("single_statement");
                        parent->addChild(temp);
                        try {
                            single_statement(temp);
                        }
                        catch (exception& e) {
                             cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                            isError = true;

                        }
                    }
                }
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void switch_expr(ParseTreeNode* parent) {
            // switch_expr -> switch(id) { case_expr default_expr }
            try {
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
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void case_expr(ParseTreeNode* parent) {
            // case_expr -> case const : body break; | case const : body | case_expr | ε
            while (look_ahead.first == "case") {
                try {
                    match("case", parent);
                    temp = new ParseTreeNode("const_expr");
                    parent->addChild(temp);
                    try {
                        const_expr(temp);
                    }
                    catch (exception& e) {
                         cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                        isError = true;

                    }
                    match(":", parent);
                    temp = new ParseTreeNode("body");
                    parent->addChild(temp);
                    body(temp);
                    if (look_ahead.first == "break") {
                        match("break", parent);
                        match(";", parent);
                    }
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void default_expr(ParseTreeNode* parent) {
            // default_expr -> default: body | default: body break;
            if (look_ahead.first == "default") {
                try {
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
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void const_expr(ParseTreeNode* parent) {
            // const -> number | string | char
            if (look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.second == "char") {
                try {
                    match(look_ahead.first, parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
            else {
                throw std::runtime_error("Unexpected token in const_expr: " + look_ahead.first);
            }
        }

        void variable_declaration(ParseTreeNode* parent) {
            // variable_declaration -> data_type variable_list ;
            ParseTreeNode* temp = new ParseTreeNode("data_type");
            parent->addChild(temp);
            data_type(temp);

            temp = new ParseTreeNode("variable_list");
            parent->addChild(temp);
            variable_list(temp);
            try {
                match(";", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void data_type(ParseTreeNode* parent) {
            // data_type -> type_modifier type
            temp = new ParseTreeNode("type_modifier");
            parent->addChild(temp);
            type_modifier(temp);

            temp = new ParseTreeNode("type");
            parent->addChild(temp);
            try {
                type(temp);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void type(ParseTreeNode* parent) {
            // type -> int | float | double | char | string | long | short | signed | unsigned
            if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" ||
                look_ahead.first == "char" || look_ahead.first == "string") {
                try {
                    match(look_ahead.first, parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
            else {
                throw std::runtime_error("Unexpected token in type: " + look_ahead.first);
            }
        }

        void type_modifier(ParseTreeNode* parent) {
            // type_modifier -> const | volatile | restrict | ε
            if (look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict" || look_ahead.first == "long" ||
                look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned") {
                try {
                    match(look_ahead.first, parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void variable_list(ParseTreeNode* parent) {
            // variable_list -> id equal_assign | id equal_assign, variable_list
            try {
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
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void equal_assign(ParseTreeNode* parent) {
            // equal_assign -> = const | = id | = arithmetic_expr |ε
            try {
                if (look_ahead.first == "=") {
                    match("=", parent);
                    string s = tokens[currentTokenIndex + 1].first;
                    if (s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>") {
                        temp = new ParseTreeNode("arithmetic_expr");
                        parent->addChild(temp);
                        arithmetic_expr(temp);
                    }
                    else if (look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.second == "char") {
                        temp = new ParseTreeNode("const_expr");
                        parent->addChild(temp);
                        const_expr(temp);
                    }
                    else {
                        match("id", parent);
                    }
                }
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void function_definition(ParseTreeNode* parent) {
            // function_definition -> function_header { body }
            temp = new ParseTreeNode("function_header");
            parent->addChild(temp);
            function_header(temp);

            try {
                match("{", parent);
                temp = new ParseTreeNode("body");
                parent->addChild(temp);
                body(temp);
                match("}", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void function_header(ParseTreeNode* parent) {
            // function_header -> function_datatype id ( parameter_list )
            temp = new ParseTreeNode("function_datatype");
            parent->addChild(temp);
            function_datatype(temp);

            try {
                match("id", parent);
                match("(", parent);

                temp = new ParseTreeNode("parameter_list");
                parent->addChild(temp);
                parameter_list(temp);

                match(")", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void function_datatype(ParseTreeNode* parent) {
            // function_datatype -> data_type | void
            if (look_ahead.first == "void") {
                try {
                    match("void", parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
            else {
                temp = new ParseTreeNode("data_type");
                parent->addChild(temp);
                data_type(temp);
            }
        }

        void parameter_list(ParseTreeNode* parent) {
            // parameter_list -> parameter sub_parameter_list
            if (look_ahead.first == ")") {
                return;
            }
            temp = new ParseTreeNode("parameter");
            parent->addChild(temp);
            parameter(temp);

            if (look_ahead.first == ")") {
                return;
            }

            temp = new ParseTreeNode("sub_parameter_list");
            parent->addChild(temp);
            sub_parameter_list(temp);
        }

        void sub_parameter_list(ParseTreeNode* parent) {
            // sub_parameter_list -> , parameter sub_parameter_list | ε
            if (look_ahead.first == ",") {
                try {
                    match(",", parent);
                    temp = new ParseTreeNode("parameter");
                    parent->addChild(temp);
                    parameter(temp);

                    temp = new ParseTreeNode("sub_parameter_list");
                    parent->addChild(temp);
                    sub_parameter_list(temp);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void parameter(ParseTreeNode* parent) {
            // parameter -> data_type id | ε
            if (look_ahead.first == ")") {
                return;
            }
            else {
                temp = new ParseTreeNode("data_type");
                parent->addChild(temp);
                data_type(temp);
                try {
                    match("id", parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }
            }
        }

        void array_declaration(ParseTreeNode* parent) {
            // array_declaration -> data_type id array_dimensions
            temp = new ParseTreeNode("data_type");
            parent->addChild(temp);
            data_type(temp);
            try {
                match("id", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
            temp = new ParseTreeNode("array_dimensions");
            parent->addChild(temp);
            array_dimensions(temp);
            try {
                match(";", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void array_dimensions(ParseTreeNode* parent) {
            // array_dimensions -> [ variable ] | [ variable ] array_dimensions
            try {
                match("[", parent);
                variable(parent);
                match("]", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }

            if (look_ahead.first == "[") {
                temp = new ParseTreeNode("array_dimensions");
                parent->addChild(temp);
                array_dimensions(temp);
            }
        }

        void pointer_declaration(ParseTreeNode* parent) {
            // pointer_declaration -> data_type variable_list_point ;
            temp = new ParseTreeNode("data_type");
            parent->addChild(temp);
            data_type(temp);
            temp = new ParseTreeNode("variable_list_point");
            parent->addChild(temp);
            variable_list_point(temp);
            try {
                match(";", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void variable_list_point(ParseTreeNode* parent) {
            // variable_list_point -> *variable_point | *variable_point, variable_list_point
            try {
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
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void variable_point(ParseTreeNode* parent) {
            // variable_point -> id | id = &id
            try {
                if (look_ahead.first == "id") {
                    match("id", parent);
                    if (look_ahead.first == "=") {
                        match("=", parent);
                        match("&", parent);
                        match("id", parent);
                    }
                }
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void enum_declaration(ParseTreeNode* parent) {
            // enum_declaration -> enum id { enum_constants } ;
            try {
                match("enum", parent);

                match("id", parent);

                match("{", parent);

                temp = new ParseTreeNode("enum_constants");
                parent->addChild(temp);
                enum_constants(temp);

                match("}", parent);
                match(";", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void enum_constants(ParseTreeNode* parent) {
            // enum_constants -> enum_constant | enum_constant , enum_constants
            temp = new ParseTreeNode("enum_constant");
            parent->addChild(temp);
            enum_constant(temp);

            if (look_ahead.first == ",") {
                try {
                    match(",", parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;

                }

                temp = new ParseTreeNode("enum_constants");
                parent->addChild(temp);
                enum_constants(temp);
            }
        }

        void enum_constant(ParseTreeNode* parent) {
            // enum_constant -> id
            try {
                match("id", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;

            }
        }

        void structure_declaration(ParseTreeNode* parent) {
            // structure_declaration -> struct id { member_list } ;
            try {
                match("struct", parent);
                match("id", parent);
                match("{", parent);

                temp = new ParseTreeNode("member_list");
                parent->addChild(temp);
                member_list(temp);

                match("}", parent);
                match(";", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                isError = true;
            }
        }

        void member_list(ParseTreeNode* parent) {
            // member_list -> member | member ; member_list | ε
            if (look_ahead.first == "}") return;
            temp = new ParseTreeNode("member");
            parent->addChild(temp);
            member(temp);

            if (look_ahead.first == ";") {
                try {
                    match(";", parent);
                }
                catch (exception& e) {
                     cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;
                    isError = true;
                }
                temp = new ParseTreeNode("member_list");
                parent->addChild(temp);
                member_list(temp);
            }
        }

        void member(ParseTreeNode* parent) {
            // member -> data_type id equal_assign
            temp = new ParseTreeNode("data_type");
            parent->addChild(temp);
            data_type(temp);
            try {
                match("id", parent);
            }
            catch (exception& e) {
                 cout << e.what() << endl;
 String^ str = gcnew String(e.what());
 error->Text = str;

                isError = true;
            }
            temp = new ParseTreeNode("equal_assign");
            parent->addChild(temp);
            equal_assign(temp);
        }

        




    private: System::Void updateDictionbutton_Click(System::Object^ sender, System::EventArgs^ e) {
        // Open a file dialog for the user to select a text file
        OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
        openFileDialog->Filter = "Text files|*.txt|All files (*.*)|*.*";
        openFileDialog->FilterIndex = 1;
        openFileDialog->RestoreDirectory = true;

        if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
        {
            // Get the selected file path
            String^ filePath = openFileDialog->FileName;

            // Convert System::String^ to std::string
            std::string filePathStd = msclr::interop::marshal_as<std::string>(filePath);

            // Read the content of the file and store it in sourceCode
            std::ifstream input(filePathStd);
            std::string line;
            std::string sourceCode;
            while (std::getline(input, line))
            {
                sourceCode += line + "\n"; // Concatenate each line with a newline character
            }

            // Extract preprocessors, remove comments, and extra spaces
            string preprocessors = extractPreprocessors(sourceCode);
            string codeWithoutComments = removeComments(sourceCode);
            string cleanCode = removeExtraSpaces(codeWithoutComments);

            // Clear previous lexemes

            // Analyze code and store the lexemes
            vector<pair<string, string>> tokens = analyzeCode(cleanCode);
            

            // Update the label to show the number of lexemes
            label3->Text = "Dictionary updated. Total lexemes: " + lexemes.size();

            // Display the lexical analysis result in a RichTextBox
            Form^ form = gcnew Form();
            form->Text = "Lexical Analysis Result";
            form->Size = System::Drawing::Size(600, 400); // Adjusted size for better visibility

            RichTextBox^ richTextBox = gcnew RichTextBox();
            richTextBox->Dock = DockStyle::Fill;

            // Format the lexical analysis result for better clarity
            String^ result = gcnew String("Lexeme\t\tToken\n");
            auto lexIt = lexemes.begin();
            for (const auto& token : tokens) {
                if (token.second != "") {
                    result += gcnew String((*lexIt + "\t\t<" + token.first + ", " + token.second + ">\n").c_str());
                }
                else {
                    result += gcnew String((*lexIt + "\t\t<" + token.first + ">\n").c_str());
                }
                ++lexIt;
            }
            // Add errors to the result
            result += gcnew String("\nErrors\n");
            for (const auto& error : errors) {
                result += gcnew String((error + "\n").c_str());
            }

            printSymbolTable(symbolTableVector);
            richTextBox->Text = result;

            richTextBox->ReadOnly = true;
            richTextBox->ScrollBars = RichTextBoxScrollBars::Both;

            form->Controls->Add(richTextBox);
            form->Show();
        }
        
    }

  private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {

      // Extract preprocessors, remove comments, and extra spaces
      string preprocessors = extractPreprocessors(msclr::interop::marshal_as<std::string>(textBox1->Text));
      string codeWithoutComments = removeComments(msclr::interop::marshal_as<std::string>(textBox1->Text));
      string cleanCode = removeExtraSpaces(codeWithoutComments);

      // Analyze code and display the result
      vector<pair<string, string>> token = analyzeCode(cleanCode);
      // Update tokens and look_ahead
      tokens = token;
      currentTokenIndex = 0;
      look_ahead = tokens[currentTokenIndex];
      String^ result = gcnew String("Lexeme\t\tToken\n");
      auto lexIt = lexemes.begin();
      for (const auto& token : tokens) {
          if (lexIt == lexemes.end()) {
              throw std::runtime_error("Mismatch between tokens and lexemes sizes");
          }

          if (!token.second.empty()) {
              result += gcnew String((*lexIt + "\t\t<" + token.first + ", " + token.second + ">\n").c_str());
          }
          else {
              result += gcnew String((*lexIt + "\t\t<" + token.first + ">\n").c_str());
          }
          lexIt++;
      }
      // Add errors to the result
      result += gcnew String("\nErrors\n");
      for (const auto& error : errors) {
          result += gcnew String((error + "\n").c_str());
      }

      // Display the lexical analysis result in a RichTextBox within a form
      Form^ form1 = gcnew Form();
      form1->Text = "Lexical Analysis Result";
      form1->Size = System::Drawing::Size(600, 400); // Adjusted size for better visibility

      RichTextBox^ richTextBox1 = gcnew RichTextBox();
      richTextBox1->Dock = DockStyle::Fill;
      richTextBox1->Text = result;
      richTextBox1->ReadOnly = true;
      richTextBox1->ScrollBars = RichTextBoxScrollBars::Both;

      form1->Controls->Add(richTextBox1);
      form1->Show(); // Changed from ShowDialog to Show

      // Create a new form for displaying the tree structure
      string tree = parse();
      Form^ treeForm = gcnew Form();
      treeForm->Text = "Parse Tree";
      treeForm->Size = System::Drawing::Size(600, 400); // Adjusted size for better visibility

      RichTextBox^ treeRichTextBox = gcnew RichTextBox();
      treeRichTextBox->Dock = DockStyle::Fill;
      treeRichTextBox->Text = gcnew String(tree.c_str()); // Display the tree string
      treeRichTextBox->ReadOnly = true;
      treeRichTextBox->ScrollBars = RichTextBoxScrollBars::Both;

      treeForm->Controls->Add(treeRichTextBox);
      treeForm->Show();
      printSymbolTable(symbolTableVector);
     
  }


       void printSymbolTable(const vector<pair<string, string>>& symbolTable) {
           // Create a new form
           Form^ form2 = gcnew Form();
           form2->Text = "Symbol Table";
           form2->Size = System::Drawing::Size(600, 400); // Adjusted size for better visibility

           // Create a new RichTextBox
           RichTextBox^ richTextBox2 = gcnew RichTextBox();
           richTextBox2->Dock = DockStyle::Fill;

           // Format the symbol table for better clarity
           String^ table = gcnew String("Identifier\t\tIndex\n");
           for (const auto& entry : symbolTable) {
               table += gcnew String((entry.first + "\t\t" + entry.second + "\n").c_str());
           }
           richTextBox2->Text = table;

           richTextBox2->ReadOnly = true;
           richTextBox2->ScrollBars = RichTextBoxScrollBars::Both;

           // Add the RichTextBox to the form
           form2->Controls->Add(richTextBox2);

           // Show the form
           form2->Show(); // Changed from ShowDialog to Show
       }




    };
}
