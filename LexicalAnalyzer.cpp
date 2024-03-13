#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
using namespace std;

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

void analyzeCode(const string& code) {
    vector<string> keywords;
    vector<string> dataTypes;
    vector<string> arithExpressions;
    vector<string> boolExpressions;
    vector<string> errors;
    vector<string> assignmentStatements;
    vector<string> punctuations;

    regex keywordPattern("\\b(alignas|alignof|auto|bool|break|case|const|constexpr|continue|default|do|else|enum|extern|false|for|goto|if|inline|register|restrict|return|signed|sizeof|static|static_assert|struct|switch|thread_local|true|typedef|typeof|typeof_unqual|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Decimal128|_Decimal32|_Decimal64|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local)\\b");
    regex dataTypePattern("\\b(char|double|float|int|long|short|signed|unsigned|void)\\b");
    regex arithPattern("(\\+\\+|\\-\\-|\\+|\\-|\\*|\\/|\\%|\\~|\\<\\<|\\>\\>|\\^|([^\\&]|^)\\&([^\\&]|$)|([^\\|]|^)\\|([^\\|]|$))");
    regex boolPattern("(\\=\\=|\\!\\=|\\<\\=|\\>\\=|([^\\>]|^)\\>([^\\>]|$)|([^\\<]|^)\\<([^\\<]|$)|\\!|\\&\\&|\\|\\|)");
    regex assignmentPattern("((\\+=)|(-=)|(\\*=)|(\\/=)|(\\%=)|(&=)|(\\|=)|(\\^=)|(<<=)|(>>=)|(=))");
    regex punctuationPattern(R"([{}()\[\];,.<>:])");

    string temp;
    for (char c : code) {
        if (c != ' ') {
            temp += c;
        } else {
            if (regex_match(temp, keywordPattern)) {
                keywords.push_back(temp);
            } else if (regex_match(temp, dataTypePattern)) {
                dataTypes.push_back(temp);
            } else if (regex_match(temp, arithPattern)) {
                arithExpressions.push_back(temp);
            } else if (regex_match(temp, boolPattern)) {
                boolExpressions.push_back(temp);
            } else if (regex_match(temp, assignmentPattern)) {
                assignmentStatements.push_back(temp);
            } else if (regex_match(temp, punctuationPattern)) {
                punctuations.push_back(temp);
            } else if (!temp.empty()) {
                errors.push_back(temp);
            }
            temp.clear();
        }
    }

    // Print the vectors
    cout << "Keywords:\n";
    for (const auto& keyword : keywords) {
        cout << keyword << "\n";
    }

    cout << "\nData Types:\n";
    for (const auto& dataType : dataTypes) {
        cout << dataType << "\n";
    }

    cout << "\nArithmetic Expressions:\n";
    for (const auto& expr : arithExpressions) {
        cout << expr << "\n";
    }

    cout << "\nBoolean Expressions:\n";
    for (const auto& expr : boolExpressions) {
        cout << expr << "\n";
    }

    cout << "\nAssignment Statements:\n";
    for (const auto& stmt : assignmentStatements) {
        cout << stmt << "\n";
    }

    cout << "\nPunctuations:\n";
    for (const auto& punc : punctuations) {
        cout << punc << "\n";
    }

    cout << "\nErrors:\n";
    for (const auto& error : errors) {
        cout << error << "\n";
    }
}

int main(){
    string code = R"(#include <stdio.h>

int main ( ) {
    // This is a single line comment
    int a = 5 , b = 10 ;
    int sum = a + b ; // Arithmetic operation
    printf ( sum ) ;

    /* This is a
       multi-line comment */
    if ( a < b ) { // Boolean expression
        printf ( ) ;
    } else {
        printf ( ) ;
    }

    // For loop
    for ( int i = 0 ; i < 5 ; i ++ ) {
        printf ( i ) ;
    }
    return 0 ;
} )";

string preprocessors = extractPreprocessors(code);
string noComments = removeComments(code);
string noExtraSpaces = removeExtraSpaces(noComments);

analyzeCode(noExtraSpaces);
return 0;

}