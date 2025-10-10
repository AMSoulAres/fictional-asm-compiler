#include "Assembler.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>

void Assembler::initialize_optab() {
    optab    = {1, 2, 1};
    optab    = {2, 2, 1};
    optab   = {3, 2, 1};
    optab    = {4, 2, 1};
    optab["JMP"]    = {5, 2, 1};
    optab["JMPN"]   = {6, 2, 1};
    optab["JMPP"]   = {7, 2, 1};
    optab["JMPZ"]   = {8, 2, 1};
    optab   = {9, 3, 2};
    optab   = {10, 2, 1};
    optab  = {11, 2, 1};
    optab  = {12, 2, 1};
    optab = {13, 2, 1};
    optab   = {14, 1, 0};
}