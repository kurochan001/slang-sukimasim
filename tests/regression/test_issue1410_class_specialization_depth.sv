// IEEE 1800-2023 §8.25: matching specializations represent the same type.
// The depth bound is an implementation resource limit, not an LRM prohibition.
`timescale 1ns/1ns
`ifdef ISSUE1410_SELF
  `define ISSUE1410_NEXT(N) (N)
`elsif ISSUE1410_UNBOUNDED
  `define ISSUE1410_NEXT(N) ((N) - 1)
`else
  `define ISSUE1410_NEXT(N) ((N) > 0 ? (N) - 1 : 0)
`endif

class issue1410_node #(int N = 64);
`ifdef ISSUE1410_MEMBER
    issue1410_node #(`ISSUE1410_NEXT(N)) child;
`elsif ISSUE1410_STATIC
    static issue1410_node #(`ISSUE1410_NEXT(N)) child;
`elsif ISSUE1410_TYPEDEF
    typedef issue1410_node #(`ISSUE1410_NEXT(N)) next_t;
`else
    function issue1410_node #(`ISSUE1410_NEXT(N)) next();
        return null;
    endfunction
`endif
endclass

`ifdef ISSUE1410_WIDE
class issue1410_leaf #(int N = 0);
    int value;
endclass
`endif

module test_issue1410_class_specialization_depth;
    issue1410_node #(64) node;
`ifdef ISSUE1410_WIDE
    for (genvar i = 0; i < 300; i++) begin : leaves
        issue1410_leaf #(i) leaf;
    end
`endif
    initial begin
`ifdef ISSUE1410_UNBOUNDED
        $fatal(1, "FAILED: unbounded specialization reached simulation");
`else
        $display("TEST PASSED: issue1410 finite specialization");
        $finish;
`endif
    end
endmodule

`undef ISSUE1410_NEXT
