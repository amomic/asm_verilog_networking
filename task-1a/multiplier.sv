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


//clock and reset
always_ff @(posedge clk_i, posedge rst_i) begin
  if(rst_i == 1) begin
    state_present <= INIT;
    end
  else begin
    state_present <= state_next;
  end  
end

always_ff @(posedge clk_i, posedge rst_i) begin
  if(rst_i == 1) begin
    result_present <= 8'b00000000;
    result_next <= 8'b00000000;
  end
  else begin
    result_present <= result_next;
  end
end

always_ff @(posedge clk_i, posedge rst_i) begin
  if(rst_i == 1) begin
    counter_present <= 8'b00000000;
    counter_next <= 8'b00000000;
  end
  else begin
    counter_present <= counter_next;
  end; 
end

//next state calculation
always_comb begin
  state_next = state_present;
   case(state_present)
      INIT: begin if(start_i ==1) begin
        state_next = CALC;
      end
        else begin
         state_next = INIT;
        end
      end
      CALC: begin if(counter_present == 3) begin
        state_next = DONE;
      end
        else begin
          state_next = CALC;
        end
      end
      DONE: begin if(start_i == 1) begin
        state_next = CALC;
      end
        else begin
          state_next = DONE;
        end
      end
      default: begin 
        state_next = state_present;
      end
endcase
end

//output logic 
always_comb begin
  case(state_present)
    INIT: begin 
      busy_o = 0;
      valid_o = 0;
      result_present = 8'b00000000;
      counter_present = 8'b00000000;
      result_next = 8'b00000000;
      counter_next = 8'b00000000;
    end
    CALC: begin
      busy_o = 1;
      valid_o = 0;
      counter_next = counter_present + 8'b00000001;
      if(a_i[counter_present] == 1) begin
        result_next = result_present + (b_i << counter_present);
      end
      else begin
        result_next = result_present;
      end
    end
    DONE: begin
      busy_o = 0;
      valid_o = 1;
      result_next = result_present;
      counter_next = 8'b00000000;
      if(start_i == 1) begin
        result_next = 8'b00000000;
      end
      else begin
        result_next = result_present;
      end
    end
    default: begin
             busy_o = 0;
             valid_o = 0;
             result_present = 8'b00000000;
             result_next = 8'b00000000;
             counter_present = 8'b00000000;
             counter_next = 8'b00000000;
    end
  endcase
end

assign result_o = result_next;
endmodule
