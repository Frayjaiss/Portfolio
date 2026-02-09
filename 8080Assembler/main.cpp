#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <cctype>
#include <type_traits>
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
int operandCount = 0;

ParsedLine(std::string lab, std::string op, std::string a1, std::string a2,uint8_t Is){
    this->label = lab;
    this->opcode = op;
    this->argument1 = a1;
    this->argument2 = a2;
    this->instructionSize = Is;
}

void print(){
    std::cout << this -> opcode << this -> argument1 << this -> argument2;
}

void set_operand_count(){
    if(!argument1.empty()){
        this -> operandCount += 1;
    }
    if(!argument2.empty()){
        this -> operandCount += 1;
    }
}

};

template <typename T>
T parse_uint(const std::string& input) {
    static_assert(
        std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value,
        "parseUint only supports uint8_t or uint16_t"
    );

    //Trim leading and trailing whitespace
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };

    size_t startIndex = 0;
    while (startIndex < input.size() && isSpace(input[startIndex])) {
        startIndex++;
    }

    size_t endIndex = input.size();
    while (endIndex > startIndex && isSpace(input[endIndex - 1])) {
        endIndex--;
    }

    std::string trimmedInput = input.substr(startIndex, endIndex - startIndex);

    //Determine numeric base
    int numericBase = 10;

    // Binary: 0b1010 or 1010b
    if (trimmedInput.size() >= 2 &&
        (trimmedInput.rfind("0b", 0) == 0 || trimmedInput.rfind("0B", 0) == 0)) {
        numericBase = 2;
        trimmedInput.erase(0, 2);
    }
    else if (!trimmedInput.empty() &&
             (trimmedInput.back() == 'b' || trimmedInput.back() == 'B')) {
        numericBase = 2;
        trimmedInput.pop_back();
    }
    // Hexadecimal: 0x1A2F or 1A2Fh
    else if (trimmedInput.size() >= 2 &&
             (trimmedInput.rfind("0x", 0) == 0 || trimmedInput.rfind("0X", 0) == 0)) {
        numericBase = 16;
        trimmedInput.erase(0, 2);
    }
    else if (!trimmedInput.empty() &&
             (trimmedInput.back() == 'h' || trimmedInput.back() == 'H')) {
        numericBase = 16;
        trimmedInput.pop_back();
    }

    //Convert and truncate to target width
    unsigned long parsedValue = std::stoul(trimmedInput, nullptr, numericBase);
    return static_cast<T>(parsedValue);
}

void err(std::string msg,int lineNumber){
    std::cout << "error on line " << lineNumber+1 << ": "+msg<<std::endl;
    exit(1);
}

bool check_valid_byte(const std::string& number, int byteSize) {
    std::string s = number;
    int base = 10;

    // Detect hex suffix (Intel style)
    if (!s.empty() && (s.back() == 'h' || s.back() == 'H')) {
        base = 16;
        s.pop_back();
        if (s.empty()) return false;
    }
    // Detect hex prefix (C style)
    else if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s = s.substr(2);
        if (s.empty()) return false;
    }

    int n;
    size_t pos = 0;

    try {
        n = std::stoi(s, &pos, base);
    } catch (const std::exception&) {
        return false;
    }

    // Reject partial parses (e.g. "12G", "5abc")
    if (pos != s.size()) {
        return false;
    }

    // Range check
    int maxValue = (1 << (byteSize * 8)) - 1;
    if (n < 0 || n > maxValue) {
        return false;
    }

    return true;
}

std::vector<uint8_t> to_little_endian(std::string value,AssembledFile& workingFile){
    uint8_t high_byte;
    uint8_t low_byte;
    if(!check_valid_byte(value,2)){
        err("value ["+value+"] is not a valid 16bit byte and could not be converted to little endian.",workingFile.lineNum);
    }
    uint16_t byte = parse_uint<uint16_t>(value);
    // Extract the high byte (most significant)
    // Shift the 16-bit value right by 8 bits (0x1234 -> 0x0012)
    high_byte = static_cast<uint8_t>(byte >> 8);

    // Extract the low byte (least significant)
    // Use AND mask to get only the rightmost 8 bits (0x1234 & 0x00FF -> 0x0034)
    low_byte = static_cast<uint8_t>(byte & 0xFF);

    std::vector<uint8_t> littleEndian = {low_byte,high_byte};
    return littleEndian;
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
    return instructionEntry -> second.size;

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
        line.clear();
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
        line.clear();
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
        err("Error, ["+currentLine.opcode+"] is not a recognized mnemonic.",workingFile.lineNum);
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
                currentLine.argument1.clear();
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
                    currentLine.argument2.clear();
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
                    currentLine.argument1.clear();
                    break;}
            case 'N':{
                    //if no a1, error out
                    if(currentLine.argument1.empty()){
                        err("Error, mnemonic ["+currentLine.opcode+"] expects an argument.",workingFile.lineNum);
                    }
                    bool valid_8_bit = check_valid_byte(currentLine.argument1,1);
                    if(!valid_8_bit){
                        err("Error, RST expects a number 0–7.", workingFile.lineNum);
                    }
                    //its safe to cast this to integer because check_valid_byte already checks to make sure the argument is a valid number
                    if(std::stoi(currentLine.argument1) > 7 || std::stoi(currentLine.argument1) < 0){
                        err("Error, RST expects a number 0 to 7.", workingFile.lineNum);
                    }
                    opcode <<= 3;
                    opcode |= static_cast<uint8_t>(std::stoi(currentLine.argument1))&0b111;
                    currentLine.argument1.clear();
                    break;
                    }
            }
        }
    return opcode;
}

//this function will return a vector of 8-bit bytes that can be added to the output vector in the order they are returned
std::vector<uint8_t> build_operand(ParsedLine& currentLine,AssembledFile& workingFile){
    //operands are in little endian notation
    std::vector<uint8_t> operandVector;
    if(!currentLine.argument1.empty()){
        //if we find argument1 in our symbol table, then that is the value we want to return
        auto symbolLookup = workingFile.symbolTable.find(currentLine.argument1);
        if(symbolLookup != workingFile.symbolTable.end()){
            operandVector = to_little_endian(std::to_string(symbolLookup -> second),workingFile);
        }
        //if it is not in the symbol table
        else{
            //if its a valid 8bit number, then we add it to our operand vector
            if(check_valid_byte(currentLine.argument1,1)){
                uint8_t parsedByte = parse_uint<uint8_t>(currentLine.argument1);
                operandVector.push_back(parsedByte);
            }
            //if its a valid 16-bit number then we do exactly what we did with a symbol address.
            else if(check_valid_byte(currentLine.argument1,2)){
                operandVector = to_little_endian(currentLine.argument1,workingFile);
            }
            else{
                err("Unrecognized symbol ["+currentLine.argument1+"]",workingFile.lineNum);
            }
        }
    }
    return operandVector;
}

void assemble(AssembledFile& workingFile){
    //PASS 1: Linking labels
    uint16_t PC = 0;
    std::vector<ParsedLine> allLines;
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
        ParsedLine currentLineInfo = parse(workingFile.lines[workingFile.lineNum],workingFile);
        if(currentLineInfo.opcode.empty()){
            std::cout << "Warning on line " + std::to_string(workingFile.lineNum+1) + ", no opcode" << std::endl;
            continue;
        }
        uint8_t opcode = build_opcode(currentLineInfo,workingFile);
        //now that we have built the opcode and consumed any arguments that process used, we can count how many operands the user gave the opcode.
        currentLineInfo.set_operand_count();
        //check to make sure the user sent the appropriate amount of operands for this opcode
        //if the size is 1, we should expect no operands left after building the opcode
        if(currentLineInfo.instructionSize == 1 && currentLineInfo.operandCount != 0){
            err("Error, mnemonic ["+currentLineInfo.opcode+"] expects 0 operands but was passed "+std::to_string(currentLineInfo.operandCount),workingFile.lineNum);
        }
        //if the size is greater than 1, we should expect 1 operand left after building the opcode
        else if(currentLineInfo.instructionSize > 1 && currentLineInfo.operandCount != 1){
            err("Error, mnemonic ["+currentLineInfo.opcode+"] expects 1 operands but was passed "+std::to_string(currentLineInfo.operandCount),workingFile.lineNum);
        }
        //at this point, we should only have 1 operand that we need to translate. Were going to move it to argument1 if its not there already
        if(currentLineInfo.argument1.empty()){
            currentLineInfo.argument1 = currentLineInfo.argument2;
        }
        //get the bytes for the operands
        std::vector<uint8_t> operandBytes = build_operand(currentLineInfo,workingFile);
        //check for mismatch in operand types (expecting 8 bit or 16 bit)
        if(currentLineInfo.instructionSize-1 != operandBytes.size()){
            err("Error, mismatch in operand type and opcode ["+currentLineInfo.opcode+"] expectation",workingFile.lineNum);
        }
        //add the bytes into the output file
        workingFile.output.push_back(opcode);
        workingFile.output.insert(workingFile.output.end(),operandBytes.begin(),operandBytes.end());
    }

}

void printBits(uint8_t value) {
    for (int i = 7; i >= 0; --i) {
        std::cout << ((value >> i) & 1);
    }
    std::cout << '\n';
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
    for(int i = 0; i < currentFile.output.size(); i++){
        printBits(currentFile.output[i]);
    }
}
