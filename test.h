#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <string>
using namespace std;

int columnWidth = 13;  
std::unordered_map<std::string, char> strToChar;  // Maps strings to single characters
std::unordered_map<char, std::string> charToStr;  // Maps single characters back to original strings
std::set<char> usedChars;  // Keeps track of used characters
char nextChar = 'A';  // Start mapping from 'A'
//char term = 'a';  

void printMappings() 
{
    cout << "\nMappings:\n";
    cout << "---------------------------------\n";
    cout << "|   String   |   Mapped Char   |\n";
    cout << "---------------------------------\n";
    
    for (const auto& pair : strToChar) 
    {
        cout << "| " << pair.first << "\t | \t" << pair.second << "  |\n";
    }
    
    cout << "---------------------------------\n";
}


// Function to get the next available unique character
char getNextAvailableChar(string type) 
{
    char chad = nextChar;

    while (usedChars.count(chad)) 
    {
        chad++;
        if (chad > 'Z' && chad < 'a') chad = 'a';  // Skip non-letter characters
        if (chad > 'z') 
        {
            std::cerr << "Error: Ran out of unique characters to map.\n";
            exit(1);
        }
    }
    return chad++;
}

// Function to get or assign a unique character for a string
char getMappedChar(const std::string& str ,std::string type)
{
    // If the string is already mapped, return the existing mapping
    if (strToChar.find(str) != strToChar.end())
    {
        return strToChar[str];
    }

    // If the string is a single character, it should map to itself if available
    if (str.length() == 1)
    {
        char ch = str[0];

        // If the character is not already mapped to another string, map it to itself
        if (charToStr.find(ch) == charToStr.end()) 
        {
            charToStr[ch] = str;
            strToChar[str] = ch;
            usedChars.insert(ch);  // Mark character as used
            return ch;
        }

        // If the character is already mapped to something else, assign a new unique character
        if (charToStr[ch] != str)
         {
            char newChar = getNextAvailableChar(type);
            strToChar[str] = newChar;
            charToStr[newChar] = str;
            usedChars.insert(newChar);
            return newChar;
        }

        return ch;  // Return existing correct mapping
    }

    // If it's a multi-character string, assign a new unique character
    char newChar = getNextAvailableChar(type);
    strToChar[str] = newChar;
    charToStr[newChar] = str;
    usedChars.insert(newChar);
    return newChar;
}

// Function to process CFG
void processCFG(const std::string& inputFile, const std::string& outputFile) 
{
    std::ifstream inFile(inputFile);
    std::ofstream outFile(outputFile);
    std::string line;

    if (!inFile || !outFile) 
    {
        std::cerr << "Error opening files!\n";
        return;
    }

    while (std::getline(inFile, line)) 
    {
        std::istringstream iss(line);
        std::string lhs, arrow, token;

        iss >> lhs >> arrow;  // Read LHS and '->'

        char lhsChar = getMappedChar(lhs, "non");  // Map LHS
        outFile << lhsChar << " -> ";

        bool first = true;
        while (iss >> token) 
        {  // Read RHS tokens
            if (token == "|") 
            {
                outFile << " | ";
                first = true;  // Reset for new production
                continue;
            }

            char rhsChar = getMappedChar(token , "term");  // Map RHS token	
            if (!first) outFile << rhsChar;
            else { outFile << rhsChar; first = false; }
        }
        outFile << "\n";
    }

    inFile.close();
    outFile.close();
}


void printMappedProduction(const std::string& production) 
{
    for (char ch : production) 
    {  
        if (charToStr.find(ch) != charToStr.end()) 
        {  
            std::cout << charToStr[ch] << " "; // Print the mapped string
        } 
        else 
        {  
            std::cout << ch << " ";  // Print character if not mapped
        }
    }
}

string p(const std::string& production) 
{
    string s = "";
    for (char ch : production) 
    {  
        if (charToStr.find(ch) != charToStr.end()) 
        {  
            s += charToStr[ch]; // Print the mapped string
        } 
        else 
        {  
            s += ch;  // Print character if not mapped
        }
    }
return s;
}

void pstb(vector<char>& terminalSSS) 
{
    cout << "Parsing Table:\n";
    cout << setw(5 + 3) << left << " ";
    for (char terminal : terminalSSS) 
    {
        string s = p(string(1, terminal));
        cout << setw(columnWidth) << left << s << "|";
    }
    cout << setw(columnWidth) << left << " $";
    cout << endl;

    // Print separator line
    cout << string((columnWidth + 3) * terminalSSS.size() + 6, '-') << endl;

    // Print separator line at the end
   // cout << string((columnWidth + 3) * terminalSSS.size() + 8, '-') << endl;
}


void pMappedProduction(char &name,vector<char>& terminalSSS, map<char, string>& ParsingTable) 
{
    if (charToStr.find(name) != charToStr.end()) 
    cout << setw(5) << left << charToStr[name] << " : ";
    else cout << setw(5) << left << name << " : ";

    
    for (char terminal : terminalSSS) 
    {
        if (ParsingTable.find(terminal) != ParsingTable.end()) 
        {
            string productionStr = p(ParsingTable[terminal]);
            
            
            cout << setw(columnWidth) << left << productionStr << "|";
        } 
        else 
        {
            cout << setw(columnWidth) << left << " " << "|";
        }
    }
    if (ParsingTable.find('$') != ParsingTable.end()) 
        {
            string productionStr = p(ParsingTable['$']);
            
            
            cout << setw(columnWidth) << left << productionStr << "|";
        } 
        else 
        {
            cout << setw(columnWidth) << left << " " << "|";
        }


    cout << endl;
}
