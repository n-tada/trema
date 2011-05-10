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
require "trema/app"


module Trema
  describe App do
    context "when options are specified" do
      before :each do
        stanza = { :path => "/usr/bin/tetris", :name => "NAME", :options => [ "OPTION0", "OPTION1" ] }
        @app = App.new( stanza )
      end


      it "should daemonize with options" do
        @app.should_receive( :sh ).with( "/usr/bin/tetris --name NAME -d OPTION0 OPTION1" )
        @app.daemonize
      end


      it "should run with options" do
        @app.should_receive( :sh ).with( "/usr/bin/tetris --name NAME OPTION0 OPTION1" )
        @app.run
      end
    end


    context "when options are not specified" do
      before :each do
        stanza = { :path => "/usr/bin/tetris", :name => "NAME" }
        @app = App.new( stanza )
      end


      it "should daemonize without options" do
        @app.should_receive( :sh ).with( "/usr/bin/tetris --name NAME -d" )
        @app.daemonize
      end


      it "should run without options" do
        @app.should_receive( :sh ).with( "/usr/bin/tetris --name NAME" )
        @app.run
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:

