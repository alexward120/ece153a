// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Fri Nov 17 14:40:08 2023
// Host        : Alexander-PC running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix -prefix
//               decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix_ system_stream_grabber_0_0_stub.v
// Design      : system_stream_grabber_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a100tcsg324-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "stream_grabber,Vivado 2019.1" *)
module decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix(sys_clk, sys_rst, s_axis_stream_tdata, 
  s_axis_stream_tvalid, s_axis_stream_tready, s_axi_cpu_awaddr, s_axi_cpu_awvalid, 
  s_axi_cpu_awready, s_axi_cpu_wdata, s_axi_cpu_wvalid, s_axi_cpu_wready, s_axi_cpu_bresp, 
  s_axi_cpu_bvalid, s_axi_cpu_bready, s_axi_cpu_araddr, s_axi_cpu_arvalid, 
  s_axi_cpu_arready, s_axi_cpu_rdata, s_axi_cpu_rresp, s_axi_cpu_rvalid, s_axi_cpu_rready)
/* synthesis syn_black_box black_box_pad_pin="sys_clk,sys_rst,s_axis_stream_tdata[31:0],s_axis_stream_tvalid,s_axis_stream_tready,s_axi_cpu_awaddr[11:0],s_axi_cpu_awvalid,s_axi_cpu_awready,s_axi_cpu_wdata[31:0],s_axi_cpu_wvalid,s_axi_cpu_wready,s_axi_cpu_bresp[1:0],s_axi_cpu_bvalid,s_axi_cpu_bready,s_axi_cpu_araddr[11:0],s_axi_cpu_arvalid,s_axi_cpu_arready,s_axi_cpu_rdata[31:0],s_axi_cpu_rresp[1:0],s_axi_cpu_rvalid,s_axi_cpu_rready" */;
  input sys_clk;
  input sys_rst;
  input [31:0]s_axis_stream_tdata;
  input s_axis_stream_tvalid;
  output s_axis_stream_tready;
  input [11:0]s_axi_cpu_awaddr;
  input s_axi_cpu_awvalid;
  output s_axi_cpu_awready;
  input [31:0]s_axi_cpu_wdata;
  input s_axi_cpu_wvalid;
  output s_axi_cpu_wready;
  output [1:0]s_axi_cpu_bresp;
  output s_axi_cpu_bvalid;
  input s_axi_cpu_bready;
  input [11:0]s_axi_cpu_araddr;
  input s_axi_cpu_arvalid;
  output s_axi_cpu_arready;
  output [31:0]s_axi_cpu_rdata;
  output [1:0]s_axi_cpu_rresp;
  output s_axi_cpu_rvalid;
  input s_axi_cpu_rready;
endmodule
