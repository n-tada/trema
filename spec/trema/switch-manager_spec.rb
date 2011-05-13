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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema/switch-manager"


module Trema
  describe SwitchManager do
    it "should run switch_manager command with proper options" do
      switch_manager =
        SwitchManager.new( :port_status => "PORT_STATUS_HANDLER", :packet_in => "PACKETIN_HANDLER", :state_notify => "STATENOTIFY_HANDLER" )
      switch_manager.should_receive( :sh ).once.with( /switch_manager \-\-daemonize \-\- port_status::PORT_STATUS_HANDLER packet_in::PACKETIN_HANDLER state_notify::STATENOTIFY_HANDLER$/ )

      switch_manager.run
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
