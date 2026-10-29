// IEEE 1800-2023 6.7.1 permits typeless interconnect packed dimensions.
module issue2322_sink(inout wand [3:0] w);
endmodule
module issue2322_top;
    interconnect [3:0] packed_net;
    interconnect unpacked_net[4];
    issue2322_sink sink(packed_net);
endmodule
