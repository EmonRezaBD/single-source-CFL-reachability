#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include<iostream>
#include<vector>
#include<string>
#include<set>
#include <utility>
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

void dfs(int from, int current, string path, const unordered_map<int, vector<pair<char, int>>>& edges) {
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

__device__ bool is_member_CUDA(const char* d_strings, int numStrings, int stringLength, const char* target) {

    int left = 0, right = numStrings - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        const char* midStr = &d_strings[mid * stringLength];

        int cmp = strncmp(midStr, target, stringLength);
        if (cmp == 0) return true;
        else if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return false;
}

__global__ void checkMembershipKernel(
    const pair<int, int>* cfgVertices,
    const char* cfgStrings,
    int stringLength,
    bool* isMemberResults,
    int numEntries)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < numEntries) {
        const char* str = &cfgStrings[idx * stringLength];
        isMemberResults[idx] = is_member(str);
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

   /* for (const auto& entry : possibleCFGStrings) //Converting this in CUDA
    {
        if (is_member(entry.second)) {
            reacheableVertices.push_back({ entry.first.first, entry.first.second });
        }
    }*/

    int numEntries = possibleCFGStrings.size();
    int stringLength = 0;
    if (numEntries > 0) {
        stringLength = possibleCFGStrings[0].second.size();
    }
    // Allocate and copy cfgVertices and cfgStrings to GPU
    pair<int, int>* d_cfgVertices;
    char* d_cfgStrings;
    bool* d_isMemberResults;

    cudaMalloc(&d_cfgVertices, numEntries * sizeof(pair<int, int>));
    cudaMalloc(&d_cfgStrings, numEntries * stringLength * sizeof(char));
    cudaMalloc(&d_isMemberResults, numEntries * sizeof(bool));

    pair<int, int>* h_cfgVertices = new pair<int, int>[numEntries];
    char* h_cfgStrings = new char[numEntries * stringLength];

    for (int i = 0; i < numEntries; ++i) {
        h_cfgVertices[i] = possibleCFGStrings[i].first;
        memcpy(&h_cfgStrings[i * stringLength], possibleCFGStrings[i].second.c_str(), stringLength);
    }

    cudaMemcpy(d_cfgVertices, h_cfgVertices, numEntries * sizeof(pair<int, int>), cudaMemcpyHostToDevice);
    cudaMemcpy(d_cfgStrings, h_cfgStrings, numEntries * stringLength * sizeof(char), cudaMemcpyHostToDevice);

    // Copy results back to CPU
    bool* h_isMemberResults = new bool[numEntries];
    cudaMemcpy(h_isMemberResults, d_isMemberResults, numEntries * sizeof(bool), cudaMemcpyDeviceToHost);

    // Process the results
    for (int i = 0; i < numEntries; ++i) {
        if (h_isMemberResults[i]) {
            reacheableVertices.push_back(h_cfgVertices[i]);
        }
    }

    // Free GPU memory
    cudaFree(d_cfgVertices);
    cudaFree(d_cfgStrings);
    cudaFree(d_isMemberResults);

    delete[] h_cfgVertices;
    delete[] h_cfgStrings;
    delete[] h_isMemberResults;

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
