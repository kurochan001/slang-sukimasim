// Test token pasting with declared identifier
`define append(f) f``_master

module top ();
    reg clock_master; // Declare the identifier
    initial $display(`append(clock));
endmodule