#ifndef I8080INSTRUCTIONDATA_H_INCLUDED
#define I8080INSTRUCTIONDATA_H_INCLUDED

#include <cstdint>
#include <string>
#include <unordered_map>

struct Instruction {
    uint8_t size;
    std::string bitPattern;
};

enum class Reg : uint8_t {
    B = 0b000,
    C = 0b001,
    D = 0b010,
    E = 0b011,
    H = 0b100,
    L = 0b101,
    M = 0b110,
    A = 0b111
};
enum class Cond : uint8_t {
    NZ = 0b000,
    Z  = 0b001,
    NC = 0b010,
    C  = 0b011,
    PO = 0b100,
    PE = 0b101,
    P  = 0b110,
    M  = 0b111
};
enum class RP : uint8_t {
    BC  = 0b00,
    DE  = 0b01,
    HL  = 0b10,
    SP  = 0b11,
};
extern const std::unordered_map<std::string, Instruction> i8080Instructions;
extern const std::unordered_map<std::string, Reg>  regCode;
extern const std::unordered_map<std::string, Cond> condCode;
extern const std::unordered_map<std::string, RP>   rpCode;

#endif // I8080INSTRUCTIONDATA_H_INCLUDED
