module regFile(
    input clock,
    input start,
    input [31:0] instr,
    input WE, //Write Enable Signal
    input [4:0]WriteDst,
    input [31:0] d_datain, //Data write to W1
    output reg [31:0] out1,
    output reg [31:0] out2
);
reg[31:0] RF[31:0];//An array of 32 registeres each of 32 bits.
integer i;
always @(start)
begin
    for (i = 1;i < 32 ; i = i+1) begin
        RF[i] = 32'h0000_0002;
    end
    //$zero 
    RF[0] = 32'h0000_0000;
    //set $s6 be 32'hFFFF_FFFFand $s7 be 32'h0000_0000 to test and, or instructions.
    RF[22] = 32'hFFFF_FFFF;
    RF[23] = 32'h0000_0000;
    RF[31] = 32'h0000_0000;
end

always @(instr, d_datain, WE)
begin
    //Write Back
    if(WE == 1 && WriteDst != 0)
    RF[WriteDst] = d_datain;
    //Get two outputs from RF according to the Instruction.
    out1 = RF[instr[25:21]];
    out2 = RF[instr[20:16]];
end
endmodule 