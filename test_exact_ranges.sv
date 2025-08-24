module test_exact_ranges;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins a = {[0:3]};
            bins b = {[4:7]};
            bins c = {[8:15]};
        }
    endgroup
endmodule
