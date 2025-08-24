// Test for Covergroup 3+ bins limitation
module test_covergroup_3bins;
    logic [3:0] value;
    logic [2:0] data;
    logic signal;
    
    // Test 1: Simple 3 bins - This should fail with current slang
    covergroup cg1;
        coverpoint value {
            bins low    = {[0:3]};   // bin 1 - OK
            bins medium = {[4:7]};   // bin 2 - OK
            bins high   = {[8:15]};  // bin 3 - Expected to fail
        }
    endgroup
    
    // Test 2: Even more bins
    covergroup cg2;
        coverpoint data {
            bins bin0 = {0};      // bin 1
            bins bin1 = {1};      // bin 2  
            bins bin2 = {2};      // bin 3 - Expected to fail
            bins bin3 = {3};      // bin 4
            bins bin4 = {4};      // bin 5
        }
    endgroup
    
    // Test 3: Two bins only (should work)
    covergroup cg3;
        coverpoint signal {
            bins zero = {0};      // bin 1 - OK
            bins one  = {1};      // bin 2 - OK
        }
    endgroup
    
    initial begin
        cg1 c1 = new();
        cg2 c2 = new();
        cg3 c3 = new();
        
        $display("Covergroup test completed");
    end
endmodule