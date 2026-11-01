// SukimaSim #3054: expression context must stop at an untyped actual's own type.
// IEEE 1800-2023 §16.8 keeps non-lvalue actuals self-determined before
// substitution. Direct comparisons still propagate their operand widths.
// Stable inputs are sampled twice, at 5 and 15; check all actions at 16.
`timescale 1ns/1ns
module test_issue3054_untyped_context_boundary;
  bit clk = 0;
  logic a = 1, b = 1;
  logic [7:0] aa = 8'haa, bb = 8'h55, two = 8'd2, zero = 0;
  logic [3:0] selected = 4'b1010;
  int directP = 0, directF = 0, untypedP = 0, untypedF = 0;
  int typedP = 0, typedF = 0, xnorP = 0, xnorF = 0;
  int actualP = 0, actualF = 0, compareP = 0, compareF = 0;
  int concatP = 0, concatF = 0, namedP = 0, namedF = 0;
  int defaultP = 0, defaultF = 0, nestedP = 0, nestedF = 0;
  int selectP = 0, selectF = 0, lvalueP = 0, lvalueF = 0;
  always #5 clk = ~clk;

  sequence two_untyped(x); x == 2'd2; endsequence
  sequence two_typed(logic [1:0] x); x == 2'd2; endsequence
  sequence pattern32(x); x == 32'hffffff00; endsequence
  sequence predicate(x); x; endsequence
  sequence concat_zero(x); x === 2'b00; endsequence
  sequence default_two(x = a + b); x == 2'd2; endsequence
  sequence nested_two(x); two_untyped(x); endsequence
  sequence lvalue_concat(x); x == 2'b11; endsequence

  direct_sum: assert property (@(posedge clk) ((a + b) == 2'd2))
    directP++; else directF++;
  untyped_sum: assert property (@(posedge clk) two_untyped(a + b))
    untypedP++; else untypedF++;
  typed_sum: assert property (@(posedge clk) two_typed(a + b))
    typedP++; else typedF++;
  direct_xnor: assert property (@(posedge clk) ((aa ~^ bb) == 32'hffffff00))
    xnorP++; else xnorF++;
  untyped_xnor: assert property (@(posedge clk) pattern32(aa ~^ bb))
    actualP++; else actualF++;
  comparison_actual: assert property (@(posedge clk) predicate(two == zero))
    compareP++; else compareF++;
  concatenation_actual: assert property (@(posedge clk) concat_zero({1'b0, a + b}))
    concatP++; else concatF++;
  named_actual: assert property (@(posedge clk) two_untyped(.x(a + b)))
    namedP++; else namedF++;
  default_actual: assert property (@(posedge clk) default_two())
    defaultP++; else defaultF++;
  nested_actual: assert property (@(posedge clk) nested_two(a + b))
    nestedP++; else nestedF++;
  selected_actual: assert property (@(posedge clk) two_untyped(selected[1:0]))
    selectP++; else selectF++;
  concatenated_lvalue: assert property (@(posedge clk) lvalue_concat({a, b}))
    lvalueP++; else lvalueF++;

  task automatic check_case(input string label, input int passes,
                            input int failures, input bit expectPass);
    if (passes != (expectPass ? 2 : 0) || failures != (expectPass ? 0 : 2))
      $fatal(1, "%s: pass/fail=%0d/%0d", label, passes, failures);
  endtask

  initial begin
    #16;
    $display("sum direct=%0d/%0d untyped=%0d/%0d typed=%0d/%0d xnor direct=%0d/%0d actual=%0d/%0d",
             directP, directF, untypedP, untypedF, typedP, typedF,
             xnorP, xnorF, actualP, actualF);
    $display("compare=%0d/%0d concat=%0d/%0d named=%0d/%0d default=%0d/%0d nested=%0d/%0d",
             compareP, compareF, concatP, concatF, namedP, namedF,
             defaultP, defaultF, nestedP, nestedF);
    check_case("direct sum", directP, directF, 1);
    check_case("untyped sum", untypedP, untypedF, 0);
    check_case("typed sum", typedP, typedF, 1);
    check_case("direct XNOR", xnorP, xnorF, 1);
    check_case("untyped XNOR", actualP, actualF, 0);
    check_case("comparison inside actual", compareP, compareF, 0);
    check_case("concatenation inside actual", concatP, concatF, 1);
    check_case("named actual", namedP, namedF, 0);
    check_case("default actual", defaultP, defaultF, 0);
    check_case("nested actual", nestedP, nestedF, 0);
    check_case("selected variable", selectP, selectF, 1);
    check_case("concatenated variables", lvalueP, lvalueF, 1);
    $display("TEST PASSED: untyped assertion actuals preserve context boundaries");
    $finish;
  end
endmodule
