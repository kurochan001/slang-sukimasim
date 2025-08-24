module test_mix;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins a = {0};
            bins b = {[1:3]};
            bins c = {4};
        }
    endgroup
endmodule
