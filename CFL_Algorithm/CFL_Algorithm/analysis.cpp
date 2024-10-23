#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <queue>
#include<unordered_set>

#include"analysis.h"

using namespace std;

vector<string> CAnalysis::split(const string& str, char delimiter) 
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

string CAnalysis::trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

void CAnalysis::parseGrammar(const string& filename, unordered_map<char, vector<string>>& productions, string& startSymbol) {
    ifstream file(filename);
    string line;

    bool isProductionSection = false;

    while (getline(file, line)) {
        line = trim(line);

        if (line.empty()) continue;

        if (line.find("ALPHABET") != string::npos || line.find("VARIABLES") != string::npos) {
            // Skip Alphabet and Variable
            continue;
        }
        else if (line.find("PRODUCTIONS") != string::npos) {
            // Reading production rules
            isProductionSection = true;
        }
        else if (isProductionSection) {
            // Read production rules in the form: A->B
            vector<string> parts = split(line, '-');
            if (parts.size() == 2 && parts[1][0] == '>') {
                char variable = trim(parts[0])[0];  // Variable on the left-hand side
                string rules = trim(parts[1].substr(1));  // Right-hand side of the rule

                // Split the right-hand side by the '|' symbol to get multiple productions
                vector<string> ruleParts = split(rules, '|');
                for (string rule : ruleParts) {
                    productions[variable].push_back(trim(rule));
                }

                // Set the start symbol as the first production's variable
                if (startSymbol.empty()) {
                    startSymbol = variable;
                }
            }
        }
    }

    file.close();
}

// Function to generate all possible strings up to a certain depth
void CAnalysis::generateStrings(const unordered_map<char, vector<string>>& productions, string current, int depth, int maxDepth, /*vector<string>& result*/ unordered_set<string>& result) {
    if (depth > maxDepth) {
        return;
    }

    // Check if the string has any variables (non-terminals) left to expand
    bool hasVariable = false;
    for (char c : current) {
        if (productions.find(c) != productions.end()) {
            hasVariable = true;
            break;
        }
    }

    // If no variables are left, add the current string to the result
    if (!hasVariable) {
        //result.push_back(current);
        result.insert(current);
        return;
    }

    // Expand each variable according to its production rules
    for (size_t i = 0; i < current.size(); ++i) {
        char symbol = current[i];
        if (productions.find(symbol) != productions.end()) {
            for (const string& prod : productions.at(symbol)) {
                string newString = current.substr(0, i) + prod + current.substr(i + 1);
                generateStrings(productions, newString, depth + 1, maxDepth, result);
            }
        }
    }
}

unordered_map<int, vector<pair<char, int>>> CAnalysis::contructGraph(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Can't open file: " + filename);
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        int from, to;
        char label;
        if (!(iss >> from >> label >> to)) {
            throw runtime_error("Invalid: " + line);
        }
        graph[from].push_back({ label, to });
    }

    return graph;
}

const unordered_map<int, vector<pair<char, int>>>& CAnalysis::getEdges() const {
    return graph;
}