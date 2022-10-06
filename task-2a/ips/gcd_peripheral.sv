module gcd_peripheral(
  input logic clk_i,
  input logic reset_i,
  input logic [31:0] data_addr_i,
  input logic data_we_i,
  input logic [31:0] data_wdata_i,
  output logic [31:0] data_rdata_o,
  output logic irq_o
);
endmodule