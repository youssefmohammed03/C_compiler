#include <iostream>
#include <regex>
#include <string>
using namespace std;

vector<string> keywords;
vector<string> dataTypes;
vector<string> arithExpressions;
vector<string> boolExpressions;
vector<string> assignmentStatements;
vector<string> punctuations;
vector<string> errors;

regex keywordPattern("\\b(alignas|alignof|auto|bool|break|case|const|constexpr|continue|default|do|else|enum|extern|false|for|goto|if|inline|register|restrict|return|signed|sizeof|static|static_assert|struct|switch|thread_local|true|typedef|typeof|typeof_unqual|union|unsigned|void|volatile|while|_Alignas|_Alignof|_Atomic|_Bool|_Complex|_Decimal128|_Decimal32|_Decimal64|_Generic|_Imaginary|_Noreturn|_Static_assert|_Thread_local)\\b");
regex dataTypePattern("\\b(char|double|float|int|long|short|signed|unsigned|void)\\b");
regex arithPattern("(\\+\\+|\\-\\-|\\+|\\-|\\*|\\/|\\%|\\~|\\<\\<|\\>\\>|\\^|([^\\&]|^)\\&([^\\&]|$)|([^\\|]|^)\\|([^\\|]|$))");
regex boolPattern("(\\=\\=|\\!\\=|\\<\\=|\\>\\=|([^\\>]|^)\\>([^\\>]|$)|([^\\<]|^)\\<([^\\<]|$)|\\!|\\&\\&|\\|\\|)");
regex assignmentPattern("((\\+=)|(-=)|(\\*=)|(\\/=)|(\\%=)|(&=)|(\\|=)|(\\^=)|(<<=)|(>>=)|(=))");
regex punctuationPattern(R"([{}()\[\];,.<>:])");

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

void processToken(const string& temp) {
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
    } else {
        errors.push_back(temp);
    }
    cout << temp << endl; // Print temp
}

void analyzeCode(const string& code) {
    string separators = "(){}[].,;+-*/%~<>^&|!=:";
    string temp;
    for (int i = 0; i < code.size(); ++i) {
        char c = code[i];
        if (separators.find(c) != string::npos) {
            if (!temp.empty()) {
                processToken(temp);
                temp.clear();
            }
            temp += c;
            string multiCharOp;
            string twoCharOps[] = {"||", "&&", "<=", ">=", "==", "!=", "<<", ">>", "++", "--", "-=", "+=", "*=", "/=", "%=", "&=", "|=", "^="};
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
            processToken(temp);
            temp.clear();
        } else if (c == ' ') {
            if (!temp.empty()) {
                processToken(temp);
                temp.clear();
            }
        } else {
            temp += c;
        }
    }

    // Handle the last token if it's not empty
    if (!temp.empty()) {
        processToken(temp);
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

int main(){
    // This is a single line comment
    int a=5, b=10;
    int sum=a+b; // Arithmetic operation
    printf(sum);

    /* This is a
       multi-line comment */
    if (a<b){ // Boolean expression
        printf();
    }else{
        printf();
    }

    // For loop
    for (int i=0;i<5;i++){
        printf(i);
    }

    a<<=c;
    a==f
    x!=e

    return 0;
})";

string preprocessors = extractPreprocessors(code);
string noComments = removeComments(code);
string noExtraSpaces = removeExtraSpaces(noComments);

analyzeCode(noExtraSpaces);
return 0;

}

/*
1- string in the sample code handle
2- numbers decimal, octal, hexa, binary
3- check for any missing types to handle
4- how to return the tokens
*/
