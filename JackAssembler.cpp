// JackAssembler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
using namespace std;

#define A_COMMAND 0
#define C_COMMAND 1
#define L_COMMAND 2

class Parser;
class Code;



class Parser {
public:
    fstream ASMtoParse;
    string currentLine = "";

    Parser(string asmFilePath) {
            this->ASMtoParse.open(asmFilePath);
    }

    Parser() {
            this->ASMtoParse.open("~tmpASM.asm");
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
        if (this->currentLine.substr(0, 1) == "@")
            return A_COMMAND;

        if (this->currentLine.substr(0, 1) == "(")
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
            return this->currentLine.substr(0, indexOfEq);
        }
    }

    string comp() {
        if (this->commandType() == C_COMMAND) {
            int indexOfEq = this->currentLine.find("=");
            int indexOfSc = this->currentLine.find(";");

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
            compBIN.replace(1, 6, "111010");
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

int main()
{
    cout << "Welcome to Jack Assembler!" << endl;

    fstream asmFile;
    asmFile.open("Add.asm");
    if (!asmFile.is_open()) {
        cout << "Error! ASM file can not be opened!" << endl;
        return 0;
    }

    string line;

    cout << "ASM file is being initialized..." << endl;
    cout << "ASM file will be printed without comment lines & empty lines...\n" << endl;

    int lineCounter = 0;

    ofstream tmpASM;
    tmpASM.open("~tmpASM.asm");
    if (!tmpASM.is_open()) {
        cout << "Error! Temporary ASM file can not be created!" << endl;
        return 0;
    }
    cout << "\t--- ASM ---\n" << endl;
    while (getline(asmFile, line)) {
        //Ignore comment lines and empty lines
        if (line.substr(0, 2) == "//" || line == "")
            continue;

        cout << lineCounter << ". ";
        cout << line << endl;
        lineCounter++;

        tmpASM << line << endl;
    }
    tmpASM.close();
    asmFile.close();

    Parser parser;
    cout << "\n\t--- BINARY ---\n" << endl;
    while(parser.hasMoreCommands()) {
        string outLineBIN;
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
        cout << outLineBIN <<endl;

    }


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
