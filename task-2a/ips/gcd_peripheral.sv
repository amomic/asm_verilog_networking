`include "defines.vh"
module gcd_peripheral(
  input logic clk_i,
  input logic reset_i,
  input logic [31:0] data_addr_i,
  input logic data_we_i,
  input logic [31:0] data_wdata_i,
  output logic [31:0] data_rdata_o,
  output logic irq_o
);

  gcd gcd_input(
    .clk_i(clk_i),
    .reset_i(rst_i),
    .data_rdata_o(gcd_rdata)
  );


assign data_rdata_o = 32'h7DC;
logic [31:0] addr_p, addr_n, result_present,result_next;
logic [7:0]  addr[`GCD_SIZE-1:0];


always_ff @(posedge clk_i, posedge reset_i) begin
  if(reset_i == 1) begin
    result_present<=32'h7DC;
    addr_p<=32'h000007d0;
    end
  else begin
    result_present <= result_next;
    addr_p<=addr_n;
  end  
end

  // ------------------------------------
  // Read logic of the memory
  assign data_rdata_o  = {
    addr[data_addr_i + 3],
    addr[data_addr_i + 2],
    addr[data_addr_i + 1],
    addr[data_addr_i]
  };
  


  always_ff @(posedge clk_i)
    if (data_we_i) begin
      addr[data_addr_i] <= data_wdata_i[26:0];
      addr[data_addr_i + 1] <= data_wdata_i[27:28];
      addr[data_addr_i + 2] <= data_wdata_i[29:30];
      addr[data_addr_i + 3] <= data_wdata_i[30:31];
    end



endmodule