#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <cctype>
#include "i8080InstructionData.h"

//A struct that holds our output file and all information related to that file
struct AssembledFile{
//the input files contents
std::vector<std::string> lines;
//the binary output
std::vector<uint8_t> output;
//the current line were on in output
size_t lineNum=0;
//what pass were currently on, this assembler will preform 2 passes.
int currentPass = 1;
//addresses of our labels.
std::unordered_map<std::string,uint16_t> symbolTable;
//constructor


AssembledFile(std::vector<std::string> Infile){
    this->lines = Infile;
}

};

struct ParsedLine{
//some variables for managing the current line were parsing
std::string label,opcode,argument1,argument2;
uint8_t instructionSize;

ParsedLine(std::string lab, std::string op, std::string a1, std::string a2,uint8_t Is){
    this->label = lab;
    this->opcode = op;
    this->argument1 = a1;
    this->argument2 = a2;
    this->instructionSize = Is;
}
};

void err(std::string msg,int lineNumber){
    std::cout << "error on line " << lineNumber << ": "+msg<<std::endl;
    exit(1);
}

void to_upper(std::string& str){

    for (char& c : str) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

}

void ltrim(std::string& s) {
    auto p = s.find_first_not_of(" \t");
    if (p == std::string::npos) { s.clear(); return; }
    s.erase(0, p);
}

void trim(std::string& s) {
    auto start = s.find_first_not_of(" \t");
    if (start == std::string::npos) { s.clear(); return; }
    auto end = s.find_last_not_of(" \t");
    s = s.substr(start, end - start + 1);
}

void check_and_add_symbol_reference(const ParsedLine& currentLine,uint16_t pc,AssembledFile& workingFile){
    //if their is no LABEL, then there is no symbol to record, and we can skip this line in pass 1.
    if(currentLine.label.empty()){
        return;
    }
    //if there is not currently an entry for this label, then this should be symbolTable.end().
    auto symbolEntry = workingFile.symbolTable.find(currentLine.label);
    //if the symbol already exists, thats an error and we should stop assembly.
    if(symbolEntry != workingFile.symbolTable.end()){
        err("Error, label "+currentLine.label+" is already present in symbol table.",workingFile.lineNum);
    }
    workingFile.symbolTable.insert({currentLine.label,pc});
}

size_t find_delim_pos(const std::string& s, char delim) {
    return s.find(delim);  // returns std::string::npos if not found
}

uint8_t get_instruction_size(const std::string& instruction){

    auto instructionEntry = i8080Instructions.find(instruction);
    if(instructionEntry == i8080Instructions.end()){
        return 0;
    }
    return instructionEntry -> second;

}

ParsedLine parse(std::string line,AssembledFile& workingFile){
    // Strip comment first
    auto commentPos = line.find(';');
    if (commentPos != std::string::npos) {
        line.erase(commentPos);
    }

    std::string label,op,a1,a2;
    uint8_t Size;
    // LABEL
    size_t pos = line.find(':');
    if (pos != std::string::npos) {
        label = line.substr(0, pos);
        trim(label);
        line.erase(0, pos + 1);
    }

    // OP
    ltrim(line);
    pos = line.find_first_of(" \t");;
    if (pos != std::string::npos) {
        op = line.substr(0, pos);
        trim(op);
        line.erase(0, pos + 1);
    } else {
        op = line;
        trim(op);
    }
    to_upper(op);
    // ARG1
    ltrim(line);
    pos = line.find(',');
    if (pos != std::string::npos) {
        a1 = line.substr(0, pos);
        trim(a1);
        line.erase(0, pos + 1);
    } else {
        a1 = line;
        trim(a1);
    }

    // ARG2 / COMMENT
    //at this point, line will just be a2 as we stripped everything else.
    a2 = line;
    trim(a2);

    //FINDING SIZE
    //check to see if the line is just a label
    if(op.empty()){
        Size = 0;
       }
    //otherwise, the size depends on the opcode
    else{
        Size = get_instruction_size(op);
        //get_instruction_size returns 0 if the opcode is not found in the size table
        if(Size == 0){
            err("Error: opcode not found in size table during pass 1",workingFile.lineNum);
        }
    }
    return ParsedLine(label,op,a1,a2,Size);
}


uint8_t build_opcode(ParsedLine& currentLine,AssembledFile& workingFile){
    auto it = i8080Instructions.find(currentLine.opcode);
    //if the current lines opcode is not recognized, then the user input an unrecognized mnemonic and we should exit.
    if(it == i8080Instructions.end()){
        err("Error, unrecognized mnemonic.",workingFile.lineNum);
    }
    const std::string& bitPattern = it -> second.bitPattern;
    uint8_t opcode = 0;
    for(char c : bitPattern){
        switch(c){
            case '1': opcode <<= 1;opcode |= 1;break;
            case '0': opcode <<= 1;break;
            case 'D':{
                //if no a1, error out
                if(currentLine.argument1.empty()){
                    err("Error, mnemonic ["+currentLine.opcode+"] expects an argument.",workingFile.lineNum);
                }
                auto reg = regCode.find(currentLine.argument1);
                //if a1 didnt exist in regCodes, error out
                if(reg == regCode.end()){
                    err("Error, reg code ["+currentLine.argument1+"] is not recognized",workingFile.lineNum);
                }
                //shift out of the DDD section of the bit pattern.
                opcode <<= 3;
                //mask regCode into our opcode
                opcode |= static_cast<uint8_t>(reg->second)&0b111;
                //clear the argument so future steps dont flag this as an argument that should not exist.
                currentLine.argument1.clear()
                break;}
            case 'S':{
                    //if no a2, error out
                    if(currentLine.argument2.empty()){
                        err("Error, mnemonic ["+currentLine.opcode+"] expects a second argument.",workingFile.lineNum);
                    }
                    auto reg = regCode.find(currentLine.argument2);
                    //if a2 didnt exist in regCodes, error out
                    if(reg == regCode.end()){
                        err("Error, reg code ["+currentLine.argument2+"] is not recognized",workingFile.lineNum);
                    }
                    //shift out of the SSS section of the bit pattern.
                    opcode <<= 3;
                    //mask regCode into our opcode
                    opcode |= static_cast<uint8_t>(reg->second)&0b111;
                    currentLine.argument2.clear()
                    break;}
            case 'R':{
                     //if no a1, error out
                    if(currentLine.argument1.empty()){
                        err("Error, mnemonic ["+currentLine.opcode+"] expects an argument.",workingFile.lineNum);
                    }
                    auto regp = rpCode.find(currentLine.argument1);
                    //if a1 didnt exist in regCodes, error out
                    if(regp == rpCode.end()){
                        err("Error, rp code ["+currentLine.argument1+"] is not recognized",workingFile.lineNum);
                    }
                    //shift out of the RP section of the bit pattern.
                    opcode <<= 2;
                    //mask regCode into our opcode
                    opcode |= static_cast<uint8_t>(regp->second)& 0b11;
                    currentLine.argument1.clear()
                    break;}
            case 'N':{
                    //if no a1, error out
                    if(currentLine.argument1.empty()){
                        err("Error, mnemonic ["+currentLine.opcode+"] expects an argument.",workingFile.lineNum);
                    }
                    int n;
                    size_t pos = 0;
                    try {
                        n = std::stoi(currentLine.argument1, &pos, 10);
                    } catch (const std::exception&) {
                        err("Error, RST expects a number 0–7.", workingFile.lineNum);
                    }

                    if (pos != currentLine.argument1.size() || n < 0 || n > 7) {
                        err("Error, RST expects a number 0–7.", workingFile.lineNum);
                    }
                    opcode <<= 3;
                    opcode |= static_cast<uint8_t>(n)&0b111;
                    currentLine.argument1.clear()
                    break;
                    }
            }
        }
    return opcode;
}

void assemble(AssembledFile& workingFile){
    //PASS 1: Linking labels
    uint16_t PC = 0;
    for(; workingFile.lineNum < workingFile.lines.size();workingFile.lineNum++){
        ParsedLine currentLineInfo = parse(workingFile.lines[workingFile.lineNum],workingFile);
        check_and_add_symbol_reference(currentLineInfo,PC,workingFile);
        PC += currentLineInfo.instructionSize;
    }
    //Reset lineNum and PC
    workingFile.lineNum = 0;
    PC = 0;
    //PASS 2: Build the output file
    for(; workingFile.lineNum < workingFile.lines.size(); workingFile.lineNum++){
        ParsedLine currentLineInfo = parse(workingFile.lines[workingFile.lineNum]);
        uint8_t opcode = build_opcode(currentLineInfo,workingFile);
        if(currentLineInfo.instructionSize == 1){
            //if the size is 1, but we have arguments that were not used up in building our opcode, we error out.
            if(!currentLineInfo.argument1.empty() || !currentLineInfo.argument2.empty()){
                err("Error, mnemonic ["+currentLineInfo.opcode+"] expects no arguments but was passed 1",workingFile.lineNum);
            }
            workingFile.lines.push_back(opcode);
        }


    }

}

int main(int argc, char* argv[]){
    //validate that we were given a file name as a command line argument
    if(argc < 2){
        std::cout << "Provide A file name";
        return 1;
    }

    //open the file
    std::string FilePath = argv[1];
    std::ifstream infile(FilePath);
    //if the file could not be opened, print an error
    if(infile.good() == false){
        std::cout << FilePath << " could not be opened." << std::endl;
        //exit the program
        return 1;
        }

    //we want to split the contents of the file into an array of lines
    //rather than writing my own dynamically resizing array, I will just use a vector.
    std::vector<std::string> lines;
    std::string currentLine;
    while(std::getline(infile,currentLine)){
        lines.push_back(currentLine);
    }
    AssembledFile currentFile(lines);
    assemble(currentFile);
}
