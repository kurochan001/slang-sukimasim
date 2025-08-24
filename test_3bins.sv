module test_simple;
    logic [1:0] a;
    covergroup cg;
        coverpoint a {
            bins b0 = {0};
            bins b1 = {1};
            bins b2 = {2};
        }
    endgroup
endmodule
