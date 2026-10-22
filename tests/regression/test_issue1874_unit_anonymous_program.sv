// IEEE 1800-2023 §24.6 / §3.12.1: use caller permissions for $unit lookup.
program;
    function automatic int twice1874(int x);
        return 2 * x;
    endfunction
endprogram
program test_issue1874_unit_anonymous_program;
    function automatic int twice1874(int x);
        return 99;
    endfunction
    initial begin
        if ($unit::twice1874(21) != 42 || twice1874(21) != 99)
            $fatal(1, "unit anonymous program scope mismatch");
    end
endprogram
