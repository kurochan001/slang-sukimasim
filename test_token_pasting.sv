// Test token pasting with undefined identifier
`define append(f) f``_master

module top ();
// The challenge: clock_master is undefined but should be allowed during parsing
initial $display(`append(clock));
endmodule