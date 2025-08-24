module test_ranges;
    logic [3:0] value;
    covergroup cg;
        coverpoint value {
            bins low = {[0:3]};
            bins high = {[4:7]};
        }
    endgroup
endmodule
