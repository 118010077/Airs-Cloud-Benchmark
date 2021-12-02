`timescale 1ns / 1ps
`include "control.v"
`include "ALU.v"
`include "Reg.v"
`include "Instruction.v"
`include "Memory.v"
//This is an example for a single cycle cpu.
//You should:
//1. Extend the combinational logic part to construct a single cycle processor.(85%)
//2. Add more sequential logic to construct a pipeline processor.(100%)

module CPU(
    input wire clock,
    input wire start,
    output wire [31:0] d_dataout
    );
    
    //Instruction 
    wire [31:0] instruction;
    //Program Counter Branch
    reg PCSrcM ;
    //Program Counter Jump
    reg PCJump;
    //Program Counter
    reg [31:0]pc;
    reg [31:0]pcD;
    reg [31:0]pcE;
    reg [31:0]pcBranchE;
    reg [31:0]pcBranchM;
    reg [31:0]pcJumpD; //Jump instruction's target (in word address).
    //Pipeline Registers
    reg [63:0] IFID;
    reg [184:0] IDEX;
    reg [107:0] EXMEM;
    reg [70:0] MEMWB;
    //Flush Control Signals
    wire IF_FLUSH;


    //Instrument "Registers"
    reg [31:0] instrD;
    reg [31:0] instrE; //Used For ALU

    //InstrDuction Decode - Stage 2
    reg [5:0]opcode;
    reg [5:0]func;

    //Register File Output - Stage 2 and Stage 3
    wire [31:0] RD1;
    wire [31:0] RD2;
    reg [31:0] RD1E;
    reg [31:0] RD2E;
    //Register Rt, Rd (Possible Write Location) 
    reg [4:0] RtD;
    reg [4:0] RtE;
    reg [4:0] RdD;
    reg [4:0] RdE;
    reg [4:0] RsD;
    reg [4:0] RsE;
    //Sign extension (Immediate for I-type) 
    reg [31:0] immD;
    reg [31:0] immE;
    //Inputs of ALU 
    reg [31:0] SrcAE;
    reg [31:0] SrcBE;

    //Outputs of ALU (Stage 3 - Stage 5)
    wire [31:0] resultE;
    wire overflowE;
    wire zeroE;
    wire negE;
    reg overflowM;
    reg negM;
    reg zeroM; //Result from ALU to check Branch Equation
    reg [31:0] resultM; //Data writen to the Memory
    reg [31:0] resultW; //Data writen to the Register File
    
    //Data from Memory
    wire [31:0] ReadDataM;
    reg [31:0] ReadDataW;

    //Write to Register Signal (lw, # of Registers)
    reg [4:0] WriteRegE; 
    reg [4:0] WriteRegM;
    reg [4:0] WriteRegW; //Address of Register File

    //Write to Memory (sw, data)
    reg [31:0] WriteDataE;
    reg [31:0] WriteDataM;

    
    //Control Signal
    //ALU Control Signal
    //aluSrc: 1 -> Choose Imm as the second input of ALU (I-type) 0 -> Choose Rt as the second input.
    wire [1:0] aluOpD;
    wire aluSrcD;
    reg [1:0] aluOpE;
    reg aluSrcE;

    //RegDst Control Signal 1 -> Rd(R-type); 0 -> Rt (lw);
    wire [1:0] RegDstD;
    reg [1:0] RegDstE;

    //MemWriteControl Signal 1-> Write; 0 -> None
    wire MemWriteD;
    reg MemWriteE;
    reg MemWriteM;

    //RegWrite Signal 1-> Write (R-type, lw); 0 -> None (sw, beq, J-type)
    wire RegWriteD;
    reg RegWriteE;
    reg RegWriteM;
    reg RegWriteW;

    //MemtoReg Signal 1-> Write the Data from Memory to Register File 0 -> Write the Data from ALU
    wire MemtoRegD;
    reg MemtoRegE;
    reg MemtoRegM;
    reg MemtoRegW;

    //Branch Signal 1-> The branch may be token (Depends on the result of ALU ).
    // 0 -> PC = PC +4;
    wire BranchD;
    reg BranchE;
    reg BranchM;
    //Jump Signal 1-> Jump 0-> As Usual
    wire JumpD;
    reg JumpM;
    //MemSrc Signal 1-> jal 0 -> As usual 
    wire MemSrcD;
    reg MemSrcE;

    //Data Hazard Forwarding Signal
    reg [1:0]ForwardA;
    reg [1:0]ForwardB;
    reg DataHazard;

    //Control Hazard Signal 
    reg ControlHazard;
    //Output:
    reg [31:0]reg_A;  //First input of ALU
    reg [31:0]reg_B;  //Second input of ALU
    reg [31:0]reg_C;  //Write Back Data
    reg [31:0]reg_C1; //Write Back Data
    reg [31:0]d_datain; //Write Back Data

//Module Instantiation
controlUnit CTL(clock, opcode, func, aluSrcD,
           aluOpD, RegDstD, BranchD, JumpD,  MemWriteD, MemtoRegD, RegWriteD, MemSrcD, IF_FLUSH);
regFile REG(clock, start, instrD,
            RegWriteW, WriteRegW, d_datain, RD1, RD2);
alu ALU (instrE, aluOpE, SrcAE, SrcBE,resultE, zeroE, overflowE, negE);
Imem IM(clock, pc, instruction);
RAM mem(clock, MemWriteM, WriteDataM, resultM, ReadDataM);
//Sequential Logic

always@(start)
begin
    pc = 32'h00000000;
    PCSrcM = 0;
end
always @(posedge clock)
begin
    //First Stage (Sequential):
    //Change PC and Fectch the instrDuction, then save it into IFID Reg.
    if(PCSrcM == 0 && ControlHazard == 1)
        pc <= pc;
    if(PCSrcM == 0 && ControlHazard == 0)
        pc <= pc + 4;
    if(PCJump == 1)
        pc <= pcJumpD;
    if(DataHazard == 1)
        pc <= pc - 32'h00000004;
    
    if(PCSrcM == 1)
    begin
        pc <= pcBranchM;
        PCSrcM = 0; //Reset the PCSrcM value.
    end
    //Second Stage: Decode and get data from Register File.
    //Generate Control Signals.
    //Do Sign Extension
    instrD <= IFID[63:32];
    pcD <= IFID[31:0];


    //Third Stage: Choose the Input of ALU and get the Branch Address
    //Then, save the results from ALU, address, into EXMEM Register.
    //Control Signals

    MemSrcE <= IDEX[184];
    RegWriteE <= IDEX[183];
    MemtoRegE <= IDEX[182];
    MemWriteE <= IDEX[181];
    BranchE <= IDEX[180];
    aluOpE <= IDEX[179:178];
    aluSrcE <= IDEX[177];
    RegDstE <= IDEX[176:175];
    //Save other data from Register File.
    instrE <= IDEX[174:143];
    RD1E <= IDEX[142:111];
    RD2E <= IDEX[110:79];
    RsE <= IDEX[78:74];
    RtE <= IDEX[73:69];
    RdE <= IDEX[68:64];
    //Other Data -- Stage 3
    immE <= IDEX[63:32];
    WriteDataE <= IDEX[110:79]; //Data in Register File. 
    pcE <= IDEX[31:0];

    //Fourth Stage: Write/Read data to/from Data Memory. Save the output into MEMWB
    //Control Sginals
    RegWriteM <= EXMEM[107];
    MemtoRegM <= EXMEM[106];
    MemWriteM <= EXMEM[105];
    BranchM  <= EXMEM[104];

    //Other Results from ALU and PC 
    negM <= EXMEM[103];
    overflowM <= EXMEM[102];
    zeroM <= EXMEM[101];
    resultM <= EXMEM[100:69];
    WriteDataM <= EXMEM[68:37];
    WriteRegM <= EXMEM[36:32];
    pcBranchM <= EXMEM[31:0];

    //Fifth Stage: Write Data back to the Register File. (Choose between the result from ALU/MEM)
    RegWriteW <= MEMWB[70];
    MemtoRegW <= MEMWB[69];
    resultW <= MEMWB[68:37];
    ReadDataW <= MEMWB[36:5];
    WriteRegW <= MEMWB[4:0];
end


//First Stage(cont.)--Combinational Logic
always@(pc, instruction, IF_FLUSH)
begin
    IFID[63:32] = instruction;
    IFID[31:0] = (pc+4);
    //FLUSH Control
    //If FLUSH, all the control signals are set to 1
    if(IF_FLUSH == 1)
        IFID = 32'h00000000;
end

//Second Stage(cont.)--Combinational Logic
//Decode and get data from the Register File. Save data into ID/EX
always@(instrD, pcD, RegWriteD, MemWriteD, BranchD, aluSrcD, aluOpD,RD1, RD2, RegDstD, JumpD, IF_FLUSH )
begin
    ControlHazard = 0;
    if (BranchD == 1)
    begin
    ControlHazard = 1;
    IFID = 0;
    end
    PCJump = JumpD;
    opcode = instrD[31:26];
    func = instrD[5:0];
    immD = $signed(instrD[15:0]);
    RsD = instrD[25:21];
    RtD = instrD[20:16];
    RdD = instrD[15:11];
    //Check whether it is an branch instruction (beq/bne)
    //If so, flush the next three instructions

    //Calculate pcJumpD:
    if(JumpD == 1 && aluSrcD == 1)
    pcJumpD = (instrD[25:0] <<2); //J and Jal 
    if(JumpD == 1 && aluSrcD == 0)
    pcJumpD = (RD1 << 2); //Jr

    //Save value into ID/EX register
    IDEX[184] = MemSrcD;
    IDEX[183] = RegWriteD;
    IDEX[182] = MemtoRegD;
    IDEX[181] = MemWriteD;
    IDEX[180] = BranchD;
    IDEX[179:178] = aluOpD;
    IDEX[177] = aluSrcD;
    IDEX[176:175] = RegDstD;
    IDEX[174:143] = instrD;
    IDEX[142:111] = RD1; //Value of Output1
    IDEX[110:79] = RD2; //Value of Output2
    IDEX[78:74] = RsD;
    IDEX[73:69] = RtD; 
    IDEX[68:64] = RdD;
    IDEX[63:32] = immD;
    IDEX[31:0] = pcD;
    
end

//Third Stage(cont.) -- Combinational Logic
//Save the results from ALU. Save the address of the Branch Insturction.
//Save the # of register that will be writen back.

always @(instrE, resultE, zeroE, overflowE, negE, WriteDataE, pcE, immE, MemSrcE,ForwardA, ForwardB
)
begin
    if (BranchE == 1)
    begin
    ControlHazard = 1;
    IFID = 0;
    end
    DataHazard = 0;
    if((instrE[31:26] == 6'b100011) && ((RtE == RsD)||(RtE == RtD)))
    begin
    IDEX = 0;
    DataHazard = 1;
    end
    //Inputs of ALU for Branch Instructions
    if((instrE[31:26] == 6'b000100) || instrE[31:26] == 6'b000101)
    begin
    SrcAE = RD1E; 
    SrcBE = RD2E;
    end
    else
    begin
    //Choose the first input of ALU
    SrcAE = RD1E; //Rs
    if(ForwardA == 2'b01)
    SrcAE = resultW; //Forwarding MEM Hazard
    if(ForwardA == 2'b10)
    SrcAE = resultM; //Forwarding EX Hazard
    
    //Choose the second input of ALU
    //ALUSrcE Choose the input of ALU (R-type / I-type)
    if(aluSrcE == 0)
    SrcBE = RD2E; //Value of Rt
    if(ForwardB == 2'b01)
    SrcBE = resultW;
    if(ForwardB == 2'b10)
    SrcBE = resultM;
    if(aluSrcE == 1)
    SrcBE = immE; //Imm (32bits)
    end
    //RegDstE Choose which register will be writen later in the 5th stage.
    if(RegDstE == 2'b00)
    WriteRegE = RtE; //Rt (#)
    if(RegDstE == 2'b01) 
    WriteRegE = RdE; //Rd (#)
    if(RegDstE == 2'b10)
    WriteRegE = 5'b11111;

    //MemSrcE Choose the value for WriteData (Jal or other instructions)
    if(MemSrcE == 1)
    WriteDataE = pcE;
    //Branch Instruction Address
    pcBranchE = (immE << 2) + pcE;
    EXMEM[31:0] = pcBranchE;
    //# of the register writen back later
    EXMEM[36:32] = WriteRegE;
    EXMEM[68:37] = WriteDataE;
    EXMEM[100:69] = resultE; 
    EXMEM[101] = zeroE;
    EXMEM[102] = overflowE;
    EXMEM[103] = negE;
    EXMEM[104] = BranchE;
    EXMEM[105] = MemWriteE;
    EXMEM[106] = MemtoRegE;
    EXMEM[107] = RegWriteE;
    //For jal:
    if(instrE[31:26] == 6'b000011)
    EXMEM[100:69] = WriteDataE;
end

//Fourth Stage(cont.) --Combinational Logic
//Save the result from Data Memory and the address of Register need to be writen back
//Save the result of ALU.

always@ (zeroM, BranchM, resultM, ReadDataM, WriteRegM)
begin
    if (BranchM == 1)
    begin
    ControlHazard = 1;
    IFID = 0;
    PCSrcM = zeroM;
    end 
    MEMWB[70] = RegWriteM;
    MEMWB[69] = MemtoRegM;
    MEMWB[68:37] = resultM;
    MEMWB[36:5] = ReadDataM;
    MEMWB[4:0] = WriteRegM;
end

//Fifth Stage(cont.) --Combinational Logic
//Choose the data that will be writen back to the Register File (ALU/MEM)
always@ (resultW, ReadDataW, WriteRegW,MemtoRegW)
begin
    if(MemtoRegW == 1)
    d_datain = ReadDataW;
    if(MemtoRegW == 0)
    d_datain = resultW;
end
always @(instrE, resultM,  WriteDataM,RegWriteM, RegWriteW, WriteRegM, WriteRegW,RsE,RtE)
begin
    //EX Hazard 
    ForwardA = 2'b00;
    ForwardB = 2'b00;
    if((RegWriteM == 1) && (WriteRegM != 0) && (WriteRegM == RsE))
        ForwardA = 2'b10;
    if((RegWriteM == 1) && (WriteRegM != 0) && (WriteRegM == RtE))
        ForwardB = 2'b10;
    //Mem Hazard
    if((RegWriteW == 1) && (WriteRegW != 0) && (WriteRegW == RsE) 
    && (~((RegWriteM == 1) && (WriteRegM != 0) && (WriteRegM == RsE))))
        ForwardA = 2'b01;
    if((RegWriteW == 1) && (WriteRegW != 0) && (WriteRegW == RtE)
    && (~((RegWriteM == 1) && (WriteRegM != 0) && (WriteRegM == RtE))))
        ForwardB = 2'b01;
end
endmodule
                
