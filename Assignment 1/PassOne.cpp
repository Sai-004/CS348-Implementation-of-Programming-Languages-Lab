#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

// Define a structure to represent an assembly language instruction
struct Instruction
{
    string label;
    string opcode;
    string operand;
};

// Global maps for opcode table and symbol table
map<string, string> opCodeTable;
map<string, int> symTab;

// Function to load opcode table from a file
void loadOpCodeTable()
{
    ifstream optabFile("opTab.dat");
    if (!optabFile)
    {
        cerr << "Error opening optab file\n";
        exit(EXIT_FAILURE);
    }

    string mnemonic, opcode;
    while (optabFile >> opcode >> mnemonic)
    {
        opCodeTable[opcode] = mnemonic;
    }

    optabFile.close();
}

// Function to process a line of assembly code and extract its components
void processLine(const string &line, Instruction *instruction)
{
    if (line == "")
        return;
    istringstream iss(line);

    // Skip leading whitespaces
    iss >> std::ws;

    // Check if the line is a comment
    if (iss.peek() == '.')
    {
        instruction->label = ".";
        instruction->opcode = "";
        instruction->operand = "";
        return;
    }

    // Count the words in the line
    size_t wordCount = 0;
    string word;
    while (iss >> word)
    {
        wordCount++;
    }

    // Reset the stream to the beginning
    iss.clear();
    iss.seekg(0);

    // Process based on word count
    if (wordCount == 1)
    {
        // Only opcode is present
        instruction->label = "";
        iss >> instruction->opcode;
        instruction->operand = "";
    }
    else if (wordCount == 2)
    {
        // Opcode and operand are present
        instruction->label = "";
        iss >> instruction->opcode >> instruction->operand;
    }
    else if (wordCount == 3)
    {
        // Label, opcode, and operand are present
        iss >> instruction->label >> instruction->opcode >> instruction->operand;
    }
    else
    {
        iss >> instruction->label >> instruction->opcode;
        if (instruction->opcode == "BYTE")
            // For BYTE opcode with more than 3 words, consider the operand as a single word
            getline(iss >> std::ws, instruction->operand);
        else
        {
            instruction->label = ".";
            instruction->opcode = "";
            instruction->operand = "";
        }
    }
}

// Function to format a number as a hexadecimal string with a specified width
string formatNumber(string input, int width)
{
    stringstream temp;
    temp << hex << uppercase << setfill('0') << setw(width) << stoi(input);
    return temp.str();
}

// Function to format a string with a specified width and left alignment
string formatName(string name, int width)
{
    stringstream temp;
    temp << left << setfill(' ') << setw(width) << name;
    return temp.str();
}

int main()
{
    // Open input and output files
    ifstream fp1("input.dat");
    ofstream fp2("intermediate.dat");

    int LOCCTR = 0, STADDR = 0, LENGTH;
    loadOpCodeTable(); // Load opcode table from optab.dat

    bool flag = false;
    string line;

    // Process each line of the input file
    while (getline(fp1, line))
    {
        Instruction instr;
        processLine(line, &instr);

        if (instr.label != ".")
        {
            if (instr.opcode == "START")
            {
                // Handle START directive
                STADDR = stoi(instr.operand, NULL, 16);
                LOCCTR = STADDR;
                fp2 << formatNumber(to_string(LOCCTR), 4) << "\t" << formatName(instr.label, 6) << "\t" << formatName(instr.opcode, 6) << "\t" << formatName(instr.operand, 6) << '\n';
            }
            else if (instr.opcode == "END")
            {
                // Handle END directive
                LENGTH = LOCCTR - STADDR;
                fp2 << formatName("", 4) << "\t\t\t" << formatName(instr.opcode, 6) << "\t" << formatName(instr.operand, 6);
                break;
            }
            else
            {
                // Handle other instructions
                if (instr.label != "")
                {
                    if (symTab.find(instr.label) == symTab.end())
                        symTab[instr.label] = LOCCTR;
                    else
                    {
                        flag = true; // duplicate symbol
                        cout << "Duplicate symbol\n";
                    }
                }
                fp2 << formatNumber(to_string(LOCCTR), 4) << "\t" << formatName(instr.label, 6) << "\t" << formatName(instr.opcode, 6) << "\t" << formatName(instr.operand, 6) << '\n';

                if (opCodeTable.find(instr.opcode) != opCodeTable.end())
                    LOCCTR += 3;
                else if (instr.opcode == "BYTE")
                {
                    // Handle BYTE directive
                    if (instr.operand[0] == 'C' || instr.operand[0] == 'L')
                        LOCCTR += (instr.operand.length() - 3);
                    if (instr.operand[0] == 'X')
                        LOCCTR += (instr.operand.length() - 3) / 2;
                }
                else if (instr.opcode == "RESB")
                    // Handle RESB directive
                    LOCCTR += stoi(instr.operand);
                else if (instr.opcode == "RESW")
                    // Handle RESW directive
                    LOCCTR += 3 * stoi(instr.operand);
                else if (instr.opcode == "WORD")
                    // Handle WORD directive
                    LOCCTR += 3;
                else
                {
                    flag = true; // invalid operation code
                    cout << "Invalid operation code\n";
                    cout << line << endl;
                }
            }
        }
        else
        {
            // If the line is a comment, copy it to the output file
            fp2 << line << '\n';
        }
    }

    int PROGRAM_LENGTH = LENGTH;

    // Save symTab to a file
    ofstream fp3("symTab.dat");
    for (const auto &entry : symTab)
    {
        fp3 << entry.first << " " << entry.second << endl;
    }
    fp3 << "Program_Length " << to_string(LENGTH);

    // Close files
    fp1.close();
    fp2.close();
    fp3.close();
    return 0;
}
