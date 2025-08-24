module test_ranges3;
    logic [3:0] value;
    covergroup cg;
        coverpoint value {
            bins low = {[0:3]};
            bins mid = {[4:7]};
            bins high = {[8:15]};
        }
    endgroup
endmodule
