module gcd (
  input  logic        clk_i,
  input  logic        rst_i,
  input  logic        start_i,
  input  logic [31:0] a_i,
  input  logic [31:0] b_i,
  output logic        busy_o,
  output logic        valid_o,
  output logic [31:0] result_o
);
endmodule
