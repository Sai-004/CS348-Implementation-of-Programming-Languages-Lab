#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <stack>

using namespace std;

// Global maps for opcode table
map<string, pair<string, string>> opCodeTable;

// helper functions
string formatNumber(int num, int width)
{
    stringstream temp;
    temp << hex << uppercase << setfill('0') << setw(width) << num;
    return temp.str();
}

string formatString(string name, int width)
{
    stringstream temp;
    temp << left << setfill(' ') << setw(width) << name;
    return temp.str();
}

bool isNumber(string s)
{
    auto it = s.begin();
    while (it != s.end() && isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
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

    string mnemonic, opcode, i;
    while (optabFile >> opcode >> i >> mnemonic)
    {
        opCodeTable[opcode] = {i, mnemonic};
    }

    optabFile.close();
}

// Function to retrieve opcode from the opcode table
pair<int, string> OPTAB(string opcode)
{
    auto it = opCodeTable.find(opcode);
    if (it != opCodeTable.end())
    {
        return {stoi((it->second).first), (it->second).second}; // Return corresponding opcode
    }
    else
    {
        return {-1, "-1"}; // Return empty string if opcode not found
    }
}

class SYMBOL
{
    string CSECT;
    string name;

public:
    SYMBOL(const string &CSECT, const string &name) : CSECT(CSECT), name(name) {}

    bool operator<(const SYMBOL &s) const
    {
        if (CSECT != s.CSECT)
            return CSECT < s.CSECT;
        return name < s.name;
    }

    // Getter functions
    const string &getCSECT() const { return CSECT; }
    const string &getName() const { return name; }
};

class VALUE
{

    int value;
    bool type; // 0: absolute, 1: relative
    int length;

public:
    // Constructor with default parameter values
    VALUE(int value = 0, int length = 3, bool type = true) : value(value), length(length), type(type) {}

    // Getter functions
    int getValue() const { return value; }
    bool getType() const { return type; }
    int getLength() const { return length; }

    // Setter functions
    void setValue(int newValue) { value = newValue; }
};

// Symbol Table SYMTAB[(csect,name)] = address
map<SYMBOL, VALUE> SYMTAB;

// Literal Pool LITTAB[(csect,name)] = address
map<SYMBOL, VALUE> LITTAB;

// Some preprocessed constants in SYMTAB
void preprocess(string CSECT)
{
    // other mnemonics
    SYMTAB.insert({SYMBOL(CSECT, "A"), VALUE(0, 3, 0)});
    SYMTAB.insert({SYMBOL(CSECT, "X"), VALUE(1, 3, 0)});
    SYMTAB.insert({SYMBOL(CSECT, "L"), VALUE(2, 3, 0)});
    SYMTAB.insert({SYMBOL(CSECT, "B"), VALUE(3, 3, 0)});
    SYMTAB.insert({SYMBOL(CSECT, "S"), VALUE(4, 3, 0)});
    SYMTAB.insert({SYMBOL(CSECT, "T"), VALUE(5, 3, 0)});
    SYMTAB.insert({SYMBOL(CSECT, "F"), VALUE(6, 3, 0)});
    SYMTAB.insert({SYMBOL(CSECT, "PC"), VALUE(8, 3, 0)});
    SYMTAB.insert({SYMBOL(CSECT, "SW"), VALUE(9, 3, 0)});
}

struct evalExpression
{
    string CSECT;
    evalExpression(string CSECT)
    {
        this->CSECT = CSECT;
    }
    int precedence(string op)
    {
        if (op == "+" || op == "-")
            return 1;
        if (op == "*" || op == "/")
            return 2;
        return 0;
    }
    pair<int, int> applyOperator(pair<int, int> a, pair<int, int> b, string op)
    {
        if (op == "+")
            return make_pair(a.first + b.first, a.second + b.second);
        if (op == "-")
            return make_pair(a.first - b.first, a.second - b.second);
        if (op == "*")
            return make_pair(a.first * b.first, a.second * b.second);
        if (op == "/")
            return make_pair(a.first / b.first, a.second / b.second);
        return make_pair(0, 0);
    }

    vector<string> tokenize(string exp)
    {
        int n = exp.length();
        vector<string> tokens;
        int i = 0;
        while (i < n)
        {
            if (exp[i] == '*' || exp[i] == '+' || exp[i] == '-' || exp[i] == '/' || exp[i] == ')' || exp[i] == '(')
            {
                tokens.push_back(exp.substr(i, 1));
                i++;
            }
            else
            {
                int j = i;
                while (j < n && !(exp[j] == '*' || exp[j] == '+' || exp[j] == '-' || exp[j] == '/' || exp[j] == '(' || exp[j] == ')'))
                    j++;
                tokens.push_back(exp.substr(i, j - i));
                i = j;
            }
        }
        return tokens;
    }

    pair<int, int> evaluate(string exp)
    {
        vector<string> tokens = tokenize(exp);

        stack<pair<int, int>> operands;
        stack<int> type;
        stack<string> operators;

        int n = tokens.size();
        for (int i = 0; i < n; i++)
        {
            if (tokens[i] == "(")
                operators.push("(");
            else if (isNumber(tokens[i]))
            {
                operands.push(make_pair(stoi(tokens[i]), stoi(tokens[i])));
            }
            else if (tokens[i] == ")")
            {
                while (!operators.empty() && operators.top() != "(")
                {
                    pair<int, int> operand_2 = operands.top();
                    operands.pop();

                    pair<int, int> operand_1 = operands.top();
                    operands.pop();

                    string op = operators.top();
                    operators.pop();

                    operands.push(applyOperator(operand_1, operand_2, op));
                }
                if (!operators.empty())
                    operators.pop();
            }
            else if (tokens[i] == "+" || tokens[i] == "-" || tokens[i] == "*" || tokens[i] == "/")
            {
                while (!operators.empty() && precedence(operators.top()) >= precedence(tokens[i]))
                {
                    pair<int, int> operand_2 = operands.top();
                    operands.pop();

                    pair<int, int> operand_1 = operands.top();
                    operands.pop();

                    string op = operators.top();
                    operators.pop();

                    operands.push(applyOperator(operand_1, operand_2, op));
                }

                operators.push(tokens[i]);
            }
            else
            {
                if (SYMTAB.find(SYMBOL(CSECT, tokens[i])) != SYMTAB.end())
                {
                    int value = SYMTAB[SYMBOL(CSECT, tokens[i])].getValue();
                    int type = SYMTAB[SYMBOL(CSECT, tokens[i])].getType();
                    operands.push(make_pair(type, value));
                }
                else
                {
                    operands.push(make_pair(0, 0));
                }
            }
        }
        while (!operators.empty())
        {
            pair<int, int> operand_2 = operands.top();
            operands.pop();

            pair<int, int> operand_1 = operands.top();
            operands.pop();

            string op = operators.top();
            operators.pop();

            operands.push(applyOperator(operand_1, operand_2, op));
        }
        return operands.top();
    }
};

// instruction class
struct Instruction
{
    string label;
    string opcode;
    string operands;
    int length;
    bool i, n, p, b, e, x;
    bool literal;

    // default constructor
    Instruction(string label = "", string opcode = "", string operands = "", int length = 3, bool i = 1, bool n = 1, bool p = 1, bool b = 0, int e = 0, int literal = 0, int x = 0)
    {
        this->label = label;
        this->opcode = opcode;
        this->operands = operands;
        this->length = length;
        this->i = i;
        this->n = n;
        this->p = p;
        this->b = b;
        this->e = e;
        this->x = x;
        this->literal = literal;
    }

    // parse multiple operands
    vector<string> getOperands()
    {
        vector<string> m_operands;
        int i = 0;
        int l = operands.length();
        while (i < l)
        {
            int j = i + 1;
            while (j < l && operands[j] != ',')
                j++;
            m_operands.push_back(operands.substr(i, j - i));
            i = j + 1;
        }
        return m_operands;
    }

    // work in the flags for opcode
    void formatOpcode(int &op)
    {
        if (length == 3 || length == 4)
        {
            op |= i;
            op |= (n << 1);
        }
    }

    // work in the flags for operands
    void formatOperand(int &operand)
    {
        if (length == 3 || length == 4)
        {
            // apply mask for negative operand
            int mask = 0;
            for (int i = 0; i < ((length == 3) ? 12 : 20); i++)
            {
                mask |= 1;
                mask = mask << 1;
            }
            mask = mask >> 1;
            operand = operand & mask;

            if (!e)
            {
                operand |= (p << 13);
                operand |= (b << 14);
                operand |= (x << 15);
            }
            else
            {
                operand |= (e << 20);
                operand |= (p << 21);
                operand |= (b << 22);
                operand |= (x << 23);
            }
        }
    }
};

// takes a line from source code and tokenize the line
void processLine(string line, Instruction *instr)
{
    if (line == "")
        return;
    istringstream iss(line);

    // Skip leading whitespaces
    iss >> ws;

    // Check if the line is a comment
    if (iss.peek() == '.')
    {
        instr->label = ".";
        instr->opcode = "";
        instr->operands = "";
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
        instr->label = "";
        iss >> instr->opcode;
        instr->operands = "";
    }
    else if (wordCount == 2)
    {
        // Opcode and operand are present
        instr->label = "";
        iss >> instr->opcode >> instr->operands;

        // CSECT don't have operands
        if (instr->operands == "CSECT")
        {
            instr->label = instr->opcode;
            instr->opcode = "CSECT";
            instr->operands = "";
        }
    }
    else if (wordCount == 3)
    {
        // Label, opcode, and operand are present
        iss >> instr->label >> instr->opcode >> instr->operands;
    }
    else
    {
        iss >> instr->label >> instr->opcode;
        if (instr->opcode == "BYTE")
            // For BYTE opcode with more than 3 words, consider the operand as a single word
            getline(iss >> ws, instr->operands);
        else
        {
            instr->label = ".";
            instr->opcode = "";
            instr->operands = "";
        }
    }

    // extended format instruction
    if (instr->opcode[0] == '+')
    {
        instr->e = 1;
        instr->p = 0;
        instr->b = 0;
        instr->n = 1;
        instr->i = 1;
        instr->opcode.erase(0, 1);
    }

    // operand types
    char temp = instr->operands[0];
    // immediate
    if (temp == '#')
    {
        instr->i = 1;
        instr->n = 0;
        if (isNumber(instr->operands))
        {
            instr->p = 0;
            instr->b = 0;
        }
        instr->operands.erase(0, 1);
    }
    // indirect
    else if (temp == '@')
    {
        instr->i = 0;
        instr->n = 1;
        instr->operands.erase(0, 1);
    }
    else if (temp == '=')
    {
        instr->literal = true;
        instr->operands.erase(0, 1);
    }
    cout << instr->label << " |" << instr->opcode << " |" << instr->operands << " |" << instr->literal << endl;
}

int main()
{
    // Open input and output files
    ifstream fp1("input.dat");
    ofstream fp2("intermediate.dat");

    // some useful variables' initialisation
    int LOCCTR = 0, STADDR = 0, LENGTH;
    string CSECT = "";

    loadOpCodeTable(); // Load opcode table from optab.dat

    bool flag = false;
    string line;

    // Process each line of the input file
    while (getline(fp1, line))
    {
        // end of file stop reading
        if (line.empty())
            break;

        Instruction instr;
        processLine(line, &instr);

        if (instr.label != ".")
        {
            if (instr.opcode == "START" || instr.opcode == "CSECT")
            {
                // enter the previous CSECT in the SYMTAB
                if (!CSECT.empty())
                {
                    LENGTH = LOCCTR - STADDR;
                    SYMTAB.insert({SYMBOL(CSECT, CSECT), VALUE(STADDR, LENGTH)});
                }

                // initialize LOCCTR ,STADDR and CSECT
                if (instr.operands != "")
                    STADDR = stoi(instr.operands, NULL, 16);
                else
                    STADDR = 0;

                LOCCTR = STADDR;
                CSECT = instr.label;

                // initialize some symbols for the csection
                preprocess(CSECT);

                // write to intermediate file
                fp2 << formatNumber(LOCCTR, 4) << "\t" << line << "\n";
                // fp2 << formatNumber(LOCCTR, 4) << "\t" << formatString(instr.label, 6) << "\t\t" << formatString(instr.opcode, 6) << "\t\t" << instr.operands << '\n';
            }
            else if (instr.opcode == "END" || instr.opcode == "LTORG")
            {
                // write to intermediate file
                fp2 << formatString("", 4) << "\t" << line << "\n";
                // fp2 << formatString("", 4) << "\t\t\t\t" << formatString(instr.opcode, 6) << "\t\t" << formatString(instr.operands, 6) << "\n";

                // Update LITTAB
                for (auto &x : LITTAB)
                {
                    if (x.second.getValue() == -1)
                    {
                        // write to intermediate file
                        fp2 << formatNumber(LOCCTR, 4) << "\t" << formatString("*", 6) << "\t\t=" << formatString(x.first.getName(), 6) << '\n';

                        // Update literal address
                        x.second.setValue(LOCCTR);

                        // Update LOCCTR according to the literal type
                        if (x.first.getName().front() == 'C')
                            LOCCTR += (x.first.getName().length() - 3);
                        else if (x.first.getName().front() == 'X')
                            LOCCTR += ((x.first.getName().length() - 3) / 2);
                        else
                            LOCCTR += 3;
                    }
                }

                // enter the last CSECT in the SYMTAB
                if (instr.opcode == "END")
                {
                    LENGTH = LOCCTR - STADDR;
                    SYMTAB.insert({SYMBOL(CSECT, CSECT), VALUE(STADDR, LENGTH)});
                }
            }
            else if (instr.opcode == "BASE")
            {
                // write to intermediate file
                fp2 << formatNumber(LOCCTR, 4) << "\t" << line << "\n";
                // fp2 << formatNumber(LOCCTR, 4) << "\t" << formatString(instr.label, 6) << "\t\t" << formatString(instr.opcode, 6) << "\t\t" << formatString(instr.operands, 6) << "\n";
            }
            else if (instr.opcode == "EQU")
            {
                if (instr.label != "")
                {
                    if (SYMTAB.find(SYMBOL(CSECT, instr.label)) == SYMTAB.end())
                    {
                        if (instr.operands == "*")
                        {
                            SYMTAB.insert({SYMBOL(CSECT, instr.label), VALUE(LOCCTR)});

                            // write to intermediate file
                            fp2 << formatNumber(LOCCTR, 4) << "\t" << line << "\n";
                            // fp2 << formatNumber(LOCCTR, 4) << "\t" << formatString(instr.label, 6) << "\t\t" << formatString(instr.opcode, 6) << "\t\t" << formatString(instr.operands, 6) << "\n";
                        }
                        else if (isNumber(instr.operands)) // assumption that number is in decimal format only
                        {
                            SYMTAB.insert({SYMBOL(CSECT, instr.label), VALUE(stoi(instr.operands), 3, 0)});

                            // write to intermediate file
                            fp2 << formatNumber(stoi(instr.operands), 4) << "\t" << line << "\n";
                            // fp2 << formatNumber(stoi(instr.operands), 4) << "\t" << formatString(instr.label, 6) << "\t\t" << formatString(instr.opcode, 6) << "\t\t" << formatString(instr.operands, 6) << "\n";
                        }
                        else
                        {
                            // Evaluate the expression
                            evalExpression e(CSECT);
                            int value = e.evaluate(instr.operands).second;
                            int type = e.evaluate(instr.operands).first;
                            if (type != 0 && type != -1 && type != 1)
                            {
                                perror("Invalid Expression");
                                exit(1);
                            }

                            if (value < 0)
                            {
                                int mask = 0;
                                for (int i = 0; i < 16; i++)
                                {
                                    mask |= 1;
                                    mask = mask << 1;
                                }
                                mask = mask >> 1;
                                value = value & mask;
                            }

                            // write to intermediate file
                            fp2 << formatNumber(value, 4) << "\t" << line << "\n";
                            // fp2 << formatNumber(value, 4) << "\t" << formatString(instr.label, 6) << "\t\t" << formatString(instr.opcode, 6) << "\t\t" << formatString(instr.operands, 6) << "\n";
                        }
                    }
                    else
                    {
                        perror("duplicate symbol");
                        exit(1);
                    }
                }
            }
            else if (instr.opcode == "EXTDEF" || instr.opcode == "EXTREF")
            {
                // write to intermediate file
                fp2 << formatString("", 4) << "\t" << line << "\n";
                // fp2 << "\t\t\t\t\t" << formatString(instr.opcode, 6) << "\t\t" << formatString(instr.operands, 6) << "\n";
            }
            else
            {
                // write to intermediate file
                fp2 << formatNumber(LOCCTR, 4) << "\t" << line << "\n";
                // fp2 << formatNumber(LOCCTR, 4) << "\t" << formatString(instr.label, 6) << "\t\t" << formatString(instr.opcode, 6) << "\t\t" << formatString(instr.operands, 6) << "\n";

                // Update LITTAB
                if (instr.literal)
                {
                    if (LITTAB.find(SYMBOL(CSECT, instr.operands)) == LITTAB.end())
                    {
                        LITTAB.insert({SYMBOL(CSECT, instr.operands), VALUE(-1)});
                    }
                }

                // Update SYMTAB
                if (instr.label != "")
                {
                    if (SYMTAB.find(SYMBOL(CSECT, instr.label)) == SYMTAB.end())
                    {
                        SYMTAB.insert({SYMBOL(CSECT, instr.label), VALUE(LOCCTR)});
                    }    
                    else
                    {
                        perror("duplicate symbol");
                        exit(1);
                    }
                }

                // Update LOCCTR
                if (opCodeTable.find(instr.opcode) != opCodeTable.end())
                {
                    pair<int, string> format = OPTAB(instr.opcode);
                    // extended instruction format
                    if (instr.e)
                        format.first++;

                    instr.length = format.first;
                    LOCCTR += instr.length;
                }
                else if (instr.opcode == "WORD")
                    LOCCTR += 3;
                else if (instr.opcode == "RESW")
                    LOCCTR += 3 * stoi(instr.operands);
                else if (instr.opcode == "RESB")
                    LOCCTR += stoi(instr.operands);
                else if (instr.opcode == "BYTE")
                {
                    if (instr.operands.front() == 'C')
                        LOCCTR += (instr.operands.length() - 3);
                    if (instr.operands.front() == 'X')
                        LOCCTR += ((instr.operands.length() - 3) / 2);
                }
                else
                {
                    perror("Invalid Operation Code\n");
                    cout << line << endl;
                    exit(1);
                }
            }
        }
        else
        {
            // write to intermediate file
            fp2 << line << '\n';
        }
    }

    // closing files
    fp1.close();
    fp2.close();

    ofstream fst("symTab.dat");
    ofstream flt("litTab.dat");

    // Iterate through SYMTAB and print each element in file
    for (const auto &entry : SYMTAB) {
        const SYMBOL &symbol = entry.first;
        const VALUE &value = entry.second;

        fst << symbol.getCSECT() << " " << symbol.getName()
            << " " << value.getValue() << " " << value.getLength() << " " << value.getType() << endl;
    }

    // Iterate through LITTAB and print each element
    for (const auto &entry : LITTAB) {
        const SYMBOL &symbol = entry.first;
        const VALUE &value = entry.second;

        flt << symbol.getCSECT() << " " << symbol.getName()
            << " " << value.getValue() << " " << value.getLength() << " " << value.getType() << endl;
    }

    fst.close();
    flt.close();

    return 0;
}
