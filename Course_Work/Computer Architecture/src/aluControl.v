module aluControl(
    input [5:0] opcode,
    input [5:0] funcode, 
    input [1:0] aluOp,
    output [3:0] aluctl 
  );
reg [3:0] aluctl;
always @ (funcode, aluOp, opcode)
begin
    case (aluOp)
        //R-type
        2'b10: 
        case (funcode)
            6'b100100: aluctl = 4'b0000; //and
            6'b100101: aluctl = 4'b0001; //or
            6'b100110: aluctl = 4'b0010; //xor
            6'b100111: aluctl = 4'b0011; //nor
            6'b100000: aluctl = 4'b0100; //add
            6'b100001: aluctl = 4'b0100; //addu
            6'b100010: aluctl = 4'b0101; //sub
            6'b100011: aluctl = 4'b0101; //subu
            6'b011000: aluctl = 4'b0110; //mul
            6'b011001: aluctl = 4'b0110; //mulu
            6'b011010: aluctl = 4'b0111; //div
            6'b011011: aluctl = 4'b1000; //divu
            6'b101010: aluctl = 4'b0101; //slt
            6'b101011: aluctl = 4'b1001; //sltu
            6'b000000: aluctl = 4'b1011; //sll
            6'b000100: aluctl = 4'b1011; //sllv
            6'b000010: aluctl = 4'b1100; //srl
            6'b000110: aluctl = 4'b1100; //srlv
            6'b000011: aluctl = 4'b1101; //sra
            6'b000111: aluctl = 4'b1101; //srav
            default: aluctl = 4'b0000;
        endcase
        //lw,sw
        2'b00: aluctl = 4'b0100;
        //beq
        2'b01: aluctl = 4'b0101;
        //I-type
        2'b11:
        case (opcode)
            6'b001000: aluctl = 4'b0100; //addi
            6'b001001: aluctl = 4'b0100; //addiu
            6'b001100: aluctl = 4'b0000; //andi
            6'b001101: aluctl = 4'b0001; //ori
            6'b001110: aluctl = 4'b0010; //xori
            6'b000101: aluctl = 4'b1010; //bne
            6'b001010: aluctl = 4'b0101; //slti
            6'b001011: aluctl = 4'b1001; //sltui
            default: aluctl = 4'b0000;
        endcase
        default: aluctl = 4'b0000;
    endcase
end
endmodule // alu