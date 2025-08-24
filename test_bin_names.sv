module test_bin_names;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins low = {[0:3]};
            bins medium = {[4:7]};
            bins high = {[8:15]};
        }
    endgroup
endmodule
