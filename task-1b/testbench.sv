
module testbench_gcd ();
  logic         clk;
  logic         reset;
  logic         start;
  logic [31:0]  a;
  logic [31:0]  b;
  logic         busy;
  logic         valid;
  logic [31:0]  result;

  integer infile, outfile, read;
  logic [31:0] expected_gcd;

  gcd gcd_i(
    .clk_i      (clk),
    .rst_i      (reset),
    .start_i    (start),
    .a_i        (a),
    .b_i        (b),
    .busy_o     (busy),
    .valid_o    (valid),
    .result_o   (result)
  );

  always begin
    #50 clk = ~clk;
  end

  initial begin
    $dumpfile("_sim/gcd.vcd");
    $dumpvars(0, testbench_gcd);
  end

  initial begin
    infile = $fopen("./testcases/testcases_gcd.txt", "r");
    outfile = $fopen("_sim/output_gcd.txt", "w");

    clk   = 0;
    start = 0;
    reset = 1;
    #100
    reset = 0;
    #100
    a = 0;
    b = 0;

    while (1 == 1) begin
      read = $fscanf(infile, "%d %d %d", a, b, expected_gcd);
      if (read == -1) begin
        $fdisplay(outfile, "All tests completed successfully!");
        $display("All tests completed successfully!");
        #200
        $finish();
      end

      #120
      start = 1;
      #120
      start = 0;
      wait(valid);
      $display("gcd(%d, %d) = %d (expected %d)", a, b, result, expected_gcd);
      $fdisplay(outfile, "gcd(%d, %d) = %d (expected %d)", a, b, result, expected_gcd);

      if (expected_gcd != result) begin
        $fdisplay(outfile, "Unexpected result on last computation!");
        $display("Unexpected result on last computation!");
        #200
        $finish();
      end

      wait(!valid); // reset cycle
    end
  end

  // always @(posedge clk)
  // begin
  //   $display("state_cs = %d, reg_a_q = %d, reg_b_q = %d", gcd_i.state_cs, gcd_i.reg_a_q, gcd_i.reg_b_q);
  // end

endmodule
