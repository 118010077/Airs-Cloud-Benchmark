//This a Instruction Memory Module
module Imem(
    input clock,
    input [31:0] address,
    output reg [31:0] instruction
);
reg [31:0] IM [0:200]; //Set totally 2*26 Words.
initial
begin
    $readmemb("ins.txt", IM);
end
always@ (address)
begin
    instruction = IM[address/4];
end
endmodule