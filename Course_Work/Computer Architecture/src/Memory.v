//Create a 4GB RAM
module RAM(
    input clock,
    input WE, //Write Enable
    input [31:0]data_in, //input data
    input [31:0]addr, //input address. (Byte address)
    output reg [31:0]data_out //output data
);
reg [31:0] RAM[0:200]; //Set Totally 2^26 Words

//Initialization of RAM
integer i;
initial begin
    for(i=0; i < 150; i = i+1)
        RAM[i] <= 32'h00000000;
end

always@(WE, data_in, addr)
begin
    if(WE == 1) RAM[addr/4] = data_in;  //SW
    if(WE == 0) data_out = RAM[addr/4]; //LW 
end
endmodule