Feature: Send echo request messages

  As a Trema user
  I want to send echo request messages to openflow switches
  So that I can receive echo reply messages from openflow switches


  Scenario: Send echo request x 10
    When I try trema run "./objects/examples/openflow_message/echo_request 10" with following configuration (backgrounded):
      """
      vswitch("echo_request") { datapath_id "0xabc" }
      """
      And wait until "echo_request" is up
    Then the log file "openflowd.echo_request.log" should include "received: echo_request" x 10
