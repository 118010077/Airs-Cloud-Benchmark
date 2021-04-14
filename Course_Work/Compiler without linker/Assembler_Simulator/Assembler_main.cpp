/* File: Assembler_main.cpp
 * Ruiyu GAO 118010074
 */

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <array>
#include <sstream>
#include <algorithm>
#include <bitset>
#include <iostream>
#include <fstream>
#include <cstring>
#include <map>
#include <unordered_map>
#include "Rtype.h"
#include "Itype.h"
#include "Jtype.h"
#include "Global.h"
using namespace std;

#define MAX_SIZE 10000
#define TEXT_START 0x00000000
int pc = TEXT_START;

// operation map
unordered_map <string, int> opmap = {
    {"add", 0}, {"addu", 0}, {"addi", 8}, {"addiu", 9},
    {"and", 0}, {"andi", 0xc},
    {"div", 0}, {"divu", 0}, {"mult", 0}, {"multu", 0},
    {"nor", 0}, {"or", 0},  {"ori", 0xd},  {"sll", 0}, {"sllv", 0},
    {"sra", 0}, {"srav", 0}, {"srl", 0}, {"srlv", 0},
    {"sub", 0}, {"subu", 0}, {"xor", 0}, {"xori", 0xe},
    {"lui", 0xf}, {"slt", 0}, {"sltu", 0}, {"slti", 0xa},
    {"slti", 0xa}, {"sltiu", 0xb},
    {"beq", 4}, {"bgez", 1},  {"bgezal", 1}, {"bgtz", 7},
    {"blez", 6}, {"bltzal", 1}, {"bltz", 1}, {"bne", 5},
    {"j", 2}, {"jal", 3}, {"jr", 0},
    {"teq", 0}, {"teqi", 1},  {"tne", 0}, {"tnei", 1},
    {"tge", 0}, {"tgeu", 0}, {"tgei", 1}, {"tgeiu", 1},
    {"tlt", 0}, {"tltu", 0}, {"tlti", 1}, {"tltiu", 1},
    {"lb", 0x20},  {"lbu", 0x24}, {"lh", 0x21},  {"lhu", 0x25},
    {"lw", 0x23},  {"lwl", 0x22}, {"lwr", 0x26},
    {"ll", 0x30}, {"sb", 0x28}, {"sw", 0x2b},
    {"swl", 0x2a}, {"swr", 0x2e}, {"sc", 0x38},
    {"mflo", 0}, {"mthi", 0}, {"mtlo", 0}
};

// fuction map
unordered_map <string, int> functmap = {
    {"add", 0x20}, {"addu", 0x21}, {"and", 0x24},
    {"div", 0x1a}, {"divu", 0x1b}, {"mult", 0x18}, {"multu", 0x19},
    {"nor", 0x27}, {"or", 0x25}, {"sll", 0},
    {"sllv", 4}, {"sra", 3}, {"srav", 7}, {"jr", 8},
    {"srl", 2}, {"srlv", 6}, {"sub", 0x22},
    {"subu", 0x23}, {"xor", 0x26}, {"slt", 0x2a},
    {"sltu", 0x2b}, {"jalr", 9}, {"mtlo", 0x13},
    {"mfhi", 0x10}, {"mflo", 0x12}, {"teq", 0x34},
    {"tne", 0x36}, {"tge", 0x30}, {"tgeu", 0x31},
    {"tlt", 0x32}, {"tltu", 0x33},{"mthi",0x11}
};

// rt map
unordered_map <string, int> rtmap = {
    {"bgez", 1}, {"bgezal", 0x11}, {"bgtz", 0}, {"blez", 0},
    {"bltzal", 0x10}, {"bltz", 0}, {"teqi", 0xc},
    {"tnei", 0xe}, {"tgei", 8}, {"tgeiu", 9}, {"tlti", 0xa},
    {"tltiu", 0xb}
};

// label map
map <string, int> labelmap;
unordered_map <string, int> v_map;
unordered_map <string, int> a_map;

// registers
const string registers[32] = {"zero","at", "v0", "v1", "a0", "a1", "a2", "a3",
                                "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
                                "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
                                "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"};
// R2: rs, rt
vector<int> r2_list = {0x1a, 0x1b, 0x18, 0x19, 0x34, 0x36, 0x30, 0x31, 0x32, 0x33};
// R3: rd, rt, shamt
vector<int> r3_list = {0, 3, 2};
// R4: rs, rd (jalr)
vector<int> r4_list = {9};
// R5: rs (jr, mthi, mflo)
vector<int> r5_list = {8, 0x11, 0x13};
// R6: rd (mflo, mfhi)
vector<int> r6_list = {0x10, 0x12};
// I2: rt, address (eg.lw, sw..)
vector<int> i2_list = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x29,0x2b, 0x2a, 0x2e, 0x30, 0x38};
// I4: rt, rs, imm (eg.addi, addiu)
vector<int> i4_list = {8,9, 0xa, 0xb, 0xc, 0xd, 0xe};

// delim element in strok
char delim[] = ", $\t";
char delim2[] = " \t";

int findRnumber(string name);
int nametoi(char* h);
int stringtoi(char* h);
array<int, 3> r1(char* token);
array<int, 2> r2(char* token);
array<int, 3> r3(char* token);
array<int, 2> r4(char* token);
int r5(char* token);
int r6(char* token);
bool isInVec(vector<int> v, int i);
string interpretor(char* line, int pc);

int Assemble () {
//    string filename = (string) argv[1];
    string filename;
    filename = assemble_file;
    char input_string[MAX_SIZE];    
    string data_name;
    char* token;
//    int target_address;

    string formatted_instruction;

    // open file for 1st read

    ifstream inFile(filename.c_str());
    while (inFile.getline(input_string, MAX_SIZE))   {
//        cout << "input string: " << input_string << endl;
        token = strtok(input_string, " ");
//        cout << "token: " << (token == NULL) << endl;
        if (token == NULL) continue;
        // if label, save its address to an unordered map
        if (strstr(token, ":"))   {
            cout << "label: " << token << pc << endl;
            labelmap[(string) strtok(token, ":")] = pc; // save (current pc + 4)
            pc += 4;
        }

        else {
            pc += 4;
        }
    }
//    for(int i=0; i < labelmap.size(); i++) cout << i << "'s pc is " << labelmap[i] << endl;
    // close file
    inFile.close();

    // open two files for 2nd read & write

    ifstream inFile2(filename.c_str());
    ofstream outFile(machine_file.c_str());

    // ------------------------------- read text section
    pc = TEXT_START;
    if (!inFile2.is_open())
    {
        cout << "未成功打开文件" << endl;
    }
    while (inFile2.getline(input_string, MAX_SIZE)) {
// Bug to be fixed
        // if label, pass
        if (strstr(input_string, ":"))   {
            cout << "input string: " << strstr(input_string, ":")+1<< endl;
            formatted_instruction = interpretor(strstr(input_string, ":")+1, pc);
            cout << formatted_instruction << endl;
            outFile << formatted_instruction << endl;
            result_file << formatted_instruction << endl;
//            outFile << "\t#current_label_pc is" << pc << endl;
            pc += 4;
            continue;
        }
        // instruction
        else {
            cout << "input string: " << input_string << endl;
            try{
                formatted_instruction = interpretor(input_string, pc);
                cout << formatted_instruction << endl;
                if (formatted_instruction == " ") continue;
                else {
                    outFile << formatted_instruction << endl;
                    result_file << formatted_instruction << endl;
                    pc += 4;
                }
            } catch(...) {
                cout << "interprete error" << endl;
            }
        }
    }


    // close input files
    inFile2.close();

    // write data section and close

    outFile.close();

    return 0;
    
};

int findRnumber(string name) {
    int n = 32;
    int index = 0;
    auto itr = find(registers, registers + n, name);

    if (itr != end(registers)) {
        index = distance(registers, itr);
    }
    else {
//        cout << "Register name is not present in the name list";/
    }
//    cout << "The register " << name << " 's number is: " << index << endl;
    return index;
}

bool isInVec(vector<int> v, int i){
    vector<int>::iterator ret;
    ret = find(v.begin(), v.end(), i);
    if(ret == v.end()) return false;
    else return true;
}

int nametoi(char* h) {
    string h_str = (string) h;
    int n;
    n = findRnumber(h_str);
    
    return n;
}
// to be simplified into 1,2,3 variable
// R1: rd, rs, rt
array<int, 3> r1(char* token) {
        array<int, 3> a;
        int rd = nametoi(token);
        a[0] = rd;

        token = strtok(NULL, delim);
        int rs = nametoi(token);
        a[1] = rs;
    
        token = strtok(NULL, delim);
        int rt = nametoi(token);
        a[2] = rt;
        return a;
};

// R2: rs, rt
array<int, 2> r2(char* token) {
        array<int, 2> a;
        int rs = nametoi(token);
        a[0] = rs;
        
        token = strtok(NULL, delim);
        int rt = nametoi(token);
        a[1] = rt;
        return a;
};

// R3: rd, rt, shamt
array<int, 3> r3(char* token) {
        array<int, 3> a;
        int rd = nametoi(token);
        a[0] = rd;

        token = strtok(NULL, delim);
        int rt = nametoi(token);
        a[1] = rt;

        token = strtok(NULL, delim);
        int shamt = stringtoi(token);
        a[2] = shamt;
        return a;
};

// R4: rs, rd (jair)
array<int, 2> r4(char* token) {
        array<int, 2> a;
        int rs = nametoi(token);
        a[0] = rs;
        
        token = strtok(NULL, delim);
        int rd = nametoi(token);
        a[1] = rd;
        return a;
};

// R5: rs (jr, mthi)
int r5(char* token) {
        int rs = nametoi(token);
        return rs;
};

// R6: rd (mflo, mfhi, mflo)
int r6(char* token) {
        int rd = nametoi(token);
        return rd;
};

// convert string(eg. "100") to int
int stringtoi(char* h) {
    string h_str = (string) h;

    stringstream geek(h_str);
    
    int x;
    geek >> x;

    return x;
}

string interpretor(char* line, int pc) {
    int op;
    
    char* token = strtok(line, delim);
//    cout << "token: " << (token == NULL) << endl;
    if (token == NULL) return " ";
    op = opmap[(string) token];
    cout << "The operation is: " << token << endl;
    cout << "The operation code is: " << op << endl;
    
//    int address;
    int rs, rt, rd, shamt, funct;
    int target, offset, imm;
    string result;
    
    Rtype r;
    Itype i;
    Jtype j;
    
    // to be simplified
    // R instructions
    if(op==0){
        funct = functmap[token];
        cout << "The function code is: " << funct << endl;
        token = strtok(NULL, delim);
        // R2: rs, rt
        if(isInVec(r2_list, funct)){
            cout << "type r2" << endl;
            array<int, 2> r2a;
            r2a = r2(token);
            rs = r2a[0];
            rt = r2a[1];
            r.setValue(op, rs, rt, 0, 0, funct);
            result = r.binCode();
        }
        // R3: rd, rt, shamt
        else if(isInVec(r3_list, funct)){
            cout << "type r3" << endl;
            array<int, 3> r3a;
            r3a = r3(token);
            cout << "type r3 finished" << endl;
            rd = r3a[0];
            rt = r3a[1];
            shamt = r3a[2];
            r.setValue(op, 0, rt, rd, shamt, funct);
            result = r.binCode();
        }
        // R4: rs, rd (jair)
        else if(isInVec(r4_list, funct)){
            cout << "type r4" << endl;
            array<int, 2> r2a;
            r2a = r4(token);
            rs = r2a[0];
            rd = r2a[1];
            r.setValue(op, rs, 0, rd, 0, funct);
            result = r.binCode();
        }
        // R5: rs (jr, mthi, mflo)
        else if(isInVec(r5_list, funct)){
            cout << "type r5" << endl;
            int ra = r5(token);
            rs = ra;
            r.setValue(op, rs, 0, 0, 0, funct);
            result = r.binCode();
        }
        // R6: rd (mflo, mfhi)
        else if(isInVec(r6_list, funct)){
            cout << "type r6" << endl;
            int ra = r6(token);
            rd = ra;
            r.setValue(op, 0, 0, rd, 0, funct);
            result = r.binCode();
        }
        // R1: rd, rs, rt
        else {
            cout << "type r1" << endl;
            array<int, 3> r3a;
            r3a = r1(token);
            rd = r3a[0];
            rs = r3a[1];
            rt = r3a[2];
            r.setValue(op, rs, rt, rd, 0, funct);
            result = r.binCode();
        }
    }
    // to be fixed
    // J type(j, jal)
    else if(op == 2 or op == 3){
        token = strtok(NULL, delim);
        // label
        target = labelmap[(string) token];
        j.setValue(op, target/4);
        result = j.binCode();
    }
    // I type
    else {
        try{
            rt = rtmap[token];
            cout << "The rt code is: " << rt << endl;
        } catch(...){
            rt = 99;
        }
        token = strtok(NULL, delim);
        // to be fixed
        // I1: rs, rt, label (beq, bne)
        if(op == 4 or op == 5) {
            cout << "type i1" << endl;
            rs = nametoi(token);

            token = strtok(NULL, delim);
            rt = nametoi(token);

            token = strtok(NULL, delim);
            target = labelmap[(string) token];
            cout << token << "'s pc is: " << target << endl;
            cout << "Now's pc is:" << pc << endl;
            offset = (target - (pc+4)) / 4;
            cout << "offset is: " << offset << endl;
            i.setValue(op, rs, rt, offset);
            result = i.binCode();
        }
        // I2: rt, address (eg.lw, sw..)
        else if(isInVec(i2_list, op)) {
            cout << "type i2" << endl;
            rt = nametoi(token);
//            cout << token << endl;
            token = strtok(NULL, " \t(");
//            cout << "offset token: " << token << endl;
            offset = stringtoi(token);
//            cout << "offset: " << offset << endl;
            
            token = strtok(NULL, "$) ");
            rs = nametoi(token);
            i.setValue(op, rs, rt, offset);
            result = i.binCode();
        }
        // I3 and I4 could be merged and simplified.
        // I3: rt, imm (lui, oxt)
        else if(op == 0xf){
            cout << "type i3" << endl;
            rt = nametoi(token);
            cout << "rt: " << rt << endl;
            token = strtok(NULL, delim);
            imm = stringtoi(token);
            i.setValue(op, 0, rt, imm);
            result = i.binCode();
        }
// bug to be fixed
        // I4: rt, rs, imm (eg.addi, addiu)
        else if(isInVec(i4_list, op)){
            cout << "type i4" << endl;
            rt = nametoi(token);
//            cout << "rt: " << rt << endl;
            token = strtok(NULL, delim);
            rs = nametoi(token);
//            cout << "rs: " << rt << endl;
            token = strtok(NULL, delim);
            imm = stringtoi(token);
//            cout << "immiedia " << imm << endl;
            i.setValue(op, rs, rt, imm);
            result = i.binCode();
        }
        else{
            // I5: rs, label (eg. bgez, bgezal...)
            if(rt == 0 or rt == 1 or rt == 0x11 or rt == 0x10){
                cout << "type i5" << endl;
                rs = nametoi(token);

                token = strtok(NULL, delim);
                target = labelmap[(string) token];
                cout << token << "'s pc is: " << target << endl;
                cout << "Now's pc is:" << pc << endl;
                offset = (target - (pc+4)) / 4;
                cout << "offset is: " << offset << endl;
                i.setValue(op, rs, rt, offset);
                result = i.binCode();
            }
            // I6: rs, imm (eg.teqi, tnei...)
            else {
                cout << "type i6" << endl;
                rs = nametoi(token);
                cout << "rs: " << rs << endl;
                cout << "rt: " << rt << endl;
                token = strtok(NULL, delim);
                imm = stringtoi(token);
                i.setValue(op, rs, rt, imm);
                result = i.binCode();
            }
        }
        
        
    }
    return result;
};

