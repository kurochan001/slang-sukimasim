// Test DPI-C export functionality
module test_dpi_export;

  // Simple function export
  export "DPI-C" function add_numbers;
  function int add_numbers(input int a, input int b);
    return a + b;
  endfunction

  // Task export
  export "DPI-C" task print_message;
  task print_message(input string msg);
    $display("DPI Message: %s", msg);
  endtask

  // Export with C identifier
  export "DPI-C" c_multiply = function multiply;
  function int multiply(input int a, input int b);
    return a * b;
  endfunction

  // Test invalid cases (should generate errors)
  // export "DPI-C" function nonexistent; // Error: function doesn't exist
  
  initial begin
    int result;
    
    // Test the functions locally
    result = add_numbers(5, 3);
    $display("add_numbers(5, 3) = %d", result);
    
    print_message("Hello from SystemVerilog");
    
    result = multiply(4, 7);
    $display("multiply(4, 7) = %d", result);
    
    $display("DPI-C export test completed");
    $finish;
  end

endmodule