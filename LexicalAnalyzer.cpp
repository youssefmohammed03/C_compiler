#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
using namespace std;

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
regex decimal_regex("^[-+]?[1-9][0-9]\\.?[0-9]$");
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
            } else {
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

void printSymbolTable(const vector<pair<string, string>>& symbolTable) {
    cout << endl << setw(15) << left << "Identifier" << setw(25) << "Index" << endl;
    cout << "------------------------" << endl;

    for (const auto& entry : symbolTable) {
        cout << setw(15) << left << entry.first << setw(25) << entry.second << endl;
    }
    cout << endl;
}

int main() {
    string code = R"(#include <iostream>
    #include <vector>

    /* This is a
    multi-line comment */

    // This is a single line comment

    int main(){
        // Variable declarations
        int a=10,b=20;
        float c=3.14;
        char d='d';
        std::string str="Hello, \"World!";
        std::vector<int> vec={1,2,3,4,5};

        // Arithmetic operations
        int sum=a+b;
        int diff=a-b;
        int prod=a*b;
        int quot=b/a;
        int rem=b%a;

        // Boolean expressions
        bool less=a<b;
        bool greater = a > b;
        bool equal = a==b;
        bool notEqual = a != b;

        // Control structures
        if (a <b) {
            printf("a is less than b\n");
        } else {
            printf("a is greater than or equal to b\n");
        }

        for (int i = 0; i < 5; i++) {
            printf("i = %d\n", i);
        }

        int hex = 0x1AB32;
        int oct = 0123;
        int bin = 0b1010;
        int dec = 123;
        float f = 1.23;
        double d = 1.23;

        // Bitwise operations
        a<<=1;
        b >>= 1;

        return 0;

        12
        0x12abc
        0b1010
        0123
        1.23
        -1.23
        int 123abc;
        0123
        myname3343434

    })";

    string preprocessors = extractPreprocessors(code);
    string noComments = removeComments(code);
    string noExtraSpaces = removeExtraSpaces(noComments);

    vector<pair<string, string>> tokens = analyzeCode(noExtraSpaces);

    printTokens(tokens);

    printSymbolTable(symbolTableVector);

    printErrors();

    printLexemes();


    return 0;
}