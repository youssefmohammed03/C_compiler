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



    using namespace std;

    // Regular expressions for keywords and datatypes
    const unordered_map<string, string> keywordRegex = {
        {"alignas", "alignas"},
        {"alignof", "alignof"},
        {"auto", "auto"},
        {"bool", "bool"},
        {"break", "break"},
        {"case", "case"},
        {"const", "const"},
        {"constexpr", "constexpr"},
        {"continue", "continue"},
        {"default", "default"},
        {"do", "do"},
        {"else", "else"},
        {"enum", "enum"},
        {"extern", "extern"},
        {"false", "false"},
        {"for", "for"},
        {"goto", "goto"},
        {"if", "if"},
        {"inline", "inline"},
        {"register", "register"},
        {"restrict", "restrict"},
        {"return", "return"},
        {"signed", "signed"},
        {"sizeof", "sizeof"},
        {"static", "static"},
        {"static_assert", "static_assert"},
        {"struct", "struct"},
        {"switch", "switch"},
        {"thread_local", "thread_local"},
        {"true", "true"},
        {"typedef", "typedef"},
        {"typeof", "typeof"},
        {"typeof_unqual", "typeof_unqual"},
        {"union", "union"},
        {"unsigned", "unsigned"},
        {"void", "void"},
        {"volatile", "volatile"},
        {"while", "while"},
        {"_Alignas", "_Alignas"},
        {"_Alignof", "_Alignof"},
        {"_Atomic", "_Atomic"},
        {"_Bool", "_Bool"},
        {"_Complex", "_Complex"},
        {"_Decimal128", "_Decimal128"},
        {"_Decimal32", "_Decimal32"},
        {"_Decimal64", "_Decimal64"},
        {"_Generic", "_Generic"},
        {"_Imaginary", "_Imaginary"},
        {"_Noreturn", "_Noreturn"},
        {"_Static_assert", "_Static_assert"},
        {"_Thread_local", "_Thread_local"},
    };

    const unordered_set<string> dataTypes = {
        "char",
        "double",
        "float",
        "int",
        "long",
        "short",
        "signed",
        "unsigned",
        "void",
    };

    // Function to extract arithmetic expressions
    vector<string> extractArithExpressions(const string& code) {
        vector<string> expressions;
        regex pattern("(\\+\\+|\\-\\-|\\+|\\-|\\*|\\/|\\%|\\~|\\<\\<|\\>\\>|\\^|([^\\&]|^)\\&([^\\&]|$)|([^\\|]|^)\\|([^\\|]|$))");

        sregex_iterator it(code.begin(), code.end(), pattern);
        sregex_iterator it2;

        while (it != it2) {
            expressions.push_back(it->str());
            ++it;
        }

        return expressions;
    }

    // Function to extract boolean expressions
    vector<string> extractBoolExpressions(const string& code) {
        vector<string> expressions;
        regex pattern("(\\=\\=|\\!\\=|\\<\\=|\\>\\=|([^\\>]|^)\\>([^\\>]|$)|([^\\<]|^)\\<([^\\<]|$)|\\!|\\&\\&|\\|\\|)");

        sregex_iterator it(code.begin(), code.end(), pattern);
        sregex_iterator it2;

        while (it != it2) {
            expressions.push_back(it->str());
            ++it;
        }

        return expressions;
    }

    // Function to remove comments from code
    string removeComments(string code) {
        regex commentPattern("(\\/\\*([^*]|[\r\n]|(\\*+([^*/]|[\r\n])))*\\*\\/)|(\\/\\/.*)|#[^\\n]*");
        return regex_replace(code, commentPattern, "");
    }

    // Function to remove extra spaces from code
    string removeExtraSpaces(string code) {
        regex spacePattern("\\s+");
        return regex_replace(code, spacePattern, " ");
    }

    // Function to extract preprocessors from code
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

    // Function to perform lexical analysis
    map<string, string> performLexicalAnalysis(const string& sourceCode) {
        regex pattern("\\b\\w+\\b|\\S");
        smatch match;
        string::const_iterator searchStart(sourceCode.cbegin());
        regex assignment_operators("(\\+=|==|=)");

        map<string, string> results;

        while (regex_search(searchStart, sourceCode.cend(), match, pattern)) {
            string word = match[0];
            if (keywordRegex.find(word) != keywordRegex.end()) {
                results[word] = "<" + keywordRegex.at(word) + ">";
            }
            else if (dataTypes.count(word) > 0) {
                results[word] = "<" + word + ">";
            }
            else {
                regex punctuatorRegex("[;(){}]");
                if (regex_search(word, punctuatorRegex)) {
                    results[word] = word;
                }
                else if (word == "=" || word == "+=" || word == "==") {
                    results[word] = "<" + word + ">";
                }
                else {
                    results[word] = "<" + word + ">";
                }
            }
            // Move the search start position to avoid infinite loops
            searchStart = match.suffix().first;
        }

        // Extract arithmetic expressions
        vector<string> arithExpressions = extractArithExpressions(sourceCode);
        cout << "\nArithmetic Expressions:" << endl;
        for (const auto& expression : arithExpressions) {
            cout << expression << endl;
        }

        // Extract boolean expressions
        vector<string> boolExpressions = extractBoolExpressions(sourceCode);
        cout << "\nBoolean Expressions:" << endl;
        for (const auto& expression : boolExpressions) {
            cout << expression << endl;
        }
        return results;
    }


	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

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
	private: System::Windows::Forms::Label^ label1;
	protected:
	private: System::Windows::Forms::TextBox^ textBox1;
	private: System::Windows::Forms::PictureBox^ updateDictionbutton;
	private: System::Windows::Forms::Label^ label2;
    private: System::Windows::Forms::Button^ button1;
    private: System::Windows::Forms::Label^ label3;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(MyForm::typeid));
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->textBox1 = (gcnew System::Windows::Forms::TextBox());
            this->updateDictionbutton = (gcnew System::Windows::Forms::PictureBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->label3 = (gcnew System::Windows::Forms::Label());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->updateDictionbutton))->BeginInit();
            this->SuspendLayout();
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Font = (gcnew System::Drawing::Font(L"Old Antic Decorative", 22.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(178)));
            this->label1->Location = System::Drawing::Point(166, 38);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(461, 74);
            this->label1->TabIndex = 0;
            this->label1->Text = L"Lexical Analyzer";
            // 
            // textBox1
            // 
            this->textBox1->Location = System::Drawing::Point(228, 156);
            this->textBox1->Name = L"textBox1";
            this->textBox1->Size = System::Drawing::Size(324, 22);
            this->textBox1->TabIndex = 1;
            this->textBox1->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox1_TextChanged);
            // 
            // updateDictionbutton
            // 
            this->updateDictionbutton->Anchor = System::Windows::Forms::AnchorStyles::None;
            this->updateDictionbutton->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"updateDictionbutton.Image")));
            this->updateDictionbutton->Location = System::Drawing::Point(22, 12);
            this->updateDictionbutton->Name = L"updateDictionbutton";
            this->updateDictionbutton->Size = System::Drawing::Size(68, 76);
            this->updateDictionbutton->TabIndex = 12;
            this->updateDictionbutton->TabStop = false;
            this->updateDictionbutton->Click += gcnew System::EventHandler(this, &MyForm::updateDictionbutton_Click);
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(225, 137);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(110, 16);
            this->label2->TabIndex = 13;
            this->label2->Text = L"Input your C code";
            // 
            // button1
            // 
            this->button1->Location = System::Drawing::Point(321, 209);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(130, 46);
            this->button1->TabIndex = 14;
            this->button1->Text = L"Generate";
            this->button1->UseVisualStyleBackColor = true;
            this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(318, 275);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(44, 16);
            this->label3->TabIndex = 15;
            this->label3->Text = L"label3";
            // 
            // MyForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->BackColor = System::Drawing::SystemColors::ScrollBar;
            this->ClientSize = System::Drawing::Size(799, 635);
            this->Controls->Add(this->label3);
            this->Controls->Add(this->button1);
            this->Controls->Add(this->label2);
            this->Controls->Add(this->updateDictionbutton);
            this->Controls->Add(this->textBox1);
            this->Controls->Add(this->label1);
            this->Name = L"MyForm";
            this->Text = L"MyForm";
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->updateDictionbutton))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
	private: System::Void updateDictionbutton_Click(System::Object^ sender, System::EventArgs^ e) {
		OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
		openFileDialog->Filter = "Text files|*.txt|All files (*.*)|*.*";
		openFileDialog->FilterIndex = 1;
		openFileDialog->RestoreDirectory = true;

		if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			String^ filePath = openFileDialog->FileName;
			//errorEmpty->Text = "Selected File: " + filePath;

			// Convert System::String^ to std::string and update the file path
			std::string filePathStd = msclr::interop::marshal_as<std::string>(filePath);
            std::ifstream input(filePathStd);
            //map<string, string> lexicalResults = performLexicalAnalysis(sourceCode);

		}
	}
    private: System::Void textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e) {

       
    }
    private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
        String^ input = textBox1->Text;
        std::string inputStd = msclr::interop::marshal_as<std::string>(input);
        map<string, string> lexicalResults = performLexicalAnalysis(inputStd);

        // Construct a formatted string to display lexical results
        std::ostringstream formattedResult;
        formattedResult << std::left << std::setw(20) << "Lexeme" << "Token" << std::endl;
        formattedResult << std::setfill('-') << std::setw(40) << "-" << std::setfill(' ') << std::endl;
        for (const auto& pair : lexicalResults) {
            formattedResult << std::left << std::setw(20) << pair.first << pair.second << std::endl;
        }

        // Convert the formatted result to System::String
        String^ resultString = gcnew String(formattedResult.str().c_str());

        // Display the result in label3
        label3->Text = resultString;
    }

};
}
