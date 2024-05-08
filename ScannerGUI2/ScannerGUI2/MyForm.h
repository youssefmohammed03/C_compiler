﻿#pragma once

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
#include "ParserTreeNode.h"

using namespace std;

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
            this->output = (gcnew System::Windows::Forms::Label());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->updateDictionbutton = (gcnew System::Windows::Forms::Button());
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->textBox1 = (gcnew System::Windows::Forms::TextBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->error = (gcnew System::Windows::Forms::Label());
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
            // output
            // 
            this->output->AutoSize = true;
            this->output->Location = System::Drawing::Point(34, 300);
            this->output->Name = L"output";
            this->output->Size = System::Drawing::Size(44, 16);
            this->output->TabIndex = 5;
            this->output->Text = L"label1";
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
            // error
            // 
            this->error->AutoSize = true;
            this->error->ForeColor = System::Drawing::Color::Red;
            this->error->Location = System::Drawing::Point(34, 329);
            this->error->Name = L"error";
            this->error->Size = System::Drawing::Size(44, 16);
            this->error->TabIndex = 5;
            this->error->Text = L"label1";
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
                   parent->addChild(temp);
                   look_ahead = getNextToken();
               }
               else {
                   throw std::runtime_error("Unexpected token: " + look_ahead.first + ". Expected: " + expectedToken);
               }
           }

           void single_statement(ParseTreeNode* parent) {
               if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned" || look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
                   temp = new ParseTreeNode("variable_declaration");
                   parent->addChild(temp);
                   variable_declaration(temp);
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
               else {
                   throw std::runtime_error("Unexpected token in single_statement: " + look_ahead.first);
               }
           }

           void parse() {
               root = new ParseTreeNode("main");
               while (currentTokenIndex < tokens.size()) {
                   if (look_ahead.first == "Success" && look_ahead.second == "Tokens are finished") {
                       output->Text = "Parsing completed successfully";
                       root->printTree();
                       return;
                   }
                   single_statement(root);
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
                   catch (exception e) {
                       cout << e.what() << endl;
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
                   catch (exception e) {
                       cout << e.what() << endl;
                   }
               }
           }

           void assignment_expr(ParseTreeNode* parent) {
               // assignment_expr -> variable assignment_op variable | variable assignment_op arithmetic_expr | variable assignment_op boolean_expr
               variable(parent);
               temp = new ParseTreeNode("assignment_op");
               parent->addChild(temp);
               assignment_op(temp);
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
                   if (s == "+" || s == "-" || s == "*" || s == "/" || s == "%" || s == "&" || s == "|" || s == "^" || s == "<<" || s == ">>") {
                       temp = new ParseTreeNode("boolean_expr");
                       parent->addChild(temp);
                       arithmetic_expr(temp);
                   }
                   else {
                       variable(parent);
                   }
               }
               match(";", parent);
           }

           void assignment_op(ParseTreeNode* parent) {
               // Assignment_op -> '='| '+=' | '-=' | '*=' | '/=' | '%='| '&='| '|='| '^='| '<<=' | '>>='
               if (look_ahead.first == "=" || look_ahead.first == "+=" || look_ahead.first == "-=" || look_ahead.first == "*=" || look_ahead.first == "/=" || look_ahead.first == "%=" || look_ahead.first == "&=" || look_ahead.first == "|=" || look_ahead.first == "^=" || look_ahead.first == "<<=" || look_ahead.first == ">>=") {
                   try {
                       match(look_ahead.first, parent);
                   }
                   catch (exception e) {
                       cout << e.what() << endl;
                   }
               }
           }

           void variable(ParseTreeNode* parent) {
               // Variable -> number | id
               if (look_ahead.first == "number") {
                   try {
                       match("number", parent);
                   }
                   catch (exception e) {
                       cout << e.what() << endl;
                   }
               }
               else if (look_ahead.first == "id") {
                   try {
                       match("id", parent);
                   }
                   catch (exception e) {
                       cout << e.what() << endl;
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
                   catch (exception e) {
                       cout << e.what() << endl;
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
                       match(",", parent);
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
               match("return", parent);
               temp = new ParseTreeNode("return_expr");
               parent->addChild(temp);
               return_expr(temp);
               match(";", temp);
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
                   match(look_ahead.first, parent);
               }
               else {
                   variable(parent);
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
               single_statement(temp);
               temp = new ParseTreeNode("sub_body");
               parent->addChild(temp);
               sub_body(temp);
           }

           void sub_body(ParseTreeNode* parent) {
               // sub_body -> body | ε
               //int | float | double | char | string | long | short | signed | unsigned
               if (look_ahead.first == "if" || look_ahead.first == "while" || look_ahead.first == "for" || look_ahead.first == "switch" || look_ahead.first == "return" || look_ahead.first == "id" || look_ahead.first == "do" || look_ahead.first == "enum" || look_ahead.first == "struct" || look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" || look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" || look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned") {
                   temp = new ParseTreeNode("body");
                   parent->addChild(temp);
                   body(temp);
               }
           }

           void if_expr(ParseTreeNode* parent) {
               // if_expr -> if (boolean_expr) {body} else_expr | if (boolean_expr) single_statement else_expr
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
                   single_statement(temp);
               }
               temp = new ParseTreeNode("else_expr");
               parent->addChild(temp);
               else_expr(temp);
           }

           void else_expr(ParseTreeNode* parent) {
               // else_expr -> else {body} | else single_statement | ε
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
                       single_statement(temp);
                   }
               }
           }

           void switch_expr(ParseTreeNode* parent) {
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

           void case_expr(ParseTreeNode* parent) {
               // case_expr -> case const : body break; | case const : body | case_expr | ε
               while (look_ahead.first == "case") {
                   match("case", parent);
                   temp = new ParseTreeNode("const_expr");
                   parent->addChild(temp);
                   const_expr(temp);
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

           void default_expr(ParseTreeNode* parent) {
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

           void const_expr(ParseTreeNode* parent) {
               // const -> number | string | char
               if (look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.second == "char") {
                   match(look_ahead.first, parent);
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

               match(";", parent);
           }

           void data_type(ParseTreeNode* parent) {
               // data_type -> type_modifier type
               ParseTreeNode* temp = new ParseTreeNode("type_modifier");
               parent->addChild(temp);
               type_modifier(temp);

               temp = new ParseTreeNode("type");
               parent->addChild(temp);
               type(temp);
           }

           void type(ParseTreeNode* parent) {
               // type -> int | float | double | char | string | long | short | signed | unsigned
               if (look_ahead.first == "int" || look_ahead.first == "float" || look_ahead.first == "double" ||
                   look_ahead.first == "char" || look_ahead.first == "string" || look_ahead.first == "long" ||
                   look_ahead.first == "short" || look_ahead.first == "signed" || look_ahead.first == "unsigned") {
                   match(look_ahead.first, parent);
               }
               else {
                   throw std::runtime_error("Unexpected token in type: " + look_ahead.first);
               }
           }

           void type_modifier(ParseTreeNode* parent) {
               // type_modifier -> const | volatile | restrict | ε
               if (look_ahead.first == "const" || look_ahead.first == "volatile" || look_ahead.first == "restrict") {
                   match(look_ahead.first, parent);
               }
           }

           void variable_list(ParseTreeNode* parent) {
               // variable_list -> id equal_assign | id equal_assign, variable_list
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

           void equal_assign(ParseTreeNode* parent) {
               // equal_assign -> = const | = id | ε
               if (look_ahead.first == "=") {
                   match("=", parent);
                   if (look_ahead.first == "number" || look_ahead.second == "string" || look_ahead.second == "char") {
                       temp = new ParseTreeNode("const_expr");
                       parent->addChild(temp);
                       const_expr(temp);
                   }
                   else {
                       match("id", parent);
                   }
               }
           }

private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
    // Extract preprocessors, remove comments, and extra spaces
    string preprocessors = extractPreprocessors(msclr::interop::marshal_as<std::string>(textBox1->Text));
    string codeWithoutComments = removeComments(msclr::interop::marshal_as<std::string>(textBox1->Text));
    string cleanCode = removeExtraSpaces(codeWithoutComments);

    // Analyze code and display the result
    vector<pair<string, string>> token = analyzeCode(cleanCode);
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

    // Display the symbol table in a separate form
    printSymbolTable(symbolTableVector);
    tokens = token;
    currentTokenIndex = 0;
    look_ahead = tokens[currentTokenIndex];
    parse();
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
