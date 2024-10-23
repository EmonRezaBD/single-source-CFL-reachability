#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <queue>


using namespace std;

class CAnalysis {

public:
	unordered_map<int, vector<pair<char, int>>> graph; //to store graph
	vector<string> allPossibleStr;

	string trim(const string& str);
	vector<string> split(const string& str, char delimiter);
	void parseGrammar(const string& filename, unordered_map<char, vector<string>>& productions, string& startSymbol);
	void generateStrings(const unordered_map<char, vector<string>>&productions, string current, int depth, int maxDepth, /*vector<string>& result*/ unordered_set<string>& results);
	unordered_map<int, vector<pair<char, int>>> contructGraph(const string& filename); //load graph from the file
	const unordered_map<int, vector<pair <char, int>>>& getEdges() const;
};
