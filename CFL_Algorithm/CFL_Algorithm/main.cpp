#include<iostream>
#include<vector>
#include<string>
#include<set>
#include<unordered_set>
#include <functional>
#include<time.h>

#include"analysis.h"

unordered_set<string>generatedStringsUnique;
vector<pair<int, int>> reacheableVertices;
vector<pair<pair<int, int>, string>> possibleCFGStrings;

using namespace std;


bool is_member(const string& str)
{
    if (generatedStringsUnique.count(str)) {
        return true;
    }
    else {
        return false; 
    }
}

void dfs(int from, int current, string path, const unordered_map<int, vector<pair<char, int>>>&edges) {
    // Add the current path to the result
    if (from != current) {
        possibleCFGStrings.push_back({ {from, current}, path });
    }

    // Explore neighbors
    if (edges.find(current) != edges.end()) {
        for (const auto& edge : edges.at(current)) {
            char label = edge.first;
            int to = edge.second;
            dfs(from, to, path + label, edges);  // Recursive DFS call
        }
    }
}


void CFL_reachable(const CAnalysis& graphObj)
{
    unordered_map<int, vector<pair<char, int>>> edges = graphObj.getEdges();

    for (const auto& entry : edges)
    {
        int from = entry.first;
        dfs(from, from, "", edges);
    }

    for (const auto& entry : possibleCFGStrings)
    {
        if (is_member(entry.second)) {
            reacheableVertices.push_back({ entry.first.first, entry.first.second });
        }
    }

}


int main() {

    cout << "Program Starts\n\n";
    string graphFile = "input_graph.txt";
    string CGFFile = "CFG.txt";

    //construct graph
    CAnalysis obj;
    obj.contructGraph(graphFile);

    //generating strings
    string start = "S";
    int maxDepth = 7;// Maximum recursion depth to limit the length of generated strings

    unordered_map<char, vector<string>> productions;  // Stores the production rules
    string startSymbol;  // Start symbol (usually 'S')

    obj.parseGrammar(CGFFile, productions, startSymbol);
   
    //vector<string> generated_All_Possible_Strings;
    //obj.generateStrings(productions, startSymbol, 0, maxDepth, generatedStrings);

    clock_t parse_start, parse_end;
    parse_start = clock();
    obj.generateStrings(productions, startSymbol, 0, maxDepth, generatedStringsUnique);
    parse_end = clock();

    printf("String Generation time : %4.6f sec\n",
        (double)((double)(parse_end - parse_start) / CLOCKS_PER_SEC));

    //run DFS

    /*cout << "Generated Strings:" << endl;
    for (const string& str : generatedStringsUnique) {
        cout << str << endl;
    }*/

    //DFS
    clock_t analysis_start, analysis_end;
    analysis_start = clock();
    CFL_reachable(obj);
    analysis_end = clock();

    printf("Analysis time : %4.6f sec\n",
        (double)((double)(analysis_end - analysis_start) / CLOCKS_PER_SEC));

    cout << "All vertices CFL-reachable from s: \n";
    for (const auto& pair : reacheableVertices) {
        cout << "(" << pair.first << ", " << pair.second << ")" << endl;
    }

    cout << "\nProgram Ends\n";


    return 0;
}
