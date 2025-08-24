// Simple test for FCov initial block issue
module test_fcov_simple;
    initial begin
        // IEEE 1800-2023 function that caused "Invalid initial block"
        $assertcoveredon();
        
        // Now we can have declarations after statements
        $display("Test started");
        int counter = 0;
        
        // More statements and declarations mixed
        $display("Counter initialized");
        real value = 1.5;
        
        $display("Counter = %d, Value = %f", counter, value);
        
        $assertcoverageoff();
        $display("Test completed");
    end
endmodule