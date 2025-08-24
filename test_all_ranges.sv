module test_all_ranges;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins a = {[0:1]};
            bins b = {[2:3]};
            bins c = {[4:5]};
        }
    endgroup
endmodule
