#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
using namespace std;

class SyntaxAnalyzer {

public:
vector<pair<string, string>> tokens;

public:
    SyntaxAnalyzer(vector<pair<string, string>> tokens) {
    this->tokens = tokens;
    }
};