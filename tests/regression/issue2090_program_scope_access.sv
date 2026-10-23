// Expected errors: IEEE 1800-2023 24.3 and 24.5 program scope restrictions.
program;
    function automatic int get_level();
        return top.p.count;
    endfunction
endprogram
program p_t;
    int count = 3;
    task report(int v); $display("%0d", v); endtask
    initial $display("%0d", get_level());
endprogram
module top;
    p_t p();
    initial $display("%0d", p.count);
    initial p.report(5);
endmodule
