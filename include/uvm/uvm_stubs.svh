// UVM Stubs for slang-sukimasim
// Basic UVM infrastructure to support Property disable iff tests

package uvm_pkg;
    // UVM Message Severity levels
    typedef enum {
        UVM_NONE = 0,
        UVM_LOW = 100,
        UVM_MEDIUM = 200,
        UVM_HIGH = 300,
        UVM_FULL = 400,
        UVM_DEBUG = 500
    } uvm_verbosity;

    // Forward declarations
    typedef class uvm_object;

    // Basic UVM phase
    class uvm_phase;
        function void raise_objection(uvm_object obj);
        endfunction
        
        function void drop_objection(uvm_object obj);
        endfunction
    endclass

    // Base UVM object
    class uvm_object;
        function new(string name = "uvm_object");
        endfunction
        
        virtual function string get_full_name();
            return "stub";
        endfunction
    endclass

    // UVM Component base class
    class uvm_component extends uvm_object;
        function new(string name, uvm_component parent = null);
            super.new(name);
        endfunction
        
        virtual function void connect_phase(uvm_phase phase);
        endfunction
        
        virtual task run_phase(uvm_phase phase);
        endtask
    endclass

    // UVM Environment base class
    class uvm_env extends uvm_component;
        function new(string name, uvm_component parent = null);
            super.new(name, parent);
        endfunction
    endclass

    // Resource database stub
    class uvm_resource_db #(type T = uvm_object);
        static function bit read_by_name(string scope, string name, ref T val);
            return 1'b0;  // Stub implementation
        endfunction
        
        static function void set(string scope, string name, T val);
        endfunction
    endclass

    // Basic test runner
    function void run_test(string test_name = "");
        // Stub implementation
    endfunction
    
endpackage: uvm_pkg