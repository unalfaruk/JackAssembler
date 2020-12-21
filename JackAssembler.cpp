// JackAssembler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <ctype.h>
#include <unordered_map>
using namespace std;

#define A_COMMAND 0
#define C_COMMAND 1
#define L_COMMAND 2

class Parser;
class Code;
class SymbolTable;


class Parser {
public:
    fstream ASMtoParse;
    string currentLine = "";

    Parser(string asmFilePath) {
            this->ASMtoParse.open(asmFilePath);
    }

    Parser() {
            this->ASMtoParse.open("~tmpASMTranslated.asm");
    }

    ~Parser() {
        this->ASMtoParse.close();
    }

    bool hasMoreCommands() {
        getline(this->ASMtoParse, this->currentLine);
        if (this->currentLine == "" || this->currentLine.empty())
            return false;
        return true;
    }

    void advance() {
        //#Nothing...
    }

    int commandType() {
        if (this->currentLine[0] == '@')
            return A_COMMAND;

        if (this->currentLine[0] == '(')
            return L_COMMAND;

        return C_COMMAND;
    }

    string symbol() {
        if (this->commandType() == A_COMMAND)
            return this->currentLine.substr(1, this->currentLine.size() - 1);
        
        if (this->commandType() == L_COMMAND)
            return this->currentLine.substr(1, this->currentLine.size() - 2);
    }

    string dest() {
        if (this->commandType() == C_COMMAND) {
            int indexOfEq = this->currentLine.find("=");
            if (indexOfEq != string::npos)
                return this->currentLine.substr(0, indexOfEq);
        }
    }

    string comp() {
        if (this->commandType() == C_COMMAND) {
            int indexOfEq = this->currentLine.find("=");
            int indexOfSc = this->currentLine.find(";");

            //Ex -> D;JGT
            if (indexOfEq == string::npos && indexOfSc != string::npos)
                return this->currentLine.substr(0, indexOfSc);

            //Ex -> D=D-M
            if (indexOfSc != string::npos)
                return this->currentLine.substr(indexOfEq+1, indexOfSc-1);
            else
                return this->currentLine.substr(indexOfEq + 1, this->currentLine.size() - 1);
        }
    }    

    string jump() {
        if (this->commandType() == C_COMMAND) {
            int indexOfSc = this->currentLine.find(";");

            if (indexOfSc != string::npos)
                return this->currentLine.substr(indexOfSc + 1, this->currentLine.size() - 1);
            return "000"; //if no jump
        }
    }

};

class Code {
public:
    static string dest(string destASM) {
        string destBIN = "000";

        if (destASM.find("A") != string::npos)
            destBIN[0] = '1';
        if (destASM.find("D") != string::npos)
            destBIN[1] = '1';
        if (destASM.find("M") != string::npos)
            destBIN[2] = '1';
        
        return destBIN;
    }

    static string comp(string compASM) {
        string compBIN = "0000000"; //7 bit: 1 bit a + 6 bit comp

        //Set a bit by D or M
        if (compASM.find("D") != string::npos)
            compBIN[0] = '0';
        if (compASM.find("M") != string::npos)
            compBIN[0] = '1';

        if (compASM == "0")
            compBIN.replace(1, 6, "101010");
        if (compASM == "1")
            compBIN.replace(1, 6, "111111");
        if (compASM == "-1")
            compBIN.replace(1, 6, "111010");
        if (compASM == "D")
            compBIN.replace(1, 6, "001100");
        if (compASM == "A" || compASM == "M")
            compBIN.replace(1, 6, "110000");
        if (compASM == "!D")
            compBIN.replace(1, 6, "001101");
        if (compASM == "!A" || compASM == "!M")
            compBIN.replace(1, 6, "110001");
        if (compASM == "-D")
            compBIN.replace(1, 6, "001111");
        if (compASM == "-A" || compASM == "-M")
            compBIN.replace(1, 6, "110011");
        if (compASM == "D+1")
            compBIN.replace(1, 6, "011111");
        if (compASM == "A+1" || compASM == "M+1")
            compBIN.replace(1, 6, "110111");
        if (compASM == "D-1")
            compBIN.replace(1, 6, "001110");
        if (compASM == "A-1" || compASM == "M-1")
            compBIN.replace(1, 6, "110010");
        if (compASM == "D+A" || compASM == "D+M")
            compBIN.replace(1, 6, "000010");
        if (compASM == "D-A" || compASM == "D-M")
            compBIN.replace(1, 6, "010011");
        if (compASM == "A-D" || compASM == "M-D")
            compBIN.replace(1, 6, "000111");
        if (compASM == "D&A" || compASM == "D&M")
            compBIN.replace(1, 6, "000000");
        if (compASM == "D|A" || compASM == "D|M")
            compBIN.replace(1, 6, "010101");

        return compBIN;
    }

    static string jump(string jumpASM) {
        string jumpBIN = "000";

        if (jumpASM.find("E") != string::npos)
            jumpBIN[1] = '1';

        if (jumpASM.find("N") != string::npos) {
            //strcpy(jumpBIN, "101");
            jumpBIN[0] = '1';
            jumpBIN[1] = '0';
            jumpBIN[2] = '1';
        }

        if (jumpASM.find("G") != string::npos)
            jumpBIN[2] = '1';
        if (jumpASM.find("L") != string::npos)
            jumpBIN[0] = '1';

        if (jumpASM.find("M") != string::npos) {
            //strcpy(jumpBIN, "111");
            jumpBIN[0] = '1';
            jumpBIN[1] = '1';
            jumpBIN[2] = '1';
        }
        return jumpBIN;
    }
};

class SymbolTable {
public:
    unordered_map<string, int> table;
    int nextFreeAddr = 16;
    SymbolTable() {
        this->table.clear();
        this->table["SP"] = 0;
        this->table["LCL"] = 1;
        this->table["ARG"] = 2;
        this->table["THIS"] = 3;
        this->table["THAT"] = 4;
        this->table["R0"] = 0;
        this->table["R1"] = 1;
        this->table["R2"] = 2;
        this->table["R3"] = 3;
        this->table["R4"] = 4;
        this->table["R5"] = 5;
        this->table["R6"] = 6;
        this->table["R7"] = 7;
        this->table["R8"] = 8;
        this->table["R9"] = 9;
        this->table["R10"] = 10;
        this->table["R11"] = 11;
        this->table["R12"] = 12;
        this->table["R13"] = 13;
        this->table["R14"] = 14;
        this->table["R15"] = 15;
        this->table["SCREEN"] = 16384;
        this->table["KBD"] = 24576;
    }

    void addEntry(string symbol, int addr) {
        this->table[symbol] = addr;
    }

    bool contains(string symbol) {
        return this->table.count(symbol);
    }

    int GetAddress(string symbol) {
        return this->table[symbol];
    }
};

int main(int argc, char* argv[])
{
    if (strcmp(argv[1], "--help") == 0) {
        cout << "The .asm file must be located in the same directory with the assembler..." << endl;
        cout << "Usage: " << argv[0] << " example.asm" << endl;
        return 0;
    }

    //File extension checker needed... for the scenario below:
    //  assembler.exe test.ext
    if (argc != 2) {
        cerr << "USAGE: " << argv[0] << " example.asm" << endl;
        return 0;
    }

    cout << "Welcome to Jack Assembler!" << endl;

    fstream asmFile;
    asmFile.open(argv[1]);
    if (!asmFile.is_open()) {
        cout << "Error! ASM file("<<argv[1]<<") can not be opened!" << endl;
        return 0;
    }    

    cout << "ASM file("<<argv[1]<<") is being initialized...\n" << endl;
    cout << "ASM file will be organized...\n\tComment lines & empty lines will be removed..." << endl;

    string line;
    int lineCounter = 0;

    ofstream tmpASM;
    tmpASM.open("~tmpASM.asm");
    if (!tmpASM.is_open()) {
        cout << "Error! Temporary ASM file can not be created!" << endl;
        return 0;
    }

    //cout << "\t--- ASM ---\n" << endl;
    while (getline(asmFile, line)) {
        //Ignore comment lines and empty lines
        if (line.substr(0, 2) == "//" || line == "")
            continue;

        if (line.find("//")!=string::npos)
            line.replace(line.find("//"), line.length() - line.find("//"), "");


        //cout << lineCounter << ". ";
        //cout << line << endl;
        lineCounter++;

        tmpASM << line << endl;
    }
    tmpASM.close();
    asmFile.close();

    cout << "\tASM file organized successfully!\n" << endl;

    //First pass part for symbol table
    fstream tmpASMFirstPass;
    tmpASMFirstPass.open("~tmpASM.asm");
    SymbolTable symTable;
    if (!tmpASMFirstPass.is_open()) {
        cout << "Error! Temporary ASM file can not be opened!" << endl;
        return 0;
    }
    
    cout << "FIRST PASS: Find labels to add them into the symbol table." << endl;
    lineCounter = 0;
    while (getline(tmpASMFirstPass, line)) {        
        //Find (xxx) type lines, (This line type is for labels)
        if (line[0]=='(') {
            int lenLine = line.length();
            string label = line.substr(1, lenLine-2);
            if (!symTable.contains(label))
                //Don't add +1 to the address
                //Becase you will remove this line
                //After the removing, addr+1 will be addr
                symTable.addEntry(label,lineCounter);
            continue;
        }            

        //cout << lineCounter << ". ";
        //cout << line << endl;
        lineCounter++;
    }
    tmpASMFirstPass.close();
    cout << "\tFirst pass completed successfully!\n" << endl;

    //Second pass parts for translation
    fstream tmpASMSecondPass;
    tmpASMSecondPass.open("~tmpASM.asm");
    if (!tmpASMSecondPass.is_open()) {
        cout << "Error! Temporary ASM file can not be opened!" << endl;
        return 0;
    }

    ofstream tmpASMTranslated;
    tmpASMTranslated.open("~tmpASMTranslated.asm");
    if (!tmpASMTranslated.is_open()) {
        cout << "Error! Temporary ASM translated file can not be created!" << endl;
        return 0;
    }

    cout << "SECOND PASS: Label translation into the address and addressing for runtime variables." << endl;
    lineCounter = 0;
    while (getline(tmpASMSecondPass, line)) {
        line.erase(remove_if(line.begin(), line.end(), isblank), line.end()); //Remove blanks if exists at the end of lines...
        if (line[0] == '@' && !isdigit(line[1])) {
            int lenLine = line.length();
            string label = line.substr(1, lenLine - 1);
            if (!symTable.contains(label)) { 
                //For runtime variables... Ex: ponggame.0
                symTable.addEntry(label, symTable.nextFreeAddr++);
                line.replace(1, label.length(), to_string(symTable.GetAddress(label)));
            }                
            else
                line.replace(1,label.length(),to_string(symTable.GetAddress(label)));
        }        

        //cout << lineCounter << ". ";
        //cout << line << endl;
        line.erase(remove_if(line.begin(), line.end(), isblank), line.end()); //Remove blanks
        if(line[0]!='(')
            tmpASMTranslated << line << endl;

        lineCounter++;
    }
    tmpASMSecondPass.close();
    cout << "\tSecond pass completed successfully!\n" << endl;

    Parser parser;
    ofstream hackFile;

    //Generate the name of hack file
    //Remove ".asm" and add ".hack"
    string inputFile = argv[1];
    string outputFileName = inputFile.substr(0,inputFile.length()-4)+".hack";
     

    hackFile.open(outputFileName);
    //cout << "\n\t--- BINARY ---\n" << endl;
    cout << "Binary conversion beginning..." << endl;
    while(parser.hasMoreCommands()) {
        string outLineBIN="";
        parser.advance();
        int cmdType = parser.commandType();

        if (cmdType == C_COMMAND) {
            string c = parser.comp();
            string d = parser.dest();
            string j = parser.jump();

            string cc = Code::comp(c);
            string dd = Code::dest(d);
            string jj = Code::jump(j);

            outLineBIN = "111" + cc + dd + jj;
        }

        if (cmdType == A_COMMAND) {
            outLineBIN = "0" + bitset<15>(stoi(parser.symbol())).to_string();
        }
        //cout << outLineBIN <<endl;
        hackFile << outLineBIN << endl;
    }
    hackFile.close();

    //Remove temporary files...
    std::remove("~tmpASMTranslated.asm");
    std::remove("~tmpASM.asm");

    cout << "\tBinary conversion completed succesfully!" << endl;

    string outputPath = argv[0];
    cout << "\tThe path of hack file: "<< outputPath.substr(0, outputPath.find_last_of('\\')) << "\\" << outputFileName << endl;

    return 0;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
