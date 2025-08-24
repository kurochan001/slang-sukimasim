module test_simple_ranges;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins a = {0};
            bins b = {1};
            bins c = {2};
        }
    endgroup
endmodule
