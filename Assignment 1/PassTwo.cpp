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
int length;

// Function to load symbol table from a file
void loadSymTable()
{
    ifstream symtabFile("symTab.dat");
    if (!symtabFile)
    {
        cerr << "Error opening symTab file\n";
        exit(EXIT_FAILURE);
    }

    string mnemonic, opcode;
    while (symtabFile >> opcode >> mnemonic)
    {
        if (opcode == "Program_Length")
            length = stoi(mnemonic);
        else
            symTab[opcode] = stoi(mnemonic);
    }

    symtabFile.close();
}

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

// Function to retrieve opcode from the opcode table
string OPTAB(string opcode)
{
    auto it = opCodeTable.find(opcode);
    if (it != opCodeTable.end())
    {
        return it->second; // Return corresponding opcode
    }
    else
    {
        return ""; // Return empty string if opcode not found
    }
}

string strip(string s)
{
    s.erase(remove(s.begin(), s.end(), ' '), s.end());
    return s;
}

// Function to process a line of intermediate code
string process_line(const string &line, Instruction *instruction)
{
    string label, opcode, operand, LOCCTR;

    if (line == "")
        return "";
    istringstream iss(line);

    // Skip leading whitespaces
    iss >> std::ws;

    // Check if the line is a comment
    if (iss.peek() == '.')
    {
        instruction->label = ".";
        instruction->opcode = "";
        instruction->operand = "";
        return "";
    }

    // Check if the line is END line
    if (iss.peek() == 'E')
    {
        instruction->label = "";
        iss >> instruction->opcode >> instruction->operand;
        return "";
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
    if (wordCount == 2)
    {
        // Only opcode is present
        iss >> LOCCTR >> instruction->opcode;
        instruction->label = "";
        instruction->operand = "";
    }
    else if (wordCount == 3)
    {
        // Opcode and operand are present
        instruction->label = "";
        iss >> LOCCTR >> instruction->opcode >> instruction->operand;
    }
    else if (wordCount == 4)
    {
        // Label, opcode, and operand are present
        iss >> LOCCTR >> instruction->label >> instruction->opcode >> instruction->operand;
    }
    else
    {
        iss >> LOCCTR >> instruction->label >> instruction->opcode;
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
    return LOCCTR;
}

// Function to format a number as a hexadecimal or decimal string with a specified width
string formatNumber(string input, int width, bool hexi)
{
    int num;
    if (hexi)
        num = stoi(input, NULL, 16);
    else
        num = stoi(input);

    stringstream temp;
    temp << hex << uppercase << setfill('0') << setw(width) << num;
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
    fstream fin, fout, fout2;

    // Open input, output, and listing files
    fin.open("intermediate.dat", ios::in);
    fout.open("output.dat", ios::out);
    fout2.open("listing.dat", ios::out);

    string text_record, staddr;
    bool error = false;

    loadSymTable();
    loadOpCodeTable();
    cout << "Program Length:" << length << endl;

    // Process each line of the intermediate code
    while (fin.good())
    {
        string line;
        getline(fin, line);

        Instruction instr;
        string LOCCTR = process_line(line, &instr);

        if (instr.label != ".")
        {
            if (instr.opcode == "START")
            {                
                // Handle START directive
                stringstream header;
                header << "H";
                header << formatName(instr.label, 6);
                header << formatNumber(instr.operand, 6, true);
                header << formatNumber(to_string(length), 6, false);
                fout << header.str() << '\n';
                fout2 << formatNumber(LOCCTR, 4, false) << "\t" << formatName(instr.label, 7) << "\t" << formatName(instr.opcode, 7) << "\t" << formatName(instr.operand, 7) << '\n';
            }
            else if (instr.opcode == "END")
            {
                // Handle END directive
                if (text_record.length())
                {
                    int length = (text_record.length()) / 2;
                    fout << "T" << formatNumber(staddr, 6, true) << formatNumber(to_string(length), 2, false) << text_record << '\n';
                }
                stringstream end;
                int first = symTab[instr.operand];
                end << "E";
                end << formatNumber(to_string(first), 6, false);
                fout << end.str() << '\n';
                fout2 << formatName("", 4) << "\t\t\t" << formatName(instr.opcode, 7) << "\t" << formatName(instr.operand, 7) << '\n';
            }
            else
            {
                // Handle other instructions
                string obcode = "";
                if (OPTAB(instr.opcode) != "")
                {
                    obcode = OPTAB(instr.opcode);
                    if (instr.operand != "")
                    {
                        int operand = 0;
                        if (instr.operand.find(",") != string::npos)
                        {
                            instr.operand.resize(instr.operand.find(","));
                            operand += 32768;
                        }
                        if (symTab.find(instr.operand) != symTab.end())
                        {
                            operand += symTab[instr.operand];
                            obcode += formatNumber(to_string(operand), 4, false);
                        }
                        else
                        {
                            obcode += formatNumber("0", 4, false);
                            error = true; // undefined symbol
                            cout << "undefined symbol\n";
                        }
                    }
                    else
                        obcode += formatNumber("0", 4, false);
                }
                else
                {
                    if (instr.opcode == "BYTE")
                    {
                        // Handle BYTE directive
                        if (instr.operand[0] == 'C')
                        {
                            string constant = instr.operand.substr(2, instr.operand.length() - 3);
                            for (char ch : constant)
                            {
                                int x = ch;
                                obcode += formatNumber(to_string(x), 2, false);
                            }
                        }
                        else
                        {
                            string constant = instr.operand.substr(2, instr.operand.length() - 3);
                            obcode += formatNumber(constant, constant.length(), true);
                        }
                    }
                    else if (instr.opcode == "WORD")
                    {    // Handle WORD directive
                        obcode += formatNumber(instr.operand, 6, false);
                    }
                }

                fout2 << formatNumber(LOCCTR, 4, false) << "\t" << formatName(instr.label, 7) << "\t" << formatName(instr.opcode, 7) << "\t" << formatName(instr.operand, 10) << "\t" << formatName(obcode, 10) << '\n';

                if (text_record.length() == 0)
                {
                    staddr = LOCCTR;
                }

                if (text_record.length() + obcode.length() <= 60 && instr.opcode != "RESW" && instr.opcode != "RESB")
                {
                    text_record += obcode;
                }
                else
                {
                    if (text_record.length())
                    {
                        int length = (text_record.length()) / 2;
                        fout << "T" << formatNumber(staddr, 6, true) << formatNumber(to_string(length), 2, false) << text_record << '\n';
                        text_record = obcode;
                        staddr = LOCCTR;
                    }
                }
            }
        }
        else
        {
            // If the line is a comment, copy it to the output listing file
            fout2 << line << '\n';
        }
    }
    return 0;
}
