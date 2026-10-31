class width_leaf #(type T = byte);
    T value;
endclass

class dynamic_payload;
    string text;
    width_leaf #(byte) child;
endclass

class fixed_payload;
    byte first;
    byte second;
endclass

module class_payload_width_control;
    localparam int WIDTH = $bits(fixed_payload);
    typedef bit [15:0] packed_t;
    fixed_payload fixed_object;
    dynamic_payload dynamic_object;
    packed_t packed_value;
    initial begin
        if (WIDTH != 16)
            $fatal(1, "FIXED_CLASS_WIDTH_FAIL");
        packed_value = packed_t'(fixed_object);
    end
endmodule
