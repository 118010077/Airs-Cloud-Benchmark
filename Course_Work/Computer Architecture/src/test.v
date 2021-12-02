`timescale 1ns/1ps
`include "CPU.v"




module CPU_test;

    // Inputs
	reg clock;
    reg start;
    reg [31:0] address;
    reg [31:0] instruction;
    wire [31:0] d_dataout;

    CPU uut(
        .clock(clock),
        .start(start), 
        .d_dataout(d_dataout)
    );

    initial begin
        // Initialize Inputs
        clock = 0;
        start = 1;
    //PC Instruction
    //RD1 RD2 are the two outputs from the register file that may be used in the next clock.
    //resultE is the main output of the ALU 
    //WriteReg is the data that will be writen back to the Register File
    $display("   pc   :          instruction           :   RD1  :   RD2  :  SrcAE : SrcBE  : resultE:WriteDataM: WriteRegW:d_datain");
    $monitor("%h:%b:%h:%h:%h:%h:%h:  %h:  %b   :%h   ", 
        uut.pc, uut.instruction,uut.RD1, uut.RD2, uut.SrcAE, uut.SrcBE, uut.resultE, uut.WriteDataM, uut.WriteRegW, uut.d_datain);
    #200
    $finish;
    end

parameter period = 10;
always #5 clock = ~clock;
endmodule
