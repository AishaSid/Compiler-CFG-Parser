#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include "test.h"
using namespace std;

vector<char> terminalSSS;
bool find_terminal(char p , vector<char> &v);

bool isNonTerminalPresent(char name);
class Non_Terminal 
{
public:
    char name;
    vector<string> Production;
    set<char> First;
    set<char> Follow;
    map<char, string> ParsingTable; // terminl : production
    Non_Terminal* next_symbol;
    vector<char> first_set;

    static Non_Terminal* head;

    Non_Terminal(char name, const string& prod) 
    {
        this->name = name;
        this->next_symbol = nullptr;

        istringstream iss(prod);
        string temp;
        while (getline(iss, temp, '|')) 
        {
            Production.push_back(temp);
        }
    }

Non_Terminal(char name, vector<string> Prod)
{
    this->name = name;
    this->next_symbol = nullptr;
    this->Production = Prod;
}

    void extract_terminals()
    {
        for (const string& prod : Production) 
        {
            for (char c : prod)
            {
                if (!isNonTerminalPresent(c) && c != ' ' && c != '|' && !isDuplicate(c)) 
                {
                    terminalSSS.push_back(c);
                }
            }
        }
    }

    void print_productions() 
    {
        printMappedProduction(string(1, name));
        cout<< " -> ";
        for (int i = 0; i < Production.size(); ++i) 
        {
            printMappedProduction(Production[i]);
            if (i < Production.size() - 1) cout << " | ";
        }
        cout << endl;
    }

    void left_factored()
    {
        while (true)
        {
            vector<int> count(Production.size(), 0); // initialize with 0 

            // to get all the counts of same left prefix 
            for (int i = 0; i < Production.size(); i++)
            {
                for (int j = i + 1; j < Production.size(); j++)
                {
                    if (Production[i][0] == Production[j][0])
                    {
                        count[i]++;
                        count[j]++;
                    }
                }
            }

            // to check if it is LL(1) parsed or not. 
            int minIndex = -1;
            for (int i = 0; i < count.size(); i++)
            {
                if (count[i] > 0 && (minIndex == -1 || Production[i].size() < Production[minIndex].size()))
                {
                    minIndex = i;
                }
            }

            if (minIndex == -1) break; // if ll(1) then break.
            
            string commonPrefix = "";
            for (int i = 0; i < Production[minIndex].size(); i++)
            {
                char c = Production[minIndex][i];
                bool allMatch = true;

                for (int j = 0; j < Production.size(); j++)
                {
                    if (count[j] > 0 && (i >= Production[j].size() || Production[j][i] != c))
                    {
                        allMatch = false;
                        break;
                    }
                }

                if (!allMatch) break;
                commonPrefix += c;
            }

            if (commonPrefix.empty()) break;

            char newNonTerminalName = 'A';
            while (isNonTerminalPresent(newNonTerminalName) || (charToStr.find(newNonTerminalName) != charToStr.end()) )
            {
                if (newNonTerminalName == 'Z')
                     cout<<"The current code has implemented upper-cases as non_terminals and all other symbols as terminals. thenkyou"<<endl;
                newNonTerminalName += 1;
            }

            vector<string> newProductions;
            vector<string> updatedProductions;

            for (int i = 0; i < Production.size(); i++)
            {
                if (count[i] > 0 && Production[i].substr(0, commonPrefix.size()) == commonPrefix)
                {
                    string newProd = Production[i].substr(commonPrefix.size());
                    if (newProd.empty()) newProd = "^";
                    newProductions.push_back(newProd);
                }
                else
                {
                    updatedProductions.push_back(Production[i]);
                }
            }

            updatedProductions.push_back(commonPrefix + newNonTerminalName);
            Production = updatedProductions;

            string newProdStr = "";
            for (int i = 0; i < newProductions.size(); i++)
            {
                newProdStr += newProductions[i];
                if (i < newProductions.size() - 1) newProdStr += "|";
            }

            Non_Terminal* newNonTerminal = new Non_Terminal(newNonTerminalName, newProdStr);
            insertInMiddle(this, newNonTerminal);
        }
    }
    // A  → β A'
    //A' → α A' | ^
    void direct_recursion() 
    {
        vector<string> alpha; //  (Aα)
        vector<string> beta;  //  (β)

        for (const string& prod : Production) 
        {
            if (!prod.empty() && prod[0] == name) 
            { 
                // Direct left recursion found: A → Aα
                if (prod.size() == 1 && prod[0] == name) 
                {
                    cerr << "Error: Production '" << prod << "' is invalid!" << endl;
                    auto it = std::find(Production.begin(), Production.end(), prod); // get index 
                    if (it != Production.end()) 
                    { 
                        Production.erase(it); 
                    }
                continue; 
                }

                alpha.push_back(prod.substr(1)); // Store α
            }
            else 
            {
                beta.push_back(prod); // Store β
            }
        }

        if (alpha.empty()) return; // No left recursion yipeee ;p

        // Create a new non-terminal 
        char newNonTerminalName = 'a';
        while (isNonTerminalPresent(newNonTerminalName) || (charToStr.find(newNonTerminalName) != charToStr.end()) ) 
        {
            newNonTerminalName += 1;
        }

        // Update original production: A → β A'
        Production.clear();
        for (const string& b : beta) 
        {     if (b == "^")
            Production.push_back(std::string(1, newNonTerminalName));
            else
            Production.push_back(b + newNonTerminalName);
        }

        // Create new production: A' → α A' | ^
        vector<string> newProductions;
        
        for (const string& a : alpha) 
        {
            newProductions.push_back(a + newNonTerminalName);
        }
        newProductions.push_back("^"); // Epsilon 

        // Insert the new non-terminal
        Non_Terminal* newNT = new Non_Terminal(newNonTerminalName, newProductions);
        insertInMiddle(this, newNT);
    }

    // void indirect_recursion() 
    // {
    //     bool changed;
    //     do 
    //     {
    //         changed = false;
    //         for (Non_Terminal* temp = head; temp != nullptr; temp = temp->next_symbol) 
    //         {
    //             for (Non_Terminal* targetNT = head; targetNT != nullptr; targetNT = targetNT->next_symbol) 
    //             {
    //                 if (temp == targetNT) continue;  // Skip same non-terminal
    
    //                 vector<string> newProductions;
    //                 bool foundIndirectRecursion = false;
    
    //                 for (const string& prod : temp->Production) 
    //                 {
    //                     // Check if the production starts with targetNT (indirect recursion candidate)
    //                     if (!prod.empty() && prod[0] == targetNT->name) 
    //                     {
    //                         // Check if targetNT eventually leads back to temp (indirect recursion)
    //                         set<char> visited;
    //                         vector<Non_Terminal*> stack;
    //                         stack.push_back(targetNT);
    //                         visited.insert(targetNT->name);
    //                         bool leadsBackToTemp = false;

    //                         while (!stack.empty()) 
    //                         {
    //                             Non_Terminal* current = stack.back();
    //                             stack.pop_back();

    //                             for (const string& subProd : current->Production) 
    //                             {
    //                                 if (!subProd.empty() && subProd[0] == temp->name) 
    //                                 {
    //                                     leadsBackToTemp = true;
    //                                     break;
    //                                 }
    //                                 if (!subProd.empty() && isNonTerminalPresent(subProd[0]) && visited.find(subProd[0]) == visited.end()) 
    //                                 {
    //                                     Non_Terminal* nextNT = head;
    //                                     while (nextNT && nextNT->name != subProd[0]) 
    //                                     {
    //                                         nextNT = nextNT->next_symbol;
    //                                     }
    //                                     if (nextNT) 
    //                                     {
    //                                         stack.push_back(nextNT);
    //                                         visited.insert(nextNT->name);
    //                                     }
    //                                 }
    //                             }
    //                             if (leadsBackToTemp) break;
    //                         }

    //                         if (leadsBackToTemp) 
    //                         {
    //                             foundIndirectRecursion = true;
    //                             for (const string& subProd : targetNT->Production) 
    //                             {
    //                                 newProductions.push_back(subProd + prod.substr(1));
    //                             }
    //                         }
    //                     }
                        
    //                     if (!foundIndirectRecursion) 
    //                     {
    //                         newProductions.push_back(prod);
    //                     }
    //                 }
    
    //                 if (foundIndirectRecursion)
    //                 {
    //                     temp->Production = newProductions;
    //                     changed = true;
    //                 }
    //             }
    //         }
    //     } while (changed);
    // }


    void indirect_recursion() 
{
    bool changed;
    do 
    {
        changed = false;
        for (Non_Terminal* temp = head; temp != nullptr; temp = temp->next_symbol) 
        {
            vector<string> updatedProductions;

            for (const string& prod : temp->Production) 
            {
                bool foundIndirectRecursion = false;

                // Check if the production starts with another Non-Terminal (targetNT)
                for (Non_Terminal* targetNT = head; targetNT != nullptr; targetNT = targetNT->next_symbol) 
                {
                    if (temp == targetNT) continue;  // Skip same non-terminal

                    if (!prod.empty() && prod[0] == targetNT->name) 
                    {
                        // Check if targetNT leads back to temp (indirect recursion detection)
                        set<char> visited;
                        vector<Non_Terminal*> stack;
                        stack.push_back(targetNT);
                        visited.insert(targetNT->name);
                        bool leadsBackToTemp = false;

                        while (!stack.empty()) 
                        {
                            Non_Terminal* current = stack.back();
                            stack.pop_back();

                            for (const string& subProd : current->Production) 
                            {
                                if (!subProd.empty() && subProd[0] == temp->name) 
                                {
                                    leadsBackToTemp = true;
                                    break;
                                }
                                if (!subProd.empty() && isNonTerminalPresent(subProd[0]) && visited.find(subProd[0]) == visited.end()) 
                                {
                                    Non_Terminal* nextNT = head;
                                    while (nextNT && nextNT->name != subProd[0]) 
                                    {
                                        nextNT = nextNT->next_symbol;
                                    }
                                    if (nextNT) 
                                    {
                                        stack.push_back(nextNT);
                                        visited.insert(nextNT->name);
                                    }
                                }
                            }
                            if (leadsBackToTemp) break;
                        }

                        // If indirect recursion is detected, replace the production
                        if (leadsBackToTemp) 
                        {
                            foundIndirectRecursion = true;
                            for (const string& subProd : targetNT->Production) 
                            {
                                updatedProductions.push_back(subProd + prod.substr(1));
                            }
                            changed = true;
                            break;  // No need to check other targetNTs
                        }
                    }
                }

                // If no recursion was found, keep the original production
                if (!foundIndirectRecursion) 
                {
                    updatedProductions.push_back(prod);
                }
            }

            temp->Production = updatedProductions;
        }
    } while (changed);
}

  void helper_first()
  {
      bool changed;
      do
      {
          changed = false;
          Non_Terminal *temp = head;
          while (temp)
          {
              for (const string &prod : temp->Production)
              {
                  bool epsilonInFirst = true; // Assume production derives epsilon unless proven otherwise

                  for (char symbol : prod)
                  {
                      if (find_terminal(symbol, terminalSSS) || symbol == '^') // Terminal
                      {
                          if (temp->First.insert(symbol).second)
                              changed = true;
                          epsilonInFirst = false;
                          break;
                      }
                      // Find the corresponding non-terminal
                      
                      Non_Terminal *nextNT = head;
                      while (nextNT && nextNT->name != symbol)
                      {
                          nextNT = nextNT->next_symbol;
                      }

                      if (nextNT)
                      {
                          for (char firstChar : nextNT->First)
                          {
                              if (firstChar != '^') // Copy First except epsilon
                              {
                                  if (temp->First.insert(firstChar).second)
                                      changed = true;
                              }
                          }
                          if (nextNT->First.find('^') == nextNT->First.end()) // If no epsilon in First(symbol), stop
                          {
                              epsilonInFirst = false;
                              break;
                          }
                      }
                      else
                      {
                          cout << "Error: Undefined non-terminal '" << symbol << "' found!" << endl;
                      }
                  }

                  if (epsilonInFirst) // If all symbols allow epsilon, add it
                  {
                      if (temp->First.insert('^').second)
                          changed = true;
                  }
              }
              temp = temp->next_symbol;
          }
      } while (changed); // Repeat until no more changes occur
  }
// there is an else case the needs to be handled with a bool -- mam said assume it doesn't exist
bool help2(char nextChar, string where ) // it only pushes the first and tells if there is epsilon
{
    bool eps = false;
    Non_Terminal* nextNT = head;
    //pehle non_term ko dhoondo
    while (nextNT && nextNT->name != nextChar)
    {
        nextNT = nextNT->next_symbol;
    }
    // mil gya to uska first push krna he. 
    if (nextNT)
    {
        for (char firstChar : nextNT->First) 
        {
          if(firstChar == '^')
              eps = true; 
          else
            { // Exclude epsilon ?? why tho ??
                if(where == "follow")
                this->Follow.insert(firstChar);
                else
                    this->First.insert(firstChar);
            }
        }
    }
    else
    {
        cout<<"Production not found: "<<nextChar<< " in the production of:"<<this->name<<endl;
    }
return eps;
}

bool help3omg(char nextChar, string Prod)
{  
    bool eps = help2(nextChar, "follow"), vv = false; 
    if (eps)
    {
        for (char firstChar : Prod)
            {
                if (find_terminal(firstChar, terminalSSS))   // If it's a terminal
                    {
                        this->Follow.insert(firstChar);
                        return false;
                    }
                else // its a non_term
                { // to uska F push krte hein 
                    eps =  help2(firstChar, "follow");
                    if(eps)
                        continue;
                    
                    return false;
                }
            }
        vv = true; 
        // if it reaches there... to kuch nahi krte bhyi, itna hi hota he 
        // nhi itna hi nhi hota agey looping krni hoti he pagal
    }
return  vv ;    
}

void helper_follow() 
{
    // Step 1: Add '$' to the Follow set of the start symbol
    if (head) 
    {
        head->Follow.insert('$'); 
    }      
        Non_Terminal* temp = head; 
    while (temp)  // (A) -> B | C | abCD  
    { // we are looking for A ki occurence everywhere 
        for (Non_Terminal* current = head; current; current = current->next_symbol) 
        {
            for (const string& prod : current->Production) // her line ke production pe loop 
            {
                int pos = prod.find(temp->name); // looking for A 
               
                while (pos != string::npos) 
                {
                    bool abcd = true;
                    
                    if (pos + 1 < prod.size()) // If there's a next | else vo looping wali cheez
                    { 
                        abcd = false;
                        char nextChar = prod[pos + 1];
                        if (find_terminal(nextChar, terminalSSS))  // If it's a terminal
                        { 
                            temp->Follow.insert(nextChar);
                            // bss push kro and move one bro!
                        }
                        else 
                        { // If it's a non-terminal
                        // agar non ho to uska FIRST push krte hein - but idk about non_term
                         abcd = temp->help3omg(nextChar, prod.substr(pos + 2));
                        //  you keep checking the next, if term then return else keep pushing the FIRST 
                            
                        }
                    } 
                    if(abcd)  // If at the end, to pichle ka Follow le lo, simple 
                    { 
                        if (current->name != temp->name)
                        {
                            temp->Follow.insert(current->Follow.begin(), current->Follow.end());
                        }
                    }
                    pos = prod.find(temp->name, pos + 1);
                }
            }
        }
           temp = temp->next_symbol;
        }
}

bool find_terminal(char p , vector<char> &v)
{
    for (char term : v)
        {
            if (term == p)
                return true;
        }
return false;
}

    void constructParsingTable()
    {
        bool epsilonPresent = false;  
             for (const string& prod : Production)
            {
            // Add entries to the parsing table
                if (prod[0] != '^' && find_terminal(prod[0], terminalSSS)) 

                {
                    // Add entry for the terminal
                    ParsingTable[prod[0]] = prod;
                    // idher duplicates ka issue nahi ana chahiye kiuke factoring ki hui he dumbo 
                    continue;
                }
                if (prod[0] != '^')
                {
                 for (char terminal : prod) // its a non_terminal
                {
                    Non_Terminal* nextNT = head;
                    //pehle non_term ko dhoondo
                    while (nextNT && nextNT->name != terminal)
                    {
                        nextNT = nextNT->next_symbol;
                    }
                    // mil gya to uska first lena he 
                    if (nextNT)
                    {
                    // S -> ABC   
            for (char firstChar : nextNT->First)
                {
                    if(firstChar == '^')
                        epsilonPresent = true;
                    else
                    {
                        if (ParsingTable.find(firstChar) != ParsingTable.end()) 
                            {
                                cout << "Grammar is not LL(1)! Conflict found at " << name << " for terminal " << firstChar << "and non_term : "<< nextNT->name<< endl;
                                exit(1);
                            }
                            ParsingTable[firstChar] = prod;
                    }
                }
                        if(!epsilonPresent)
                            break;
                    }      
                }
                    if (epsilonPresent)
                        {
                            for (char terminal : Follow) // its a non_terminal
                                {

                            if (ParsingTable.find(terminal) != ParsingTable.end()) 
                                {
                                    cout << "Grammar is not LL(1)! Conflict found at " << name << " for terminal " << terminal << endl;
                                    exit(1);
                                }
                                ParsingTable[terminal] = prod;
                                }
                        }
            }
                if (prod[0] == '^')
                {   
                for (char terminal : Follow) // its a non_terminal
                    {

                if (ParsingTable.find(terminal) != ParsingTable.end()) 
                    {
                        cout << "Grammar is not LL(1)! Conflict found at " << name << " for terminal " << terminal << endl;
                        exit(1);
                    }
                    ParsingTable[terminal] = prod;
                    }     
                }
            
        }
    }

    void print_first() 
    {
        cout << "First(";
        printMappedProduction(string(1, name));
        cout << ") = {";
        bool f = true;
        
        for (char ch : First) 
        {
            if (!f) cout<< ",";
            printMappedProduction(string(1, ch));
            f = false;
        }
        cout << "}\n";
    }

    void print_follow() 
    {
        cout << "Follow(";
        printMappedProduction(string(1, name));
        cout << ") = {";
        bool f = true;

        for (char ch : Follow) 
        {
            if (!f) cout<< ",";
            printMappedProduction(string(1, ch));
            f = false;
        }
        cout << "}\n";
    }

  void printParsingTable() 
  {
    printMappedProduction(string(1, name));
  cout << " : ";
      for (const auto& entry : ParsingTable) 
      {
        printMappedProduction(string(1, entry.first));
          cout << " = ";
        printMappedProduction(entry.second);
            cout << " | ";
      }
  cout  << endl;
  }
private:
    bool isDuplicate(char c) 
    {
        for (char term : terminalSSS) 
        {
            if (term == c) return true;
        }
        return false;
    }

    void insertInMiddle(Non_Terminal* prev, Non_Terminal* newNode)
    {
        if (!prev) return;
        
        newNode->next_symbol = prev->next_symbol;
        prev->next_symbol = newNode;
    }
};

Non_Terminal* Non_Terminal::head = nullptr;

bool isNonTerminalPresent(char name)
{
    Non_Terminal* current = Non_Terminal::head;
    while (current)
    {
        if (current->name == name) return true;
        current = current->next_symbol;
    }
    return false;
}
bool isValidNonTerminal(const string& symbol)
{
    return symbol.size() == 1;
}

void readCFG(const string& filename) 
{
    ifstream file(filename);
    Non_Terminal* tail = nullptr;
    string line;

    while (getline(file, line)) 
    {
        istringstream iss(line);
        string left, arrow, right;

        iss >> left >> arrow;
        if (arrow != "->" || !isValidNonTerminal(left)) 
        {
            cerr << "Invalid format: " << line << endl;
            exit(1);
        }

        char nonTerminal = left[0];
        Non_Terminal* current = Non_Terminal::head;
        while (current) 
        {
            if (current->name == nonTerminal) 
            {
                cerr << "Duplicate non-terminal: " << nonTerminal << endl;
                exit(1);
            }
            current = current->next_symbol;
        }

        getline(iss, right);
        if (right.empty()) 
        {
            cerr << "No production for: " << nonTerminal << endl;
            exit(1);
        }
        string cleanedProduction;
        for (char c : right) 
        {
            if (c != ' ') 
            {
                cleanedProduction += c;
            }
        }
        Non_Terminal* newNode = new Non_Terminal(nonTerminal, cleanedProduction);
        if (!Non_Terminal::head) 
        {
            Non_Terminal::head = tail = newNode;
        } else 
        {
            tail->next_symbol = newNode;
            tail = newNode;
        }
    }
    file.close();
}


void iterateAndApply(void (Non_Terminal::*func)()) 
{
    Non_Terminal* current = Non_Terminal::head;
    while (current) 
    {
        (current->*func)();
        current = current->next_symbol;
    }
}

int main()
{
    processCFG("input.txt", "Cfg.txt");

    cout << "CFG input: \n";
    string filename = "Cfg.txt";
    readCFG(filename);
    
    Non_Terminal* current = Non_Terminal::head;
   
    iterateAndApply(&Non_Terminal::print_productions);
    current = Non_Terminal::head;
    while (current) 
    {
        current->extract_terminals();
        current = current->next_symbol;
    }
    printMappings();
    cout << "\nOriginal Terminals: ";
    for (char term : terminalSSS) 
    {
        printMappedProduction(string(1, term));
    }
    cout << endl;
    current = Non_Terminal::head;
    cout << "\nOriginal Non_terms: ";
    while (current) 
    {
        printMappedProduction(string(1,current->name));
        current = current->next_symbol;
    }
    cout<<endl;

    cout << "\nApplying Left Factoring:\n";
    iterateAndApply(&Non_Terminal::left_factored);
    cout << "\nLeft Factored CFG:\n";
    iterateAndApply(&Non_Terminal::print_productions);

    cout << "\nApplying Left Recursion:\n";
    cout << "\nDirect Recursion Removed CFG:\n";
    iterateAndApply(&Non_Terminal::direct_recursion);
    iterateAndApply(&Non_Terminal::print_productions);
    cout << "\nApplying Indirect Recursion:\n";
    current = Non_Terminal::head;
    current->indirect_recursion();
    iterateAndApply(&Non_Terminal::direct_recursion);
    iterateAndApply(&Non_Terminal::print_productions);

    cout << "\nCalculating First Sets:\n";
    current = Non_Terminal::head;
    current->helper_first();

    cout << "\nFirst Sets:\n";
    
    iterateAndApply(&Non_Terminal::print_first);
    cout << "\nCalculating Follow Sets:\n";
    current->helper_follow();

    cout << "\nFollow Sets:\n";
    iterateAndApply(&Non_Terminal::print_follow);

    // cout << "\nConstructing Parsing Table:\n";
    // current = Non_Terminal::head;
    // while (current) 
    // {
    //     current->constructParsingTable();
    //     current = current->next_symbol;
    // }

    // cout << "\nLL(1) Parsing Table:\n";
    // iterateAndApply(&Non_Terminal::printParsingTable);

    return 0;
}
