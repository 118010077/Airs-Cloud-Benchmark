module controlUnit(
    input wire clock,
    input [5:0] opcode,
    input [5:0] func,
    output aluSrcD,
    output [1:0]aluOpD,
    output [1:0]RegDstD,
    output BranchD,
    output JumpD,
    output MemWriteD,
    output MemtoRegD,
    output RegWriteD,
    output MemorySrcD,
    output IF_Flush
    );


    //ALU Control Signal
    //aluSrc: 1 -> Choose Imm as the second input of ALU (I-type) 0 -> Choose Rt as the second input.
    reg [1:0] aluOpD;
    reg aluSrcD;

    //RegDst Control Signal 10 -> RD1E($Rs,jal) 01 -> Rd(R-type); 00 -> Rt (lw);
    reg [1:0] RegDstD;

    //MemWriteControl Signal 1-> Write; 0 -> None
    reg MemWriteD;

    //RegWrite Signal 1-> Write (R-type, lw); 0 -> None (sw, beq, J-type)
    reg RegWriteD;


    //MemtoReg Signal 1-> Write the Data from Memory to Register File 0 -> Write the Data from ALU
    reg MemtoRegD;


    //Branch Signal 1-> The branch may be token (Depends on the result of ALU ).
    // 0 -> PC = PC +4;
    reg BranchD;

    //Jump Signal 1-> Jump 0-> As Usual
    reg JumpD;
    reg IF_Flush;
    //MemorySrcD Signal is designed for jal instruction
    //1-> Choose Rt (lw), 0 -> Choose PC(jal)
    reg MemorySrcD;
//Combination Logic, assign Control Signal in Control Unit.
always @ (opcode, func)
begin
    case (opcode)
        //R-type
        6'b000000: 
        begin
        RegDstD = 2'b01; //Take Rt(15:11)
        aluSrcD = 0;
        aluOpD = 2'b10;
        RegWriteD = 1;
        MemWriteD = 0;
        BranchD = 0;
        JumpD = 0;
        MemtoRegD = 0;
        IF_Flush = 0;
        MemorySrcD = 0;
        //jr Instruction
        if(func == 6'b001000)
        begin  
        RegDstD = 2'b01;
        aluSrcD = 0; //Use this signal to distinct jr and j instruction.
        aluOpD = 2'b01;  
        RegWriteD = 0;
        MemWriteD = 0;
        BranchD = 0;
        JumpD = 1;
        MemtoRegD = 0;
        IF_Flush = 1;
        MemorySrcD = 0;
        end
        end 

        //LW
        6'b100011:
        begin
        RegDstD = 2'b00; //Take Rt (20:16) 
        aluSrcD = 1; 
        aluOpD = 2'b00;
        MemWriteD = 0;
        RegWriteD = 1;
        BranchD = 0;
        JumpD = 0;
        MemtoRegD = 1;
        IF_Flush = 0;
        MemorySrcD = 0;
        end

        //SW
        6'b101011:
        begin
        RegDstD = 2'b01;
        aluSrcD = 1;
        aluOpD = 2'b00;
        MemWriteD = 1;
        RegWriteD = 0;
        BranchD = 0;
        JumpD = 0;
        MemtoRegD = 0;
        IF_Flush = 0;
        MemorySrcD = 0;
        end

        //beq
        6'b000100:
        begin
        RegDstD = 2'b01;
        aluSrcD = 0; //Choose $rt instead of imm
        aluOpD = 2'b01;
        MemWriteD = 0;
        RegWriteD = 0;
        BranchD = 1;
        JumpD = 0;
        MemtoRegD = 0;
        IF_Flush = 0;
        MemorySrcD = 0;
        end
        //bne
        6'b000101:
        begin
        RegDstD = 2'b01;
        aluSrcD = 0; //Choose $rt instead of imm
        aluOpD = 2'b11;
        MemWriteD = 0;
        RegWriteD = 0;
        BranchD = 1;
        JumpD = 0;
        MemtoRegD = 0;
        IF_Flush = 0;
        MemorySrcD = 0;
        end
        //J-type
        //j
        6'b000010:
        begin
        RegDstD = 2'b01;
        aluSrcD = 1;
        aluOpD = 2'b01;
        RegWriteD = 0;
        MemWriteD = 0;
        BranchD = 0;
        JumpD = 1;
        MemtoRegD = 0;
        IF_Flush = 1;
        MemorySrcD = 0;
        end
        //jal
        6'b000011:
        begin
        RegDstD = 2'b10; //10 Choose 
        aluSrcD = 1;
        aluOpD = 2'b01;
        MemWriteD = 0;
        RegWriteD = 1;  //Save the address of PC to $ra 
        BranchD = 0; 
        JumpD = 1;
        MemtoRegD = 0;
        IF_Flush = 1;
        MemorySrcD = 1; //Set to 0. Choose PC as the WriteData.
        end
        //defalut. I-type (except bne), set ALUop to be 2'b11 and aluSrcD to be 1.
        default: 
        begin
        MemWriteD = 0;
        RegDstD = 2'b00; //01 Choose Rd(#)
        aluSrcD = 1;
        aluOpD = 2'b11;
        RegWriteD = 1;
        BranchD = 0;
        JumpD = 0;
        MemtoRegD = 0;
        IF_Flush = 0;
        MemorySrcD = 0;
        end
    endcase
end
endmodule 
