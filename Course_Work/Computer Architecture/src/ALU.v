`include "aluControl.v"
module alu(i_datain,aluOp, gr1,gr2,result, zero, overflow, neg);

output reg signed[31:0] result;
output overflow;
output zero;
output neg;

input [31:0] i_datain,gr1, gr2;
input wire [1:0] aluOp;

reg[5:0] opcode, func;
wire[3:0] aluctl;

reg signed [31:0] reg_C;
reg signed [31:0] temp; // help to detect the multiplication overflow.
reg signed [31:0] reg_A, reg_B;
reg[31:0] u_reg_A, u_reg_B;
//This variable helps to detect the overflow.
reg extra;
//Special Outputs
reg zero;
reg neg;
reg overflow;

//Decode
always@(i_datain)
begin
    opcode = i_datain[31:26];
    func = i_datain[5:0];
end

aluControl alucontrol(opcode, func, aluOp, aluctl);
always @(i_datain, aluctl, gr1, gr2,aluOp)
begin
    reg_A = gr1; //rs
    u_reg_A = gr1;
    reg_B = gr2; //rt
    u_reg_B = gr2;
    neg = 0;
    overflow = 0;
    zero = 0;

    case (aluctl)
        4'b0000: reg_C = reg_A & reg_B; //and
        4'b0001: reg_C = reg_A | reg_B; //or
        4'b0010: reg_C = reg_A ^ reg_B; //xor
        4'b0011: reg_C = reg_A ~| reg_B; //nor

        4'b0100:  //add, addu
        begin
        {extra, reg_C} = {reg_A[31], reg_A} + {reg_B[31], reg_B};
        overflow = ({extra, reg_C[31]} == 2'b01 || {extra, reg_C[31]} == 2'b10);
        end

        4'b0101: //sub, subu, beq, slt.
        begin
        {extra, reg_C} = {reg_A[31], reg_A} - {reg_B[31], reg_B};
        overflow = ({extra, reg_C[31]} == 2'b01 || {extra, reg_C[31]} == 2'b10); // check overflow.
        zero = ((reg_A - reg_B) == 0);//beq.
        neg = reg_A < reg_B ? 1:0; //slt
        end

        4'b0110: //mul, mulu
        begin
        reg_C = reg_A * reg_B;
        temp = reg_C / reg_B;
        overflow = (temp != reg_A);
        end

        4'b0111: reg_C = reg_A / reg_B; //div
        4'b1000: reg_C = u_reg_A / u_reg_B; //divu

        4'b1001: //sltu
        begin
        neg = u_reg_A < u_reg_B ? 1:0;
        end

        4'b1010: zero = ((reg_A - reg_B) != 0); //bne

        4'b1011: //sll, sllv 
        begin
        case(func[2])
        0: 
        begin
        u_reg_B = i_datain[10:6];//sll
        reg_A = reg_B;
        end
        1:  u_reg_B = reg_B;//sllv
        endcase
        
        reg_C = reg_A << u_reg_B;
        end

        4'b1100: //srl, srlv
        begin
        case(func[2])
        0: 
        begin
        u_reg_B = i_datain[10:6];//srl
        reg_A = reg_B;
        end

        1: u_reg_B = reg_B; //srlv
        endcase
        reg_C = reg_A >> u_reg_B;
        end

        4'b1101://sra, srav
        begin
        case(func[2])
        0: 
        begin
        u_reg_B = i_datain[10:6];//sra
        reg_A = reg_B;
        end
        1: u_reg_B = reg_B; //srav
        endcase
        reg_C = reg_A >>> u_reg_B;
        end

        default:  reg_C = 0;
    endcase
    result = reg_C;
end
endmodule

