module test_simple;
    logic a;
    covergroup cg;
        coverpoint a {
            bins b0 = {0};
            bins b1 = {1};
        }
    endgroup
endmodule
