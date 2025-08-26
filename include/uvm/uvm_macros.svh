// UVM Macros Stubs for slang-sukimasim
// Basic UVM macro implementations

`ifndef UVM_MACROS_SVH
`define UVM_MACROS_SVH

// UVM Info macro stub
`define uvm_info(ID, MSG, VERBOSITY) \
    $display("[UVM_INFO] %s: %s", ID, MSG)

// UVM Warning macro stub  
`define uvm_warning(ID, MSG) \
    $display("[UVM_WARNING] %s: %s", ID, MSG)

// UVM Error macro stub
`define uvm_error(ID, MSG) \
    $display("[UVM_ERROR] %s: %s", ID, MSG)

// UVM Fatal macro stub
`define uvm_fatal(ID, MSG) \
    $display("[UVM_FATAL] %s: %s", ID, MSG)

`endif // UVM_MACROS_SVH