class test_class;
    logic [3:0] value;
    covergroup cg1;
        coverpoint value {
            bins low = {[0:3]};
            bins medium = {[4:7]};
            bins high = {[8:15]};
        }
    endgroup
endclass
