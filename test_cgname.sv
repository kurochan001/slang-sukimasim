module test_cgname;
    logic [3:0] value;
    covergroup cg;  // Changed from cg1 to cg
        coverpoint value {
            bins low = {[0:3]};
            bins medium = {[4:7]};
            bins high = {[8:15]};
        }
    endgroup
endmodule
