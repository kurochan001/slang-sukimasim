module tb;
  int v;
  
  covergroup cg;
    coverpoint v {
      option.auto_bin_max = 4;
      bins auto_bins[] = {[0:15]};  // Should create 4 bins: [0:3], [4:7], [8:11], [12:15]
    }
  endgroup
  
  cg c = new;
  
  initial begin
    // Test each bin
    $display("Testing auto_bin_max splitting...");
    
    // Bin 1: [0:3]
    v = 0;  c.sample(); $display("Sampled v=%0d (should be in bin [0:3])", v);
    v = 3;  c.sample(); $display("Sampled v=%0d (should be in bin [0:3])", v);
    
    // Bin 2: [4:7]
    v = 4;  c.sample(); $display("Sampled v=%0d (should be in bin [4:7])", v);
    v = 7;  c.sample(); $display("Sampled v=%0d (should be in bin [4:7])", v);
    
    // Bin 3: [8:11]
    v = 8;  c.sample(); $display("Sampled v=%0d (should be in bin [8:11])", v);
    v = 11; c.sample(); $display("Sampled v=%0d (should be in bin [8:11])", v);
    
    // Bin 4: [12:15]
    v = 12; c.sample(); $display("Sampled v=%0d (should be in bin [12:15])", v);
    v = 15; c.sample(); $display("Sampled v=%0d (should be in bin [12:15])", v);
    
    $display("Test completed - all bins tested");
    $finish;
  end
endmodule