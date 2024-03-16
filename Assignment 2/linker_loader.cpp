#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

int PROGADDR;
int LAST;
vector<string> memory((1 << 18), "..");

map<string, int> ExSymTab;

bool isNumber(string s)
{
    auto it = s.begin();
    while (it != s.end() && isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

string removeSpaces(string s)
{
    s.erase(remove(s.begin(), s.end(), ' '), s.end());
    return s;
}

string formatNumber(int num, int width, char pad = '0')
{
    stringstream temp;
    temp << std::hex << std::uppercase << std::setfill(pad) << std::setw(width) << num;
    return temp.str();
}

string formatString(string name, int width, char pad = ' ')
{
    stringstream temp;
    temp << std::left << std::setfill(pad) << std::setw(width) << name;
    return temp.str();
}

vector<pair<string, int>> getSymbols(string record)
{
    vector<pair<string, int>> symbols;
    int n = record.length();
    for (int i = 1; i < n; i += 12)
    {
        string symbol = record.substr(i, 6);
        int value = stoi(record.substr(i + 6, 6), nullptr, 16);
        symbols.push_back(make_pair(symbol, value));
    }
    return symbols;
}

void linker_pass1(string input)
{
    ifstream fp(input);

    if (!fp.is_open())
    {
        perror(input.c_str());
        exit(1);
    }

    // Take program address as input from the user in hex
    string progaddr;
    cout << "Enter PROGADDR: ";
    cin >> progaddr;
    PROGADDR = stoi(progaddr, nullptr, 16);

    int CSADDR = PROGADDR;
    int CSLTH = 0;
    string record;

    while (fp.good())
    {
        // read record
        getline(fp, record);

        // end of input
        if (record.empty())
            break;

        if (record.front() == 'H')
        {
            // Update CSADDR
            CSADDR = CSADDR + CSLTH;

            // CSECT and CSLTH from the record
            string CSECT = record.substr(1, 6);
            CSLTH = stoi(record.substr(13, 6), nullptr, 16);

            // Enter the CSECT to ExSymTab
            if (ExSymTab.find(CSECT) == ExSymTab.end())
                ExSymTab.insert({CSECT, CSADDR});
            else
            {
                perror("Duplicate external symbol");
                exit(1);
            }
        }

        if (record.front() == 'D')
        {
            // get the symbols in the record
            for (pair<string, int> symbol : getSymbols(record))
            {
                if (ExSymTab.find(symbol.first) == ExSymTab.end())
                    ExSymTab.insert({symbol.first, symbol.second + CSADDR});
                else
                {
                    perror("Duplicate external symbol");
                    exit(1);
                }
            }
        }
    }
    LAST = CSADDR + CSLTH;
}

void linker_pass2(string input)
{
    fstream fp(input);

    if (!fp.is_open())
    {
        perror(input.c_str());
        exit(1);
    }

    int CSADDR = PROGADDR;
    int EXECADDR = PROGADDR;
    int CSLTH = 0;

    string record;
    while (fp.good())
    {
        getline(fp, record);

        // if end of input
        if (record.empty())
            break;

        if (record.front() == 'H')
            CSLTH = stoi(record.substr(13, 6), nullptr, 16);

        if (record.front() == 'T')
        {
            // move the record to its appropriate memory location byte by byte
            int STADDR = stoi(record.substr(1, 6), nullptr, 16) + CSADDR;
            int INDEX = 0;
            for (int i = 9; i < (int)record.length(); i += 2)
                memory[STADDR + INDEX++] = record.substr(i, 2);
        }

        else if (record.front() == 'M')
        {
            string symbol = record.substr(10, 6);
            if (ExSymTab.find(symbol) != ExSymTab.end())
            {
                // extract address to be modified
                int address = stoi(record.substr(1, 6), nullptr, 16) + CSADDR;
                int length = stoi(record.substr(7, 2), nullptr, 16);

                char halfByte;
                if (length % 2)
                    halfByte = memory[address][0];

                string val = "";
                for (int i = 0; i < (length + 1) / 2; i++)
                    val += memory[address + i];
                int value = stoi(val, nullptr, 16);

                cout << "value          = " << val << endl;

                // apply modification
                int modification = ExSymTab[symbol];
                char sign = record[9];
                if (sign == '+')
                    value += modification;
                else
                    value -= modification;

                // apply mask
                int mask = 0;
                for (int i = 0; i < (length + length % 2) * 4; i++)
                {
                    mask |= 1;
                    mask = mask << 1;
                }
                mask = mask >> 1;
                value = value & mask;

                // write back the modified value
                val = formatNumber(value, length + (length % 2));
                cout << "modification   = " << formatNumber(modification, 6) << endl;
                cout << "modified value = " << val << endl;
                for (int i = 0; i < length; i += 2)
                    memory[address + i / 2] = val.substr(i, 2);

                if (length % 2)
                    memory[address][0] = halfByte;
            }
            else
            {
                cout << symbol << endl;
                perror("undefined symbol");
                exit(1);
            }
        }

        if (record.front() == 'E')
        {
            if (record != "E")
            {
                int FIRST = stoi(record.substr(1, 6), nullptr, 16);
                EXECADDR = CSADDR + FIRST;
            }
            CSADDR = CSLTH + CSADDR;
        }
    }

    cout << "Starting execution at: " << formatNumber(EXECADDR, 4) << endl;
}

void print_memory_map()
{
    ofstream fp("memory.dat");
    int i = (PROGADDR / 16) * 16;
    int n = ((LAST + 16) / 16) * 16;
    // cout << i << " " << n << endl;
    while (i < n)
    {
        fp << formatNumber(i, 4) << ' ';

        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
                fp << memory[i++];
            fp << ' ';
        }
        fp << endl;
    }

    // close the memory file
    fp.close();
}

int main()
{
    // run the  2-pass link loader assembler
    linker_pass1("output.dat");
    ofstream fp("exSymTab.dat");

    for (auto x : ExSymTab)
    {
        cout << x.first << ' ' << formatNumber(x.second, 4) << endl;
        fp << x.first << "\t" << formatNumber(x.second, 4) << endl;
    }
    linker_pass2("output.dat");
    // cout << "----------------------" << endl;

    print_memory_map();
    return 0;
}