// Test generate-case construct
module test_generate_case #(
  parameter int MODE = 2
);

  // Generate case statement
  generate
    case (MODE)
      0: begin : mode_zero
        wire [7:0] data_a;
        assign data_a = 8'hAA;
        initial $display("Generated MODE 0: data_a = %h", data_a);
      end
      
      1: begin : mode_one
        wire [15:0] data_b;
        assign data_b = 16'hBBBB;
        initial $display("Generated MODE 1: data_b = %h", data_b);
      end
      
      2: begin : mode_two
        wire [31:0] data_c;
        assign data_c = 32'hCCCCCCCC;
        initial $display("Generated MODE 2: data_c = %h", data_c);
      end
      
      default: begin : mode_default
        wire [3:0] data_default;
        assign data_default = 4'hF;
        initial $display("Generated DEFAULT: data_default = %h", data_default);
      end
    endcase
  endgenerate
  
  // Test with genvar in nested generate
  genvar i;
  generate
    for (i = 0; i < 2; i++) begin : gen_loop
      case (i)
        0: begin : case_zero
          wire zero_wire;
          assign zero_wire = 1'b0;
        end
        1: begin : case_one
          wire one_wire;
          assign one_wire = 1'b1;
        end
      endcase
    end
  endgenerate

  initial begin
    #10;
    $display("Generate case test completed");
    $finish;
  end

endmodule

// Test module instantiation with different parameters
module test_generate_case_top;
  
  test_generate_case #(.MODE(0)) inst0();
  test_generate_case #(.MODE(1)) inst1();
  test_generate_case #(.MODE(2)) inst2();
  test_generate_case #(.MODE(99)) inst_default();
  
endmodule