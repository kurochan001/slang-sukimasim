module test_keywords;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins small = {[0:3]};   // small is also a keyword
            bins medium = {[4:7]};  // medium is a keyword
            bins large = {[8:15]};  // large is also a keyword
        }
    endgroup
endmodule
