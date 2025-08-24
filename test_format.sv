module test_format;
    logic [3:0] value;
    covergroup cg1;
        coverpoint value {
            bins low    = {[0:3]};   // bin 1 - OK
            bins medium = {[4:7]};   // bin 2 - OK
            bins high   = {[8:15]};  // bin 3 - Expected to fail
        }
    endgroup
endmodule
