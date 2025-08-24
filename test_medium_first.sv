module test_medium_first;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins medium = {[4:7]};
            bins low = {[0:3]};
            bins high = {[8:15]};
        }
    endgroup
endmodule
