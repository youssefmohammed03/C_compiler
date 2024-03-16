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

using namespace std;

namespace ScannerGUI2 {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;

    vector<string> lexemes;

    regex keywordPattern("\\b(alignas|alignof|auto|bool|break|case|const|constexpr|continue|default|do|else|enum|extern|false|for|goto|if|inline|register|restrict|return|signed|sizeof|static|static_assert|struct|switch|thread_local|true|typedef|typeof|typeof_unqual|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Decimal128|_Decimal32|_Decimal64|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local)\\b");
    regex dataTypePattern("\\b(char|double|float|int|long|short|signed|unsigned|void)\\b");
    regex arithPattern("(\\+\\+|\\-\\-|\\+|\\-|\\*|\\/|\\%|\\~|\\<\\<|\\>\\>|\\^|([^\\&]|^)\\&([^\\&]|$)|([^\\|]|^)\\|([^\\|]|$))");
    regex boolPattern("(\\=\\=|\\!\\=|\\<\\=|\\>\\=|([^\\>]|^)\\>([^\\>]|$)|([^\\<]|^)\\<([^\\<]|$)|\\!|\\&\\&|\\|\\|)");
    regex assignmentPattern("((\\+=)|(-=)|(\\*=)|(\\/=)|(\\%=)|(&=)|(\\|=)|(\\^=)|(<<=)|(>>=)|(=))");
    regex punctuationPattern(R"([{}()\[\];,.:])");

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

    void punctuationDetector(const string& temp, vector<pair<string, string>>& tokens) {
        if (temp == "{") {
            tokens.push_back(make_pair(temp, "start curly bracket"));
        }
        else if (temp == "}") {
            tokens.push_back(make_pair(temp, "end curly bracket"));
        }
        else if (temp == "(") {
            tokens.push_back(make_pair(temp, "start parenthesis"));
        }
        else if (temp == ")") {
            tokens.push_back(make_pair(temp, "end parenthesis"));
        }
        else if (temp == "[") {
            tokens.push_back(make_pair(temp, "start square bracket"));
        }
        else if (temp == "]") {
            tokens.push_back(make_pair(temp, "end square bracket"));
        }
        else if (temp == ";") {
            tokens.push_back(make_pair(temp, "semicolon"));
        }
        else if (temp == ",") {
            tokens.push_back(make_pair(temp, "comma"));
        }
        else if (temp == ".") {
            tokens.push_back(make_pair(temp, "dot"));
        }
        else if (temp == ":") {
            tokens.push_back(make_pair(temp, "colon"));
        }
    }

    void processToken(const string& temp, vector<pair<string, string>>& tokens) {
        if (regex_match(temp, keywordPattern)) {
            tokens.push_back(make_pair(temp, "keyword"));
        }
        else if (regex_match(temp, dataTypePattern)) {
            tokens.push_back(make_pair(temp, "data type"));
        }
        else if (regex_match(temp, arithPattern)) {
            tokens.push_back(make_pair(temp, "arithmetic expression"));
        }
        else if (regex_match(temp, boolPattern)) {
            tokens.push_back(make_pair(temp, "boolean expression"));
        }
        else if (regex_match(temp, assignmentPattern)) {
            tokens.push_back(make_pair(temp, "assignment statement"));
        }
        else if (regex_match(temp, punctuationPattern)) {
            punctuationDetector(temp, tokens);
        }
        else {
            tokens.push_back(make_pair(temp, "identifier"));
        }
        lexemes.push_back(temp);
    }

    void twoCharOps(string& temp, const string& code, int& i) {
        string multiCharOp;
        string twoCharOps[] = { "||", "&&", "<=", ">=", "==", "!=", "<<", ">>", "++", "--", "-=", "+=", "*=", "/=", "%=", "&=", "|=", "^=", "::", "->" };
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

        if (number != "0") {
            while (i + 1 < code.size() && (isdigit(code[i + 1]) || code[i + 1] == '.')) {
                number += code[++i];
            }
            tokens.push_back(make_pair(number, "decimal number"));
        }
        else {
            if (i + 1 < code.size() && isdigit(code[i + 1])) {
                while (i + 1 < code.size() && isdigit(code[i + 1])) {
                    number += code[++i];
                }
                tokens.push_back(make_pair(number, "octal number"));
            }
            else if (i + 1 < code.size() && (code[i + 1] == 'x' || code[i + 1] == 'X')) {
                number += code[++i];
                while (i + 1 < code.size() && isxdigit(code[i + 1])) {
                    number += code[++i];
                }
                tokens.push_back(make_pair(number, "hexadecimal number"));
            }
            else if (i + 1 < code.size() && (code[i + 1] == 'b' || code[i + 1] == 'B')) {
                number += code[++i];
                while (i + 1 < code.size() && (code[i + 1] == '0' || code[i + 1] == '1')) {
                    number += code[++i];
                }
                tokens.push_back(make_pair(number, "binary number"));
            }
        }
    }

    vector<pair<string, string>> analyzeCode(const string& code) {
        vector<pair<string, string>> tokens;
        string separators = "(){}[].,;+-*/%~<>^&|!=:\"\'";
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
                    tokens.push_back(make_pair(temp, "character"));
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
            else if (isdigit(c) && (temp.empty() || isdigit(temp[0]))) {
                temp += c;
                numbersDetector(temp, code, i, tokens);
                temp.clear();
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
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->textBox1 = (gcnew System::Windows::Forms::TextBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->updateDictionbutton = (gcnew System::Windows::Forms::Button());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->panel1->SuspendLayout();
            this->SuspendLayout();
            // 
            // panel1
            // 
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
            this->textBox1->Size = System::Drawing::Size(566, 273);
            this->textBox1->TabIndex = 1;
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(23, 28);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(267, 17);
            this->label2->TabIndex = 0;
            this->label2->Text = L"Enter the code below to analyze its lexemes";
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
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(26, 366);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(0, 17);
            this->label3->TabIndex = 4;
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
             lexemes.clear();

             // Analyze code and store the lexemes
             vector<pair<string, string>> tokens = analyzeCode(cleanCode);
             for (const auto& token : tokens) {
                 lexemes.push_back(token.first);
             }

             // Update the label to show the number of lexemes
             label3->Text = "Dictionary updated. Total lexemes: " + lexemes.size();

             // Display the lexical analysis result in a RichTextBox
             Form^ form = gcnew Form();
             form->Text = "Lexical Analysis Result";
             form->Size = System::Drawing::Size(600, 400); // Adjusted size for better visibility

             RichTextBox^ richTextBox = gcnew RichTextBox();
             richTextBox->Dock = DockStyle::Fill;

             // Format the lexical analysis result for better clarity
             String^ result = gcnew String("Lexemes\t\tTokens\n");
             for (const auto& token : tokens) {
                 result += gcnew String((token.first + "\t\t" + token.second + "\n").c_str());
             }
             richTextBox->Text = result;

             richTextBox->ReadOnly = true;
             richTextBox->ScrollBars = RichTextBoxScrollBars::Both;

             form->Controls->Add(richTextBox);
             form->ShowDialog();
         }
     }

private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
    // Extract preprocessors, remove comments, and extra spaces
    string preprocessors = extractPreprocessors(msclr::interop::marshal_as<std::string>(textBox1->Text));
    string codeWithoutComments = removeComments(msclr::interop::marshal_as<std::string>(textBox1->Text));
    string cleanCode = removeExtraSpaces(codeWithoutComments);

    // Analyze code and display the result
    vector<pair<string, string>> tokens = analyzeCode(cleanCode);
    String^ result = gcnew String("Lexemes\t\tTokens\n");
    for (const auto& token : tokens) {
        result += gcnew String((token.first + "\t\t" + token.second + "\n").c_str());
    }

    // Display the lexical analysis result in a RichTextBox within a form
    Form^ form = gcnew Form();
    form->Text = "Lexical Analysis Result";
    form->Size = System::Drawing::Size(600, 400); // Adjusted size for better visibility

    RichTextBox^ richTextBox = gcnew RichTextBox();
    richTextBox->Dock = DockStyle::Fill;
    richTextBox->Text = result;
    richTextBox->ReadOnly = true;
    richTextBox->ScrollBars = RichTextBoxScrollBars::Both;

    form->Controls->Add(richTextBox);
    form->ShowDialog();
}



    };
}
