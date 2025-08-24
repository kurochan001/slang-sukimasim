module test_other_names;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins low = {[0:3]};
            bins mid = {[4:7]};   // Changed from "medium" to "mid"
            bins high = {[8:15]};
        }
    endgroup
endmodule
