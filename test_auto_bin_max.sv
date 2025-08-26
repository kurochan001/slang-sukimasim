module tb;
  int v;
  
  covergroup cg;
    coverpoint v {
      option.auto_bin_max = 4;
      bins auto_bins[] = {[0:15]};
    }
  endgroup
  
  cg c = new;
  
  initial begin
    v = 0;  c.sample();
    v = 4;  c.sample();
    v = 8;  c.sample();
    v = 12; c.sample();
    $display("Test completed");
    $finish;
  end
endmodule