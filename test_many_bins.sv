module test_many_bins;
    logic [3:0] value;
    covergroup my_cg;
        coverpoint value {
            bins bin0 = {0};
            bins bin1 = {1};
            bins bin2 = {2};
            bins bin3 = {3};
            bins bin4 = {4};
            bins bin5 = {5};
            bins bin6 = {6};
            bins bin7 = {7};
            bins bin8 = {8};
            bins bin9 = {9};
            bins bin10 = {10};
        }
    endgroup
endmodule
