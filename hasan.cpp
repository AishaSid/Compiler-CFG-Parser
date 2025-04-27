#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Grammar {
    map<string, vector<string>> productions;
    map<string, int> nonter_count;
};

string START_STATE = "";

Grammar readCFG(const string& filename) {
    Grammar cfg;
    ifstream file(filename);
    string line;
    int done = 0;
    while (getline(file, line)) {
        istringstream iss(line);
        string lhs, arrow, rhs;
        iss >> lhs >> arrow;
        getline(iss, rhs);
        istringstream rhsStream(rhs);
        string production;
        while (getline(rhsStream, production, '|')) {
            cfg.productions[lhs].push_back(production);
        }
        for (const auto& rule : cfg.productions) {
            cfg.nonter_count[rule.first] = 0;
            if (done == 0) {
                START_STATE = rule.first;
                done = 1;
            }
        }
        
    }
    return cfg;
}



map<string, set<string>> computeFirstSets(const Grammar& cfg) {
    map<string, set<string>> firstSets;
    bool changed;
    
    do {
        changed = false;
        for (const auto& rule : cfg.productions) {
            string A = rule.first;
            for (const string& prod : rule.second) {
                string firstSymbol = "";
                int index = 1;
                while (firstSymbol == "") {
                    while (prod[index] != ' ' && index < prod.size()) {
                        firstSymbol += prod[index];
                        index++;
                    }
                    //cout << "firstSymbol = " << firstSymbol << endl;
                    if (firstSymbol == "^" && prod != " ^ ") {
                        firstSymbol = "";
                        index += 1;
                    }
                }
                bool isTerminal = true;
                for (const auto& rule_rule : cfg.productions) {
                    if (rule_rule.first == firstSymbol) {
                        isTerminal = false;
                        break;
                    }
                }
                if (isTerminal) {
                    if (firstSets[A].insert(firstSymbol).second) {
                        changed = true;
                    }
                } else {
                    for (const string& f : firstSets[firstSymbol]) {
                        if (firstSets[A].insert(f).second) {
                            changed = true;
                        }
                    }
                }
            }
        }
    } while (changed);
    
    return firstSets;
}



map<string, set<string>> computeFollowSets(const Grammar& cfg,  map<string, set<string>>& firstSets) {
    map<string, set<string>> followSets;
    followSets[START_STATE].insert("$");
    bool changed = true;
    
    do {
        changed = false;
        for (const auto& rule : cfg.productions) {
            string A = rule.first;
            //cout << A << " exists in:\n";
            for (const auto& rule2 : cfg.productions){
                vector<string> A_exists;
                for (const string& prod : rule2.second) {
                    int index = 1;
                    string search_A = "";
                    //cout << "prod = " << prod << endl;
                    while (search_A == "" && index < prod.size()) {
                        while (prod[index] != ' ' && index < prod.size()) {
                            search_A += prod[index];
                            index++;
                        }
                        if (search_A == A){
                            //cout << prod << endl;
                            A_exists.push_back(prod);
                            break;
                        } else {
                            search_A = "";
                            index += 1;
                        }

                    }
                }

                for (size_t i = 0; i < A_exists.size(); ++i) {
                    
                    int index = 1;
                    string search_A = "";
                    string current_A_exists = A_exists[i];
                    //cout << "prod = " << prod << endl;
                    while (search_A == "" && index < current_A_exists.size()) {
                        while (current_A_exists[index] != ' ' && index < current_A_exists.size()) {
                            search_A += current_A_exists[index];
                            index++;
                        }
                        if (search_A == A){
                            //cout << current_A_exists << endl;
                            index+=1;
                            break;
                        } else {
                            search_A = "";
                            index += 1;
                        }

                    }
                    //cout << index << current_A_exists[index]<< current_A_exists.size() << endl;
                    if (index >= current_A_exists.size()) {
                        //if (A == "A\'\'")
                        //cout << "Follow(" << A << ") = Follow(" << rule2.first << ")\n" << index << current_A_exists[index] << current_A_exists.size()  << endl;
                        for (const string& f : followSets[rule2.first]) {
                            if (followSets[A].insert(f).second) {
                                changed = true;
                            }
                        }
                    } else {
                        string nextSymbol = "";
                        while (current_A_exists[index] != ' ' && index < current_A_exists.size()) {
                            nextSymbol += current_A_exists[index];
                            index++;
                        }
                        //cout << "nextSymbol = " << nextSymbol << endl;
                        bool isTerminal = true;
                        for (const auto& rule_rule : cfg.productions) {
                            if (rule_rule.first == nextSymbol) {
                                isTerminal = false;
                                break;
                            }
                        }
                        if (isTerminal) {
                            if (followSets[A].insert(nextSymbol).second) {
                                changed = true;
                            }
                        } else {
                            for (const string& f : firstSets[nextSymbol]) {
                                if (followSets[A].insert(f).second) {
                                    changed = true;
                                }
                                if (f == "^") {
                                    for (const string& f2 : followSets[nextSymbol]) {
                                        if (followSets[A].insert(f2).second) {
                                            changed = true;
                                        }
                                    }
                                }
                            }
                        }



                    }


                    
                }




            }
        }
        //cout <<"----------------------------------------------------------------------\n";
    } while (changed);
    for (auto &rule: followSets){
        auto it = rule.second.find("^");
        if (it != rule.second.end()) {
            rule.second.erase(it);
        }
    }
    return followSets;
}


vector<string> computeSingularFirstSets(const Grammar& cfg, string A_param, string prod_param) {
    map<string, set<string>> firstSets;
    vector<string> return_firstSet;
    bool changed;
    
    do {
        changed = false;
        for (const auto& rule : cfg.productions) {
            string A = rule.first;
            for (const string& prod : rule.second) {
                string firstSymbol = "";
                int index = 1;
                while (firstSymbol == "") {
                    while (prod[index] != ' ' && index < prod.size()) {
                        firstSymbol += prod[index];
                        index++;
                    }
                    //cout << "firstSymbol = " << firstSymbol << endl;
                    if (firstSymbol == "^" && prod != " ^ ") {
                        firstSymbol = "";
                        index += 1;
                    }
                }
                bool isTerminal = true;
                for (const auto& rule_rule : cfg.productions) {
                    if (rule_rule.first == firstSymbol) {
                        isTerminal = false;
                        break;
                    }
                }
                if (isTerminal) {
                    if (firstSets[A].insert(firstSymbol).second) {
                        if (A_param == A && prod_param == prod)
                            return_firstSet.push_back(firstSymbol);
                        changed = true;
                    }
                } else {
                    for (const string& f : firstSets[firstSymbol]) {
                        if (firstSets[A].insert(f).second) {
                            if (A_param == A && prod_param == prod)
                                return_firstSet.push_back(f);
                            changed = true;
                        }
                    }
                }
            }
        }
    } while (changed);
    
    return return_firstSet;
}
void constructLL1Table(const Grammar& cfg, const map<string, set<string>>& firstSets, const map<string, set<string>>& followSets) {
    bool parsing_error = false;
    map<string, map<string, string>> parsingTable;
    vector<string> terminals;
    
    for (const auto& rule : cfg.productions) {
        string A = rule.first;
        //cout << "Rule " << A << ":\n";
        for (const string& prod : rule.second) {
            //cout << "Prod " << prod << ":\n";
            if (prod == " ^ ") {
                
                if (followSets.find(A) != followSets.end() && !followSets.at(A).empty()){
                    for (const string& f : followSets.at(A)) {
                        if (parsingTable[A].find(f) != parsingTable[A].end()) {
                            //cout << "Grammar is not LL(1)!\n";
                            parsing_error = true;
                            parsingTable[A][f] = parsingTable[A][f] + " ,, " + prod; 
                            
                        } 
                        else {
                            auto it = find(terminals.begin(), terminals.end(), f);
                            if (it == terminals.end()) {
                                //cout << "Adding terminal " << f << endl;
                                terminals.push_back(f);
                            }
                            parsingTable[A][f] = prod;
                        }
                    }
                }
                continue;
            }
            vector<string> firstSet = computeSingularFirstSets(cfg, A, prod);

            for (const string& f : firstSet) {
                if (f != "^") {
                    if (parsingTable[A].find(f) != parsingTable[A].end()) {
                        //cout << "Grammar is not LLL(1)!\n";
                        parsingTable[A][f] = parsingTable[A][f] + " ,, " + prod; 
                        parsing_error = true;

                    }
                    else {
                        auto it = find(terminals.begin(), terminals.end(), f);
                        if (it == terminals.end()) {
                            //cout << "Adding terminal " << f << endl;
                            terminals.push_back(f);
                        }
                        parsingTable[A][f] = prod;
                    }
                } else {
                    if (followSets.find(A) != followSets.end() && !followSets.at(A).empty()){
                        for (const string& f2 : followSets.at(A)) {
                            if (parsingTable[A].find(f2) != parsingTable[A].end()) {
                                //cout << "Grammar is not LL(1)!\n";
                                parsingTable[A][f] = parsingTable[A][f] + ",," + prod; 
                                parsing_error = true;

                            }
                            else {
                                auto it = find(terminals.begin(), terminals.end(), f2);
                                if (it == terminals.end()) {
                                    //cout << "Adding terminal2 " << f << endl;
                                    terminals.push_back(f);
                                }
                                parsingTable[A][f2] = prod;
                            }
                        }
                    }
                }
            }
            /*
            for (const string& f : firstSet) {
                cout << f << ",";
            }
            cout << endl;
            */
        }
        
    }
    for (auto& r : parsingTable){
        string temp = r.first;
        for (auto& t : terminals) {
            if (parsingTable[temp].find(t) == parsingTable[temp].end()) {
                parsingTable[temp][t] = "     ";
            }            
        }
    
    }

    cout << "\nLL(1) Parsing Table:\n      ";
    int max_rule = 0;
    int max_prod = 0;
    for (const auto& row : parsingTable) {
        if (row.first.length() > max_rule)
            max_rule = row.first.length();
    }
    for (const auto& row : parsingTable) {
        for (const auto& col : row.second) {
            if (col.second.length() > max_prod)
                max_prod = col.second.length();
        }
    }
    vector<size_t> max_col_lengths;
    
    for (const auto& row : parsingTable) {
        size_t col_index = 0;
        for (const auto& col : row.second) {
            if (max_col_lengths.size() <= col_index) {
                max_col_lengths.push_back(0);
            }
            max_col_lengths[col_index] = max(max_col_lengths[col_index], col.second.length());
            ++col_index;
        }
    }

    int first_i = 0;
    int col_len = 0;
    int iterate = 0;
    for (const auto& row : parsingTable) {
        col_len +=  max_rule + 1;
        for (const auto& col : row.second) {
            if (first_i++ == 0)
                cout << setw(1+max_rule+max_prod) << col.first << "    |";
            else
                cout << setw(max_prod) << col.first << "    |";
            col_len +=  max_prod+6;

        }
        cout << endl;
        break;
    }
    string line_breaker(col_len,'-');
    cout << line_breaker << endl;
    iterate = 0;
    for (const auto& row : parsingTable) {
        cout << setw(4+max_rule) << row.first << " -> ";
        for (const auto& col : row.second) {
            cout << setw(max_prod) << col.second << "   | ";
        }
        cout << endl;
    }
    if (parsing_error)
        cout << "Grammar is not LL(1)!\n";
             
   


}

void displayCFG(const Grammar& cfg) {
    for (const auto& rule : cfg.productions) {
        cout << rule.first << " -> ";
        for (size_t i = 0; i < rule.second.size(); ++i) {
            cout << rule.second[i];
            if (i < rule.second.size() - 1) cout << " | ";
        }
        std::cout << endl;
    }

    for (const auto& rule : cfg.nonter_count) {
   //     cout << rule.first << " == " << rule.second << " || ";
    }
    std::cout << endl;
}
void leftFactoring(Grammar &cfg) {
    bool continue_leftfactoring = false;
    for (const auto& rule : cfg.productions) {
        //cout << rule.first << " -> ";
        for (size_t i = 0; i < rule.second.size(); ++i) {
            vector<string> commonPrefixes;
            commonPrefixes.push_back(rule.second[i]);
            string same_letter = "";
            string save_same_letter = "";
            for (size_t j = i+1; j < rule.second.size(); ++j) {
                same_letter = "";
                bool match = false;
                //cout << i << "!" << rule.second[i] << "!" << "!" << rule.second[j] << "!"  << endl;
                if (rule.second[i][1] == rule.second[j][1]) {
                    match = true;
                    int index = 1;
                    while (rule.second[i][index] != ' ' || rule.second[j][index] != ' ') {
                        //cout << "rule.second[i][index] = " << rule.second[i][index] << " rule.second[j][index] = " << rule.second[j][index] << endl;
                        if (rule.second[i][index] != rule.second[j][index]) {
                            match = false;
                            same_letter = "";
                            break;
                        }
                        same_letter += rule.second[i][index];
                        index++;
                    }
                }
                if (match) {
                    commonPrefixes.push_back(rule.second[j]);   
                    save_same_letter = same_letter; 
                }
            }
            same_letter = save_same_letter;    
            if (commonPrefixes.size() > 1) {
                string letter(1,rule.first[0]);
                cfg.nonter_count[letter]++;
                string extra_apos(cfg.nonter_count[letter], '\'');
                string newNonTerminal = letter + extra_apos;
                cfg.productions[newNonTerminal] = {};
                //cout << "Left factoring = ";
                //cout << "SL =" << same_letter << "[";
                for (size_t k = 0; k < commonPrefixes.size(); ++k) {
                    //cout << commonPrefixes[k] << " | ";
                    if (commonPrefixes[k].length() - same_letter.length() > 2)
                        cfg.productions[newNonTerminal].push_back(commonPrefixes[k].substr(same_letter.length()+1));
                    else
                        cfg.productions[newNonTerminal].push_back(" ^ ");
                    auto &vec = cfg.productions[rule.first];
                    vec.erase(remove(vec.begin(), vec.end(), commonPrefixes[k]), vec.end());
                }
                //cout << endl;
                //cout << "%" <<  " " + same_letter + " " + newNonTerminal + " " << endl;
                cfg.productions[rule.first].push_back( " " + same_letter + " " + newNonTerminal + " ");
                continue_leftfactoring = true;
                
            } else {
                //cout << "No left factoring = " << rule.second[i] << endl;
            }
            //displayCFG(cfg);
        }
    //cout << "***************************\n";
    //displayCFG(cfg);
    //std::cout << "***************************\n";
    
    }
    if (continue_leftfactoring) {
        leftFactoring(cfg);
    }

    
}
void removeLeftRecursion(Grammar& cfg) {
    for (auto& rule : cfg.productions) {
        string A = rule.first;
        vector<string> second_rules;
        for (size_t i = 0; i < rule.second.size(); ++i) {
            bool match = true;
            //cout << "A = " << A << " rule.second[i] = " << rule.second[i] << endl;
            for (int l = 0; l < A.length(); l++) {
                if (rule.second[i][l+1] != A[l]) {
                    match = false; 
                    break;
                }
            }
            if (match) {
                //cout << "Horaha";
                second_rules.push_back(rule.second[i]);
            }
        }
        string newNonTerminal = "";
        if (second_rules.size() > 0) {
            string letter(1,rule.first[0]);
            cfg.nonter_count[letter]++;
            string extra_apos(cfg.nonter_count[letter], '\'');
            newNonTerminal = letter + extra_apos;
            cfg.productions[newNonTerminal] = {};

            for (size_t i = 0; i < second_rules.size(); ++i) {
                cfg.productions[newNonTerminal].push_back("" + second_rules[i].substr(1+A.length()) + "" + newNonTerminal + "");
                auto &vec = cfg.productions[rule.first];
                vec.erase(remove(vec.begin(), vec.end(), second_rules[i]), vec.end());
            }
            cfg.productions[newNonTerminal].push_back(" ^ ");
            
        }
        for (size_t i = 0; i < rule.second.size(); ++i) {
            rule.second[i] = rule.second[i] + "" + newNonTerminal;
            if (second_rules.size() > 0) {
                rule.second[i] = rule.second[i] + " ";
            }
        }

    }
}
void solveNonImmediateLR(string A, string B, Grammar &cfg){
    string nameA = A;
    string nameB = B;

    //cout << "nameA = " << nameA << " nameB = " << nameB << endl;
    vector<string> rulesA, rulesB, newRulesA;
    rulesA = cfg.productions[A];
    rulesB = cfg.productions[B];
    //cout << "nameA = " << nameA << " nameB = " << nameB << endl;
    //cout << endl << endl << endl;
    for (auto rule : rulesA) {
        if (rule.substr(1, nameB.size()) == nameB) {
            //cout << "rule =" << rule << "|" << endl;
            for (auto rule1 : rulesB){
                //cout << "rule1 =" << rule1 << "|" << endl;
                //cout << "combined =" << rule1 + rule.substr(nameB.size()+2) << "|" << endl;
                newRulesA.push_back(rule1 + rule.substr(nameB.size()+2));

            }
        }
        else{
            newRulesA.push_back(rule + "");
        }
    }

    auto it = cfg.productions.find(nameA);
    if (it != cfg.productions.end()) {
        cfg.productions.erase(it); // Remove all productions for the non-terminal
    //    cout << "Removed all productions for non-terminal: " << nameA << endl;
    } else {
    //    cout << "Non-terminal: " << nameA << " not found in productions" << endl;
    }

    for (auto rule : newRulesA) {
        //cout << "|" << rule << "|" << endl;
        cfg.productions[nameA].push_back(rule);
    }
    displayCFG(cfg);
    //cout << "****************\n";

}
void solveImmediateLR(string A_param, Grammar &cfg){
    for (auto& rule : cfg.productions) {
        string A = rule.first;
        if (A == A_param){
            vector<string> second_rules;
            for (size_t i = 0; i < rule.second.size(); ++i) {
                bool match = true;
                //cout << "A = " << A << " rule.second[i] = " << rule.second[i] << endl;
                for (int l = 0; l < A.length(); l++) {
                    if (rule.second[i][l+1] != A[l]) {
                        match = false; 
                        break;
                    }
                }
                if (match) {
                    //cout << "Horaha";
                    second_rules.push_back(rule.second[i]);
                }
            }
            string newNonTerminal = "";
            if (second_rules.size() > 0) {
                string letter(1,rule.first[0]);
                cfg.nonter_count[letter]++;
                string extra_apos(cfg.nonter_count[letter], '\'');
                newNonTerminal = letter + extra_apos;
                cfg.productions[newNonTerminal] = {};
    
                for (size_t i = 0; i < second_rules.size(); ++i) {
                    cfg.productions[newNonTerminal].push_back("" + second_rules[i].substr(1+A.length()) + "" + newNonTerminal + " ");
                    auto &vec = cfg.productions[rule.first];
                    vec.erase(remove(vec.begin(), vec.end(), second_rules[i]), vec.end());
                }
                cfg.productions[newNonTerminal].push_back(" ^ ");
                
            }
            for (size_t i = 0; i < rule.second.size(); ++i) {
                rule.second[i] = rule.second[i] + "" + newNonTerminal + "";
                if (second_rules.size() > 0) {
                    rule.second[i] = rule.second[i] + " ";
                }
                //cout << "()" << rule.second[i] << "[" <<endl;
            }
    
    
        }
    }
}

void indirect_algorithm(Grammar &cfg){

    vector<string> nonTerminals;
    //cout << "nonTerminalSTart ="<< START_STATE << "|";
    nonTerminals.push_back(START_STATE);
    for (const auto& pair : cfg.productions) {
        //cout << "nonTerminal ="<< pair.first << "|";
        if (pair.first != START_STATE)
            nonTerminals.push_back(pair.first);
    }



    int size = nonTerminals.size();
    //for (int i = size-1; i >= 0; i--) {
      //  for (int j = size-1; j > i; j--) {
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < i; j++) {
            //cout << "i = " << i << " j = " << j << endl;
            //cout << "nonTerminals[i] = " << nonTerminals[i] << " nonTerminals[j] = " << nonTerminals[j] << endl;

            // Optionally, you can display their productions
            //cout << "Productions for nonTerminals[i]: ";
            //for (const auto& prod : cfg.productions[nonTerminals[i]]) {
            //    cout << prod << " ";
            //}
            //cout << endl;
            /*
            cout << "Productions for nonTerminals[j]: ";
            for (const auto& prod : cfg.productions[nonTerminals[j]]) {
                cout << prod << " ";
            }
            cout << endl;

            cout << "****************\n";
            */
            solveNonImmediateLR(nonTerminals[i], nonTerminals[j],cfg);
        }
        solveImmediateLR(nonTerminals[i],cfg);
    }
}


int main() {
    string filename = "cfg.txt";
    Grammar cfg = readCFG(filename);
    
    cout << "Original CFG:\n";
    displayCFG(cfg);

 
    leftFactoring(cfg);
    cout << "\nCFG after Left Factoring:\n";
    displayCFG(cfg);
    cout << "=============================================\n";   

    indirect_algorithm(cfg);
    //removeLeftRecursion(cfg);
    cout << "\nCFG after Removing Left Recursion:\n";
    displayCFG(cfg);
    cout << "=============================================\n";
    auto firstSets = computeFirstSets(cfg);
    for (const auto& entry : firstSets) {
        cout << "First(" << entry.first << ") = { ";
        bool first = true;
        for (const auto& symbol : entry.second) {
            if (!first) {
                cout << ", ";
            }
            cout << symbol;
            first = false;
        }
        cout << " }" << endl;
    }
    cout << "=============================================\n";
    auto followSets = computeFollowSets(cfg, firstSets);
    
    for (const auto& entry : followSets) {
        cout << "Follow(" << entry.first << ") = { ";
        bool first = true;
        for (const auto& symbol : entry.second) {
            if (!first) {
                cout << ", ";
            }
            cout << symbol;
            first = false;
        }
        cout << " }" << endl;
    }
    cout << "=============================================\n";   
    constructLL1Table(cfg, firstSets, followSets);

    cout << "|" << START_STATE << "|\n";
    return 0;
}
