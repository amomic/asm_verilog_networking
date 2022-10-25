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

enum logic [4:0] {INIT,SA, SB, S1, LOOP1, LOOP2, LOOP3,IFinLOOP,BCHECK, DONE} state_next, state_present;
logic [31:0] a_present, a_next, b_present, b_next, result_present, result_next, k_present, k_next;
enum logic [3:0] {INPUT_A, SWAP_A, CURRENT_A, A_CALC} a_mux;
enum logic [3:0] {INPUT_B, SWAP_B, CURRENT_B, B_CALC} b_mux;
enum logic [3:0] {RESULT_CURRENT, RA, RB, R0,RK} res_mux;

assign result_o = result_present;

always_ff @(posedge clk_i, posedge rst_i) begin
  if(rst_i == 1) begin
    state_present <= INIT;
    result_present <= 31'b0;
    a_present <= 31'b0;
    b_present <= 31'b0;
    k_present <= 31'b0;
    end
  else begin
    state_present <= state_next;
    result_present <= result_next;
    a_present <= a_next;
    b_present <= b_next;
    k_present <= k_next;
  end  
end


always_comb begin
  state_next = state_present;
  a_mux = CURRENT_A;
  b_mux = CURRENT_B;
  res_mux = RESULT_CURRENT;
  busy_o = 1'b0;
  valid_o = 1'b0;

  case(state_present)

      INIT: begin
          a_mux = INPUT_A;
          b_mux = INPUT_B;

          if(start_i) begin
            state_next = SA;
          end
          else begin
            state_next = INIT;
          end
        end
      
      SA: begin
        busy_o = 1'b1;
        if(a_present == 31'b00) begin
          res_mux = RA;
          state_next = DONE;
        end
        else begin
          state_next = SB;
        end
      end

      SB: begin
        busy_o = 1'b1;
        if(b_present == 31'b00) begin
          res_mux = RB;
          state_next = DONE;
        end 
        else begin
          state_next = S1;
        end
      end

      S1: begin
        k_next = 31'b00;
        state_next = LOOP1;
      end

      LOOP1: begin
        busy_o = 1'b1;
        if(((a_present|b_present)& 2'b01)== 0) begin
          a_mux = A_CALC;
          b_mux = B_CALC;
          k_next = k_present + 31'b01;
        end
        else begin
          state_next = LOOP2;
         end
      end

      LOOP2: begin
        busy_o = 1'b1;
        if((a_present & 1'b1) == 0) begin
          a_mux = A_CALC;
        end
        else begin
          state_next = LOOP3;
        end
      end

      LOOP3: begin
        busy_o = 1'b1;
        if((b_present & 1'b1) == 0) begin
          b_mux = B_CALC;
        end
        else begin
          state_next = IFinLOOP;
        end
      end

      IFinLOOP:begin
        if(a_present > b_present) begin
          a_mux = SWAP_A;
          b_mux = SWAP_B;
        end
        b_next = b_present - a_present;
        state_next = BCHECK;
      end

      BCHECK: begin 
        if(b_present != 0) begin
          state_next = LOOP3;
        end
        else begin
          res_mux = RK;
          state_next = DONE;
        end
      end

      DONE: begin
        busy_o = 1'b0;
        valid_o = 1'b1;
         if(start_i) begin
          res_mux = R0;
          state_next = SA;
        end
        valid_o = 1'b1;
        state_next = INIT;

      end

      default:begin
        state_next = INIT;
      end
  endcase
end

//datapath

always_comb begin
  case(a_mux)
    INPUT_A: a_next = a_i;
    SWAP_A: a_next = b_present;
    CURRENT_A: a_next = a_present;
    A_CALC : a_next = a_present >> 2'b01;
    default: a_next = a_present;
    endcase
end


always_comb begin
  case(b_mux)
    INPUT_B: b_next = b_i;
    SWAP_B: b_next = a_present;
    CURRENT_B: b_next = b_present;
    B_CALC : b_next = b_present >> 2'b01;
    default: b_next = b_present;
    endcase
end

always_comb begin
  case(res_mux)
    RESULT_CURRENT: result_next = result_present;
    RA: result_next = b_present;
    RB: result_next = a_present;
    R0: result_next = 31'b00;
    RK: result_next = a_present << k_present;
    default: result_next = result_present;
    endcase
end





endmodule
