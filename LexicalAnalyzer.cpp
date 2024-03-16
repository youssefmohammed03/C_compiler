#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <vector>
using namespace std;

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
    if(temp == "{"){
        tokens.push_back(make_pair(temp, "start curly bracket"));
    } else if(temp == "}"){
        tokens.push_back(make_pair(temp, "end curly bracket"));
    } else if(temp == "("){
        tokens.push_back(make_pair(temp, "start parenthesis"));
    } else if(temp == ")"){
        tokens.push_back(make_pair(temp, "end parenthesis"));
    } else if(temp == "["){
        tokens.push_back(make_pair(temp, "start square bracket"));
    } else if(temp == "]"){
        tokens.push_back(make_pair(temp, "end square bracket"));
    } else if(temp == ";"){
        tokens.push_back(make_pair(temp, "semicolon"));
    } else if(temp == ","){
        tokens.push_back(make_pair(temp, "comma"));
    } else if(temp == "."){
        tokens.push_back(make_pair(temp, "dot"));
    } else if(temp == ":"){
        tokens.push_back(make_pair(temp, "colon"));
    }
}

void processToken(const string& temp, vector<pair<string, string>>& tokens) {
    if (regex_match(temp, keywordPattern)) {
        tokens.push_back(make_pair(temp, "keyword"));
    } else if (regex_match(temp, dataTypePattern)) {
        tokens.push_back(make_pair(temp, "data type"));
    } else if (regex_match(temp, arithPattern)) {
        tokens.push_back(make_pair(temp, "arithmetic expression"));
    } else if (regex_match(temp, boolPattern)) {
        tokens.push_back(make_pair(temp, "boolean expression"));
    } else if (regex_match(temp, assignmentPattern)) {
        tokens.push_back(make_pair(temp, "assignment statement"));
    } else if (regex_match(temp, punctuationPattern)) {
        punctuationDetector(temp, tokens);
    } else {
        tokens.push_back(make_pair(temp, "identifier"));
    }
    lexemes.push_back(temp);
}

void twoCharOps(string& temp, const string& code, int& i) {
    string multiCharOp;
    string twoCharOps[] = {"||", "&&", "<=", ">=", "==", "!=", "<<", ">>", "++", "--", "-=", "+=", "*=", "/=", "%=", "&=", "|=", "^=", "::"};
    string threeCharOps[] = {"<<=", ">>="};

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
            if(c == '\"'){
                int j = i + 1;
                while (j < code.size() && code[j] != '\"') {
                    temp += code[j++];
                }
                if (j < code.size()) {
                    temp += code[j++];
                }
                i = j - 1;
                tokens.push_back(make_pair(temp, "string"));
            } else if(c == '\''){
                int j = i + 1;
                while (j < code.size() && code[j] != '\'') {
                    temp += code[j++];
                }
                if (j < code.size()) {
                    temp += code[j++];
                }
                i = j - 1;
                tokens.push_back(make_pair(temp, "character"));
            } else{
                twoCharOps(temp, code, i);
                processToken(temp, tokens);

            }
            temp.clear();
        } else if (c == ' ') {
            if (!temp.empty()) {
                processToken(temp, tokens);
                temp.clear();
            }
        } else {
            temp += c;
        }
    }

    if (!temp.empty()) {
        processToken(temp, tokens);
    }

    return tokens;
}

int main(){
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
        std::string str="Hello, World!";
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

        // Bitwise operations
        a<<=1;
        b >>= 1;

        return 0;
    })";

    string preprocessors = extractPreprocessors(code);
    string noComments = removeComments(code);
    string noExtraSpaces = removeExtraSpaces(noComments);

    vector<pair<string, string>> tokens = analyzeCode(noExtraSpaces);

    cout << "Lexeme\t\tToken\n";
    for (const auto& token : tokens) {
    cout << token.first << "\t\t" << token.second << "\n";
}

    return 0;
}

/*
- numbers decimal, octal, hexa, binary
- check for any missing types to handle
*/
