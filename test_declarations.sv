// Test for DeclarationsAtStart fix
module test_declarations;
    initial begin
        $display("First statement");
        int x = 5;  // Declaration after statement (was error before)
        $display("x = %d", x);
        real y = 3.14;  // Another declaration after statement
        $display("y = %f", y);
        
        // Test nested blocks
        begin
            $display("Nested block");
            logic [7:0] data = 8'hAB;  // Declaration after statement in nested block
            $display("data = %h", data);
        end
    end
endmodule

// Test for IEEE 1800-2023 coverage functions
module test_coverage_functions;
    initial begin
        $assertcoveredon();  // New IEEE 1800-2023 function
        $display("Coverage enabled");
        
        // Do some work
        for (int i = 0; i < 10; i++) begin
            $display("i = %d", i);
            int square = i * i;  // Declaration inside loop after statement
            $display("square = %d", square);
        end
        
        $assertcoverageoff();  // New IEEE 1800-2023 function
        $display("Coverage disabled");
        
        // These functions should now be recognized
        int coverage_status = $assertion_coverage();
        real coverage_percent = $get_assertion_coverage();
        $display("Coverage status: %d, percent: %f", coverage_status, coverage_percent);
    end
endmodule