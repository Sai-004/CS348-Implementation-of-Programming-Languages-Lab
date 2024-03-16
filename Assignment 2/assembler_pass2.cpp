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

// Function to load opcode table from a file
void loadOpCodeTable()
{
    ifstream optabFile("opTab.dat");
    if (!optabFile)
    {
        cerr << "Error opening optab file\n";
        exit(EXIT_FAILURE);
    }

    string mnemonic, opcode, format;
    while (optabFile >> opcode >> format >> mnemonic)
    {
        opCodeTable[opcode] = {format, mnemonic};
    }

    optabFile.close();
}

// Function to retrieve opcode from the opcode table
pair<int, string> OPTAB(string opcode)
{
    auto it = opCodeTable.find(opcode);

    if (it != opCodeTable.end())
        return {stoi((it->second).first), (it->second).second}; // Return corresponding opcode
    else
        return {-1, "-1"}; // Return empty string if opcode not found
}

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

void applyMask(int &value, int bits)
{
    if (value < 0)
    {
        int mask = 0;
        for (int i = 0; i < bits; i++)
        {
            mask |= 1;
            mask = mask << 1;
        }
        mask = mask >> 1;
        value = value & mask;
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
    int getLength() const { return length; }
    bool getType() const { return type; }

    // Setter functions
    void setValue(int newValue) { value = newValue; }
};

// Symbol Table SYMTAB[(csect,name)] = address
map<SYMBOL, VALUE> SYMTAB;

// Literal Pool LITTAB[(csect,name)] = address
map<SYMBOL, VALUE> LITTAB;

// Function to read SYMTAB from file
void readSymtabFromFile(map<SYMBOL, VALUE> &SYMTAB)
{
    ifstream symtabFile("symTab.dat");
    if (!symtabFile)
    {
        cerr << "Error opening symtab file\n";
        exit(EXIT_FAILURE);
    }

    string csect, name;
    int value, length;
    bool type;
    while (symtabFile >> csect >> name >> value >> length >> type)
    {
        // Create SYMBOL object
        SYMBOL symbol(csect, name);

        // Create VALUE object
        VALUE val(value, length, type);

        // Insert into SYMTAB map
        SYMTAB.insert({symbol, val});
    }

    symtabFile.close();
}

// Function to read LITTAB from file
void readLittabFromFile(map<SYMBOL, VALUE> &LITTAB)
{
    ifstream littabFile("litTab.dat");
    if (!littabFile)
    {
        cerr << "Error opening litTab file\n";
        exit(EXIT_FAILURE);
    }

    string csect, name;
    int value, length;
    bool type;
    while (littabFile >> csect >> name >> value >> length >> type)
    {
        // Create SYMBOL object
        SYMBOL symbol(csect, name);

        // Create VALUE object
        VALUE val(value, length, type);

        // Insert into LITTAB map
        LITTAB.insert({symbol, val});
    }

    littabFile.close();
}

set<string> EXTREF;

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
                    if (EXTREF.find(tokens[i]) != EXTREF.end())
                    {
                    }
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
    int ext_op = 0, oper_type = 0;

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
int processLine(string line, Instruction *instr)
{
    if (line == "")
        return -1;
    istringstream iss(line);

    // Skip leading whitespaces
    iss >> ws;

    // Check if the line is a comment
    if (iss.peek() == '.')
    {
        instr->label = ".";
        instr->opcode = "";
        instr->operands = "";
        return -1;
    }

    int done = 0;
    // Check if the line is END/EXTDEF/EXTREF line
    if (iss.peek() == 'E')
    {
        instr->label = "";
        iss >> instr->opcode >> instr->operands;
        done = 1;
    }

    // Check if the line is LTORG line
    if (iss.peek() == 'L')
    {
        instr->label = "";
        iss >> instr->opcode;
        instr->operands = "";
        done = 1;
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
    string LOCCTR = "";

    // Process based on word count
    if (wordCount == 2 & done == 0)
    {
        // Only opcode is present (RSUB)
        instr->label = "";
        iss >> LOCCTR >> instr->opcode;
        instr->operands = "";
    }
    else if (wordCount == 3)
    {
        // Opcode and operand are present
        instr->label = "";
        iss >> LOCCTR >> instr->opcode >> instr->operands;

        // CSECT and * instructions don't have operands
        if (instr->operands == "CSECT" || instr->opcode == "*")
        {
            instr->label = instr->opcode;
            instr->opcode = instr->operands;
            instr->operands = "";
            if (instr->opcode[0] == '=')
            {
                instr->opcode.erase(0, 1);
            }
        }
    }
    else if (wordCount == 4)
    {
        // Label, opcode, and operand are present
        iss >> LOCCTR >> instr->label >> instr->opcode >> instr->operands;
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

    if (LOCCTR != "")
    {
        return stoi(LOCCTR, nullptr, 16);
    }
    return -1;
}

int main()
{
    // Open intermediate, output and listing files
    ifstream fp1("intermediate.dat");
    ofstream fp2("output.dat");
    ofstream fp3("listing.dat");

    if (!fp1.is_open())
    {
        perror("intermediate.dat");
        exit(1);
    }

    // useful variables' initialisation
    int LOCCTR = 0, BASE = 0, PC = 0, START = 0;
    string CSECT = "", PROGNAME = "", text = "";

    // for record storing
    map<string, vector<string>> text_list, modification_list;
    map<string, string> header_list, end_list, define_list, refer_list;
    vector<string> CSECTS;

    loadOpCodeTable();          // Load opcode table from opTab.dat
    readSymtabFromFile(SYMTAB); // Load symbol table from symTab.dat
    readLittabFromFile(LITTAB); // Load literal table from litTab.dat

    bool flag = false;
    string line;

    // Process each line of the input file
    while (getline(fp1, line))
    {
        // end of file stop reading
        if (line.empty())
            break;

        // processing line into tokens
        Instruction instr;
        LOCCTR = processLine(line, &instr);

        if (instr.label != ".")
        {
            if (instr.opcode == "START")
            {
                // write listing for the instruction
                fp3 << line << endl;

                // csection name
                CSECT = instr.label;
                PROGNAME = CSECT;
                CSECTS.push_back(CSECT);

                // starting address for the csection
                int STADDR = 0;
                if (instr.operands != "")
                    STADDR = stoi(instr.operands, NULL, 16);

                // length of the csection
                int LENGTH = 0;
                if (SYMTAB.find(SYMBOL(CSECT, CSECT)) != SYMTAB.end())
                    LENGTH = SYMTAB[SYMBOL(CSECT, CSECT)].getLength();

                // write the output machine code
                stringstream header;
                header << "H";
                header << formatString(CSECT, 6);
                header << formatNumber(STADDR, 6);
                header << formatNumber(LENGTH, 6);
                header_list.insert({CSECT, header.str()});
            }
            else if (instr.opcode == "END")
            {
                // write listing for the instruction
                fp3 << line << endl;

                // write the output machine code
                stringstream end;
                int first = SYMTAB[SYMBOL(CSECT, instr.operands)].getValue();
                end << "E";
                end_list.insert({CSECT, end.str()});

                end << formatNumber(first, 6);
                end_list[PROGNAME] = end.str();
            }
            else if (instr.opcode == "CSECT")
            {
                // write listing for the instruction
                fp3 << line << endl;

                // write the left over output machine code of the previous CSECT
                if (text.length())
                {
                    int LENGTH = (text.length()) / 2;
                    stringstream text_record;
                    text_record << "T";
                    text_record << formatNumber(START, 6);
                    text_record << formatNumber(LENGTH, 2);
                    text_record << text;

                    text_list[CSECT].push_back(text_record.str());

                    text = "";
                    START = 0;
                }

                stringstream end;
                end << "E";
                end_list.insert({CSECT, end.str()});

                // starting address, length of the new CSECT
                int STADDR = 0;
                CSECT = instr.label;
                CSECTS.push_back(CSECT);

                int LENGTH = 0;
                if (SYMTAB.find(SYMBOL(CSECT, CSECT)) != SYMTAB.end())
                    LENGTH = SYMTAB[SYMBOL(CSECT, CSECT)].getLength();

                // write the output machine code
                stringstream header;
                header << "H";
                header << formatString(CSECT, 6);
                header << formatNumber(STADDR, 6);
                header << formatNumber(LENGTH, 6);
                header_list.insert({CSECT, header.str()});
            }
            else if (instr.opcode == "EXTDEF" || instr.opcode == "EXTREF")
            {
                // write listing for the instruction
                fp3 << line << endl;

                // write the output machine code
                vector<string> operands = instr.getOperands();
                if (instr.opcode == "EXTDEF")
                {
                    stringstream def;
                    def << "D";
                    for (string operand : operands)
                    {
                        def << formatString(operand, 6);
                        int address = 0;
                        if (SYMTAB.find(SYMBOL(CSECT, operand)) != SYMTAB.end())
                            address = SYMTAB[SYMBOL(CSECT, operand)].getValue();
                        def << formatNumber(address, 6);
                    }
                    define_list.insert({CSECT, def.str()});
                }
                else
                {
                    // clear EXTREF list and update according to new CSECT
                    EXTREF.clear();
                    for (auto x : operands)
                        EXTREF.insert(x);

                    stringstream refer;
                    refer << "R";
                    for (string operand : operands)
                        refer << formatString(operand, 6);
                    refer_list.insert({CSECT, refer.str()});
                }
            }
            else if (instr.opcode == "BASE")
            {
                // write listing for the instruction
                fp3 << line << endl;

                // set BASE register location for base relative addressing
                if (SYMTAB.find(SYMBOL(CSECT, instr.operands)) != SYMTAB.end())
                    BASE = SYMTAB[SYMBOL(CSECT, instr.operands)].getValue();
            }
            else if (instr.opcode == "LTORG")
            {
                // write listing for the instruction
                fp3 << line << endl;
            }
            else if (instr.opcode == "EQU")
            {
                // write listing for the instruction
                fp3 << line << endl;
            }
            else
            {
                string obcode = "";

                if (opCodeTable.find(instr.opcode) != opCodeTable.end())
                {
                    // generate op
                    pair<int, string> format = OPTAB(instr.opcode);

                    // extended operation instruction
                    if (instr.e)
                        format.first++;
                    instr.length = format.first;

                    int op = stoi(format.second, 0, 16);
                    instr.formatOpcode(op);

                    // update program counter
                    PC = LOCCTR + instr.length;

                    // generate operandcode
                    int operandcode = 0;
                    if (instr.operands != "")
                    {
                        // format operands
                        if (instr.operands.find(",") != string::npos)
                        {
                            vector<string> m_operands = instr.getOperands();

                            // register-register instructions
                            if (instr.length == 2)
                            {
                                for (string op : m_operands)
                                {
                                    auto it = SYMTAB.find(SYMBOL(CSECT, op));
                                    if (it != SYMTAB.end())
                                    {
                                        int x = SYMTAB[SYMBOL(CSECT, op)].getValue();
                                        operandcode = operandcode << 4;
                                        operandcode += x;
                                    }
                                    else
                                    {
                                        perror("register not found");
                                        exit(1);
                                    }
                                }
                            }
                            // instructions of type BUFFER,X
                            else
                            {
                                int x = 0;
                                if (SYMTAB.find(SYMBOL(CSECT, m_operands.front())) != SYMTAB.end())
                                {
                                    x = SYMTAB[SYMBOL(CSECT, m_operands.front())].getValue();
                                    operandcode += x;
                                    if (instr.p)
                                    {
                                        // decide between PC relative and base relative
                                        if (operandcode - PC >= -2048 && operandcode - PC <= 2047)
                                            operandcode -= PC;
                                        else if (operandcode - BASE >= 0 && operandcode - BASE <= 4095)
                                        {
                                            operandcode -= BASE;
                                            instr.p = 0;
                                            instr.b = 1;
                                        }
                                        else
                                        {
                                            perror("can't fit PC or BASE");
                                            exit(1);
                                        }
                                    }
                                    if (instr.e)
                                    {
                                        stringstream modification;
                                        modification << "M";
                                        modification << formatNumber(LOCCTR + 1, 6);
                                        modification << "05+";
                                        modification << formatString(CSECT, 6);
                                        modification_list[CSECT].push_back(modification.str());
                                    }
                                }
                                else
                                {
                                    operandcode += x;
                                    if (EXTREF.find(m_operands.front()) != EXTREF.end())
                                    {
                                        stringstream modification;
                                        modification << "M";
                                        modification << formatNumber(LOCCTR + 1, 6);
                                        modification << "05+";
                                        modification << formatString(m_operands.front(), 6);
                                        modification_list[CSECT].push_back(modification.str());
                                    }
                                }
                                instr.x = 1;
                            }
                        }
                        else
                        {
                            // if instruction has literal value then get its address
                            if (instr.literal)
                            {
                                if (LITTAB.find(SYMBOL(CSECT, instr.operands)) != LITTAB.end())
                                {
                                    operandcode = LITTAB[SYMBOL(CSECT, instr.operands)].getValue();
                                    // decide between PC relative and BASE relative
                                    if (instr.p)
                                    {
                                        if (operandcode - PC >= -2048 && operandcode - PC <= 2047)
                                            operandcode -= PC;
                                        else if (operandcode - BASE >= 0 && operandcode - BASE <= 4095)
                                        {
                                            operandcode -= BASE;
                                            instr.p = 0;
                                            instr.b = 1;
                                        }
                                        else
                                        {
                                            perror("can't fit PC or BASE");
                                            exit(1);
                                        }
                                    }
                                }
                                else
                                {
                                    // symbol not found in literal table -> that should not happen
                                    perror("my error");
                                    exit(1);
                                }
                            }
                            // if instruction has operand symbol then get its address
                            else if (SYMTAB.find(SYMBOL(CSECT, instr.operands)) != SYMTAB.end())
                            {
                                operandcode = SYMTAB[SYMBOL(CSECT, instr.operands)].getValue();
                                if (instr.length == 2)
                                {
                                    operandcode = operandcode << 4;
                                }
                                else
                                {
                                    if (instr.p)
                                    {
                                        // decide between PC relative and BASE relative
                                        if (operandcode - PC >= -2048 && operandcode - PC <= 2047)
                                            operandcode -= PC;
                                        else if (operandcode - BASE >= 0 && operandcode - BASE <= 4095)
                                        {
                                            operandcode -= BASE;
                                            instr.p = 0;
                                            instr.b = 1;
                                        }
                                        else
                                        {
                                            perror("can't fit PC or BASE");
                                            exit(1);
                                        }
                                    }

                                    if (instr.e)
                                    {
                                        stringstream modification;
                                        modification << "M";
                                        modification << formatNumber(LOCCTR + 1, 6);
                                        modification << "05+";
                                        modification << formatString(CSECT, 6);
                                        modification_list[CSECT].push_back(modification.str());
                                    }
                                }
                            }
                            // if operand is immediate or external reference
                            else
                            {
                                if (EXTREF.find(instr.operands) != EXTREF.end())
                                {
                                    stringstream modification;
                                    modification << "M";
                                    modification << formatNumber(LOCCTR + 1, 6);
                                    modification << "05+";
                                    modification << formatString(instr.operands, 6);
                                    modification_list[CSECT].push_back(modification.str());
                                }
                                if (instr.i)
                                {
                                    if (isNumber(instr.operands))
                                    {
                                        operandcode = stoi(instr.operands);
                                    }
                                    else
                                        ;
                                }
                            }
                        }
                        instr.formatOperand(operandcode);
                    }

                    // writing listing for the instruction
                    obcode = formatNumber(op, 2) + formatNumber(operandcode, 2 * (instr.length - 1));
                    fp3 << line << "\t" << formatString(obcode, 10) << endl;
                }
                else
                {
                    // process constants
                    if (instr.label == "*")
                    {
                        string constant = instr.opcode.substr(2, instr.opcode.length() - 3);
                        if (instr.opcode.front() == 'X')
                        {
                            int x = stoi(constant, nullptr, 16);
                            obcode = formatNumber(x, constant.length());
                        }
                        else if (instr.opcode.front() == 'C')
                        {
                            for (char ch : constant)
                            {
                                int x = ch;
                                obcode += formatNumber(x, 2);
                            }
                        }
                        else
                        {
                            int word = stoi(instr.operands);
                            obcode = formatNumber(word, 6);
                        }
                    }

                    if (instr.opcode == "WORD")
                    {
                        if (isNumber(instr.operands))
                        {
                            int word = stoi(instr.operands);
                            obcode = formatNumber(word, 6);
                        }
                        else
                        {
                            evalExpression e(CSECT);
                            int word = e.evaluate(instr.operands).second;
                            applyMask(word, 24);
                            obcode = formatNumber(word, 6);

                            vector<string> m_operands = e.tokenize(instr.operands);
                            for (int i = 0; i < (int)(m_operands.size()); i++)
                            {
                                if (EXTREF.find(m_operands[i]) != EXTREF.end())
                                {
                                    stringstream modification;
                                    modification << "M";
                                    modification << formatNumber(LOCCTR, 6);
                                    modification << "06";
                                    if (i > 0 && m_operands[i - 1] == "-")
                                        modification << "-";
                                    else
                                        modification << "+";
                                    modification << formatString(m_operands[i], 6);
                                    modification_list[CSECT].push_back(modification.str());
                                }
                            }
                        }
                    }

                    if (instr.opcode == "BYTE")
                    {
                        string constant = instr.operands.substr(2, instr.operands.length() - 3);
                        if (instr.operands.front() == 'X')
                        {
                            int x = stoi(constant, nullptr, 16);
                            obcode = formatNumber(x, constant.length());
                        }

                        if (instr.operands.front() == 'C')
                        {
                            for (char ch : constant)
                            {
                                int x = ch;
                                obcode += formatNumber(x, 2);
                            }
                        }
                    }
                    // writing listing for the instruction
                    fp3 << line << "\t" << formatString(obcode, 10) << endl;
                }

                // write the output machine code
                if (text.length() == 0)
                    START = LOCCTR;

                if (text.length() + obcode.length() <= 60 && instr.opcode != "RESW" && instr.opcode != "RESB")
                    text += obcode;
                else
                {
                    if (text.length())
                    {
                        int LENGTH = (text.length()) / 2;
                        stringstream text_record;
                        text_record << "T";
                        text_record << formatNumber(START, 6);
                        text_record << formatNumber(LENGTH, 2);
                        text_record << text;

                        text_list[CSECT].push_back(text_record.str());

                        text = obcode;
                        START = LOCCTR;
                    }
                }
            }
        }
        else
            fp3 << line << endl;
    }

    if (text.length())
    {
        int LENGTH = (text.length()) / 2;
        stringstream text_record;
        text_record << "T";
        text_record << formatNumber(START, 6);
        text_record << formatNumber(LENGTH, 2);
        text_record << text;

        text_list[CSECT].push_back(text_record.str());

        text = "";
        START = 0;
    }

    for (string CSECT : CSECTS)
    {
        fp2 << header_list[CSECT] << endl;
        if (!define_list[CSECT].empty())
            fp2 << define_list[CSECT] << endl;
        if (!refer_list[CSECT].empty())
            fp2 << refer_list[CSECT] << endl;
        for (string text_record : text_list[CSECT])
            fp2 << text_record << endl;
        for (string modification_record : modification_list[CSECT])
            fp2 << modification_record << endl;
        fp2 << end_list[CSECT] << endl;
    }

    // closing files
    fp1.close();
    fp2.close();
    fp3.close();

    return 0;
}