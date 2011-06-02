#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


require File.join( File.dirname( __FILE__ ), "spec_helper" )
require "trema"


include Trema


class RepeaterHub < Controller
  def packet_in message
    send_flow_mod_add(
      message.datapath_id,
      :match => Match.from( message ),
      :buffer_id => message.buffer_id,
      :actions => ActionOutput.new( OFPP_FLOOD )
    )
    if not message.buffered?
      send_packet_out(
        message.datapath_id,
        message.buffer_id,
        message.in_port,
        ActionOutput.new( OFPP_FLOOD ),
        message.data
      )
    end
  end
end


describe RepeaterHub do
  around do | example |
    trema_conf {
      vswitch("repeater_hub") { datapath_id "0xabc" }

      vhost("host1") { promisc "on" }
      vhost("host2") { promisc "on" }
      vhost("host3") { promisc "on" }

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
    }
    trema_run RepeaterHub

    example.run

    trema_kill
  end
  

  context "when host1 sends one packet to host2" do
    it "should receive #packet_in" do
      controller( "RepeaterHub" ).should_receive( :packet_in )

      send_packets "host1", "host2"
    end


    it "should send #flow_mod_add" do
      controller( "RepeaterHub" ).should_receive( :send_flow_mod_add )

      send_packets "host1", "host2"
    end


    describe "repeater_hub switch" do
      before { send_packets "host1", "host2" }

      subject { switch( "repeater_hub" ) }

      it { should have( 1 ).flows }

      describe "its flow actions" do
        subject { switch( "repeater_hub" ).flows[ 0 ].actions }

        it { should == "FLOOD" }
      end
    end
  end


  context "when host1 sends 100 packets to host2" do
    before { send_packets "host1", "host2", :pps => 100 }

    describe "host2" do
      it "should receive 100 packets" do
        host( "host2" ).rx_stats.n_pkts.should == 100
      end
    end

    describe "host3" do
      it "should receive 100 packets" do
        host( "host3" ).rx_stats.n_pkts.should == 100
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
