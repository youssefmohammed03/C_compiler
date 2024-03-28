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

    vector<string> errors;
    vector<string> lexemes;
    vector<pair<string, string>> symbolTableVector;
    int counter = 0;

    regex keywordPattern("\\b(alignas|alignof|auto|bool|break|case|const|constexpr|continue|default|do|else|enum|extern|false|for|goto|if|inline|register|restrict|return|signed|sizeof|static|static_assert|struct|switch|thread_local|true|typedef|typeof|typeof_unqual|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Decimal128|_Decimal32|_Decimal64|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local)\\b");
    regex dataTypePattern("\\b(char|double|float|int|long|short|signed|unsigned|void)\\b");
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
            tokens.push_back(make_pair(number, "decimal number"));
            lexemes.push_back(number);
        }
        else if (regex_match(number, binary_regex)) {
            tokens.push_back(make_pair(number, "binary number"));
            lexemes.push_back(number);
        }
        else if (regex_match(number, octal_regex)) {
            tokens.push_back(make_pair(number, "octal number"));
            lexemes.push_back(number);
        }
        else if (regex_match(number, hex_regex)) {
            tokens.push_back(make_pair(number, "hexadecimal number"));
            lexemes.push_back(number);
        }
        else {
            errors.push_back(number);
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
    vector<pair<string, string>> tokens = analyzeCode(cleanCode);
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
