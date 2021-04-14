
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <string>
#include "defines.h"
using std::static_pointer_cast;
using std::shared_ptr;
std::string translate_ADD(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::string translate_SUB(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::string translate_MULT(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::string translate_DIV(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::string translate_CONDITION(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers, int count_label);
std::string translate_IF(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::map<int, int> reverse_comparison_sign_map = init_reverse_comparison_sign_map();
//////////
// ADD ///
//////////
std::string translate_ADD(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers) {

    //static_cast to get derived class pointer.
    auto node = static_pointer_cast<Operator>(Node);
    shared_ptr<Statement> lnode0 = node->left;
    shared_ptr<Statement> rnode0 = node->right;
    std::string mips;
    // a = iden + 1
    if ((lnode0->type==ID)&&(rnode0->type==NUM)) {
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Number>(rnode0);
        std::string name = (lnode->name);
        std::string id_addr = identifiers[name];
        int cons_val = rnode->value;
        mips+="lw  $s0, "+id_addr+"\n"+"addiu  $s1, $s0, "+std::to_string(cons_val)+"\n";
    }
    // a = 1 + iden <-> a = iden + 1
    else if ((lnode0->type==NUM)&&(lnode0->type==ID)) {
        auto lnode = static_pointer_cast<Number>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        int cons_val = lnode->value;
        std::string name = (rnode->name);
        std::string id_addr = identifiers[name];
        std::cout<<"addiu $s1, $s0, "<<std::to_string(cons_val)<<std::endl;
        mips+="addiu $s1, $s0, "+std::to_string(cons_val)+"\n";
    }
    // a = iden + iden
    else if ((lnode0->type==ID)&&(lnode0->type==ID)) {
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        std::string rname = rnode->name;
        std::string rid_addr = identifiers[rname];
        std::string lname = lnode->name;
        std::string lid_addr = identifiers[lname];

        mips+="lw  $s0, "+lid_addr+"\n"+"lw  $s1, "+rid_addr+"\n"+"addu  $s0, $s1, $s0"+"\n";
    }
    // a = 1 + 2
    else if ((lnode0->type==NUM)&&(lnode0->type==NUM)) {
        auto lnode = static_pointer_cast<Number>(lnode0);
        auto rnode = static_pointer_cast<Number>(rnode0);
        int lcons_val = lnode->value;
        int rcons_val = rnode->value;
        int res = lcons_val+rcons_val;
        std::stringstream res_lo16, res_hi16;
        res_lo16 << (res & 0xffff); // lower 16 bits
        res_hi16 << (res >> 16); // higher 16 bits
        // li $s1, result
        mips+="lui  $s1, "+res_hi16.str()+", $s0\n"+"ori  $s1, $s0,"+res_lo16.str()+"\n";
    }

    return mips;
}


//////////
// SUB ///
//////////
std::string translate_SUB(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers) {
    //static_cast to get derived class pointer.
    auto node = static_pointer_cast<Operator>(Node);
    shared_ptr<Statement> lnode0 = node->left;
    shared_ptr<Statement> rnode0 = node->right;
    std::string mips;
    if ((lnode0->type==ID)&&(lnode0->type==NUM)) {
    // a = iden - 1
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Number>(rnode0);
        int cons_val = (rnode->value)*(-1);
        std::string name = (lnode->name);
        std::string id_addr = identifiers[name];
        mips+="lw  $s0, "+id_addr+"\n"+"addiu  $s1, $s0, "+std::to_string(cons_val)+"\n";
    }
    // a = 1 - iden
    else if ((lnode0->type==NUM)&&(lnode0->type==ID)) {
        auto lnode = static_pointer_cast<Number>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        int cons_val = lnode->value;
        std::string name = (rnode->name);
        std::string id_addr = identifiers[name];
        std::stringstream val_lo16, val_hi16;
        val_lo16 << (cons_val & 0xffff); // lower 16 bits
        val_hi16 << (cons_val >> 16); // higher 16 bits

        mips+="lui  $s1, "+val_hi16.str()+", $s0"+"\n"+"ori  $s1, $s0, "+val_lo16.str()+"\n"+"lw  $s0, "+id_addr+"\n""subu  $s0, $s1, $s0"+"\n";
    }
    // a = iden - iden
    else if ((lnode0->type==ID)&&(lnode0->type==ID)) {
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        std::string rname = rnode->name;
        std::string rid_addr = identifiers[rname];
        std::string lname = lnode->name;
        std::string lid_addr = identifiers[lname];

        mips+="lw  $s1, "+lid_addr+"\n"+"lw  $s0, "+rid_addr+"\n"+"subu  $s0, $s1, $s0"+"\n";

    }
    // a = 1 - 2
    else if ((lnode0->type==NUM)&&(lnode0->type==NUM)) {
        auto lnode = static_pointer_cast<Number>(lnode0);
        auto rnode = static_pointer_cast<Number>(rnode0);
        int lcons_val = lnode->value;
        int rcons_val = rnode->value;
        int result = lcons_val-rcons_val;
        std::stringstream res_lo16, res_hi16;
        res_lo16 << (result & 0xffff); // lower 16 bits
        res_hi16 << (result >> 16); // higher 16 bits
        mips+="lui  $s1, "+res_hi16.str()+", $s0"+"\n"+"ori  $s1, $s0, "+res_lo16.str()+"\n";
    }
    return mips;
}
//////////
// MULT //
//////////
std::string translate_MULT(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers) {
    //static_cast to get derived class pointer.
    auto node = static_pointer_cast<Operator>(Node);
    shared_ptr<Statement> lnode0 = node->left;
    shared_ptr<Statement> rnode0 = node->right;
    std::string mips;
    // a = iden * 2
    if ((lnode0->type==ID)&&(lnode0->type==NUM)) {
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Number>(rnode0);
        int cons_val = lnode->value;
        std::string name = (lnode->name);
        std::string id_addr = identifiers[name];
        mips+="lw  $s0, "+id_addr+"\n"+"addi  $s0, $zero, "+std::to_string(cons_val)+"\n"+"mult $s0, $s1"+"\n"+"mflo $s0"+"\n";

    }
    // a = 2 * iden
    else if ((lnode0->type==NUM)&&(rnode0->type==ID)) {
        auto lnode = static_pointer_cast<Number>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        int cons_val = lnode->value;
        std::string name = (rnode->name);
        std::string id_addr = identifiers[name];
        mips+="lw  $s0, "+id_addr+"\n"+"addi  $s0, $zero, "+std::to_string(cons_val)+"\n"+"mult $s0, $s1"+"\n"+"mflo $s0"+"\n";

    }
    // a = iden * iden
    else if ((lnode0->type==ID)&&(lnode0->type==ID)) {
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        std::string rname = rnode->name;
        std::string rid_addr = identifiers[rname];
        std::string lname = lnode->name;
        std::string lid_addr = identifiers[lname];
        mips+="lw  $s0, "+lid_addr+"\n"+"lw  $s1, "+rid_addr+"\n"+"mult $s0, $s1"+"\n"+"mflo $s0"+"\n";
    }
    // a = 1 * 2
    else if ((lnode0->type==NUM)&&(rnode0->type==NUM)) {
        auto lnode = static_pointer_cast<Number>(lnode0);
        auto rnode = static_pointer_cast<Number>(rnode0);
        int lcons_val = lnode->value;
        int rcons_val = rnode->value;
        int res = lcons_val*rcons_val;
        std::stringstream res_lo16, res_hi16;
        res_lo16 << (res & 0xffff); // lower 16 bits
        res_hi16 << (res >> 16); // higher 16 bits
        mips+="lui  $s1, "+res_hi16.str()+", $s0"+"\n"+"ori  $s1, $s0, "+res_lo16.str()+"\n";
    }
       return mips;
}


//////////
// DIV  //
//////////
// a = x / y
std::string translate_DIV(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers) {

    //static_cast to get derived class pointer.
    auto node = static_pointer_cast<Operator>(Node);
    shared_ptr<Statement> lnode0 = node->left;
    shared_ptr<Statement> rnode0 = node->right;
    std::string mips;
    // a = iden / 2
    if ((lnode0->type==ID)&&(rnode0->type==NUM)) {
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Number>(rnode0);
        int cons_val = rnode->value;
        std::string name = (lnode->name);
        std::string id_addr = identifiers[name];

        mips+="lw  $s0, "+id_addr+"\n"+"addi  $s1, $zero, "+std::to_string(cons_val)+"\n"+"div  $s0, $s1\n"+"mflo  $s0\n";

    }
    // a = 2 / iden
    else if ((lnode0->type==NUM)&&(rnode0->type==ID)) {
        auto lnode = static_pointer_cast<Number>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        int cons_val = lnode->value;
        std::string name = (rnode->name);
        std::string id_addr = identifiers[name];

        mips+="lw  $s1, "+id_addr+"\n"+"addi  $s0, $zero, "+std::to_string(cons_val)+"\n"+"div  $s0, $s1\n"+"mflo  $s0\n";
    }
    // a = iden / iden
    else if ((lnode0->type==ID)&&(rnode0->type==ID)) {
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        std::string rname = rnode->name;
        std::string rid_addr = identifiers[rname];
        std::string lname = lnode->name;
        std::string lid_addr = identifiers[lname];
        mips+="lw  $s0, "+lid_addr+"\n"+"lw  $s1, "+rid_addr+"\n"+"div  $s0, $s1\n"+"mflo  $s0\n";
    }
    // a = 1 / 2
    else if ((lnode0->type==NUM)&&(rnode0->type==NUM)) {
        auto lnode = static_pointer_cast<Number>(lnode0);
        auto rnode = static_pointer_cast<Number>(rnode0);
        int lcons_val = lnode->value;
        int rcons_val = rnode->value;
        int res = lcons_val/rcons_val;
        std::stringstream res_lo16, res_hi16;
        res_lo16 << (res & 0xffff); // lower 16 bits
        res_hi16 << (res >> 16); // higher 16 bits
        // li $s1, result
        mips+="lui  $s1, "+res_hi16.str()+", $s0\n"+"ori  $s1, $s0"+res_lo16.str()+"\n";
    }
    return mips;
}


///////////////
// CONDITION //
///////////////
std::string translate_CONDITION(std::shared_ptr<Operator> Node, std::map<std::string, std::string> &identifiers, int count_label) {

    std::string label;
    std::string mips;
    shared_ptr<Statement> lnode0 = Node->left;
    shared_ptr<Statement> rnode0 = Node->right;
    // fetch condition of statement(>/==/!=/<)
    int condition = Node->type;
    // define a label for each if statement
    label="L_"+std::to_string(count_label);

    // if (a??3) or (3??a)
    if (  ((lnode0->type==ID)&&(rnode0->type==NUM)) || ((rnode0->type==ID)&&(lnode0->type==NUM))  ) {

        std::string id_addr;
        int int_val;
        // (a??3)
        if (lnode0->type==ID) {
            auto lnode = static_pointer_cast<Variable>(lnode0);
            auto rnode = static_pointer_cast<Number>(rnode0);
            id_addr = identifiers[lnode->name];
            int_val = rnode->value;
        }
        // (3??a)
        else {
            auto lnode = static_pointer_cast<Number>(lnode0);
            auto rnode = static_pointer_cast<Variable>(rnode0);
            id_addr = identifiers[rnode->name];
            int_val = lnode->value;
            condition = reverse_comparison_sign_map[condition];
        }

        mips+="lw  $s0, "+id_addr+"\n";
        // a < 3
        if (condition==LT){
            mips+="slt  $s0, $s0, "+std::to_string(int_val)+"\n"+"beq  $s0, $zero, "+label+"\n";
        }

        // a <= 3
        if (condition==LE){
            mips+="slt  $s0, $s0, "+std::to_string(int_val-1)+"\n"+"beq  $s0, $zero, "+label+"\n";
        }

        // a > 3
        else if (condition==GT) {
            mips+="slt  $s0, $s0, "+std::to_string(int_val+1)+"\n"+"bne  $s0, $zero, "+label+"\n";
        }

        // a >= 3
        else if (condition==GE) {
            mips+="slt  $s0, $s0, "+std::to_string(int_val)+"\n"+"bne  $s0, $zero, "+label+"\n";
        }

        // a == 3
        else if (condition==EQ) {
            mips+="lw  $s1, "+id_addr+"\n"+"li $s0, "+std::to_string(int_val)+"\nbne  $s1, $s0, "+label+"\n";
        }
        // a != 3
        else if (condition==NEQ) {
            mips+="lw  $s1, "+id_addr+"\n"+"li  $s0, "+std::to_string(int_val)+"beq $s1, $s0, "+label+"\n";
        }
    }

    // if (a??b)
    if ((lnode0->type==ID)&&(rnode0->type==ID)) {
        auto lnode = static_pointer_cast<Variable>(lnode0);
        auto rnode = static_pointer_cast<Variable>(rnode0);
        std::string lid_addr = identifiers[lnode->name];
        std::string rid_addr = identifiers[rnode->name];

        mips+="lw  $s0, "+lid_addr+"\nlw  $s1, "+rid_addr+"\n";
        // a < b
        if (condition==LT){
            mips+="slt  $s0, $s1, $s0\nbeq  $s0, $zero, "+label+"\n";
        }

        // a <= b
        if (condition==LE){
            mips+="slt  $s0, $s0, $s1\nbne  $s0, $zero, "+label+"\n";
        }

        // a > b
        else if (condition==GT) {
            mips+="slt  $s0, $s0, $s1\nbeq  $s0, $zero, "+label+"\n";
        }

        // a >= b
        else if (condition==GE) {
            mips+="slt  $s0, $s1, $s0\nbne  $s0, $zero, "+label+"\n";
        }

        // a == b
        else if (condition==EQ) {
            mips+="lw  $s1, "+lid_addr+"\nlw  $s0, "+rid_addr+"\nbne $s1, $s0, "+label+"\n";
        }
        // a != b
        else if (condition==NEQ) {
            mips+="lw  $s1, "+lid_addr+"\n"+"lw $s0, "+rid_addr+"beq $s1, $s0, "+label+"\n";
        }

    }
    return mips;
}

//////////
//  IF  //
//////////
std::string translate_IF(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers, int count_label) {

    //static_cast to get derived class pointer.
    auto temp = Node;
    auto node = static_pointer_cast<Operator>(temp);

    shared_ptr<Statement> lnode0 = node->left;// BOOL
    shared_ptr<Statement> rnode0 = node->right;// DO
    std::string mips;

    auto lnode = static_pointer_cast<Operator>(lnode0);
    auto rnode = static_pointer_cast<Compound>(rnode0);

    mips+=translate_CONDITION(lnode, identifiers, count_label);
    mips+=translate_compound(rnode);

    return mips;
}
/////////////
//  WHILE  //
/////////////
std::string translate_WHILE(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers, int count_label) {

    //static_cast to get derived class pointer.
    auto temp = Node;
    auto node = static_pointer_cast<Operator>(temp);

    shared_ptr<Statement> lnode0 = node->left;// BOOL
    shared_ptr<Statement> rnode0 = node->right;// DO

    std::string mips;

    auto lnode = static_pointer_cast<Operator>(lnode0);
    auto rnode = static_pointer_cast<Compound>(rnode0);

    mips+=translate_CONDITION(lnode, identifiers, count_label);
    mips+=translate_compound(rnode);

    mips+="b L_"+std::to_string(count_label-1)+"\n";

    return mips;
}
