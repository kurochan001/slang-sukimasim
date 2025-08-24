module test_inline;
    initial begin
        logic [3:0] value;
        covergroup cg1;
            coverpoint value {
                bins low = {[0:3]};
                bins medium = {[4:7]};
                bins high = {[8:15]};
            }
        endgroup
        cg1 c1 = new();
    end
endmodule
