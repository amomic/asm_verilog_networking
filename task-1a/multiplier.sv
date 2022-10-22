module multiplier(
  input logic        clk_i,
  input logic        rst_i,
  input logic        start_i,
  input logic [3:0]  a_i,
  input logic [3:0]  b_i,
  output logic       busy_o,
  output logic       valid_o,
  output logic [7:0] result_o
);

//internal registers
enum logic [1:0] {INIT, CALC, DONE} state_present, state_next;
logic [7:0] result_present, result_next;
logic [7:0] counter_present, counter_next;


// data path
logic [1:0] result_mux;
always_comb begin
  case(result_mux)
      8'b00: result_next = result_present;
      8'b01: result_next = result_present + (b_i << counter_present);
      8'b10: result_next = 0;
      default: result_next = result_present;
  endcase
end

logic [1:0] counter_mux;
always_comb begin
  case(counter_mux)
      8'b00: counter_next = 0;
      8'b01: counter_next = counter_present + 1;
      8'b10: counter_next = counter_present;
  endcase
end

//output logic
assign result_o = result_next;
assign busy_o = (state_present == CALC);
assign valid_o = (state_present == DONE);

//state machine
always_comb begin
  state_next = state_present;
  result_mux = 8'b00;
  counter_mux = 8'b10;

  case(state_present)
      INIT: begin
        result_mux = 8'b10;
        counter_mux = 8'b00;

        if(start_i) begin
          state_next = CALC;
        end
      end

      CALC: begin
        counter_mux = 8'b01;

        if(a_i[counter_present]) begin
          result_mux = 8'b01;
        end

        if(counter_present == 3) begin
          state_next = DONE;
        end
      end

      DONE: begin
        result_mux = 8'b00;
        counter_mux = 8'b00;

        if(start_i) begin
          result_mux = 8'b10;
          state_next = CALC;
        end
      end
  endcase
end

//registers
always_ff @(posedge clk_i, posedge rst_i) begin
  if(rst_i == 1) begin
    state_present <= INIT;
    result_present <= 8'b00000000;
    counter_present <= 8'b00000000;
    end
  else begin
    state_present <= state_next;
    result_present <= result_next;
    counter_present <= counter_next;
  end  
end

endmodule
