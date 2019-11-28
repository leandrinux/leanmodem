#include "Arduino.h"
#include "ping.h"
#include <Pinger.h>
extern "C" {
  #include <lwip/icmp.h> // needed for icmp packet definitions
}


Stream *ping_stream;

void ping(Stream *stream, char *host) {
  ping_stream = stream;
  Pinger pinger;
  pinger.OnReceive([](const PingerResponse& response) {
    if (response.ReceivedResponse) {
      ping_stream->printf(
        "Reply from %s: bytes=%d time=%lums TTL=%d\r\n",
        response.DestIPAddress.toString().c_str(),
        response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
        response.ResponseTime,
        response.TimeToLive);
    } else {
      ping_stream->printf("Request timed out.\r\n");
    }
    return true;
  });
  
  pinger.OnEnd([](const PingerResponse& response) {
    float loss = 100;
    if(response.TotalReceivedResponses > 0) {
      loss = (response.TotalSentRequests - response.TotalReceivedResponses) * 100 / response.TotalSentRequests;
    }
    ping_stream->printf(
      "Ping statistics for %s:\r\n",
      response.DestIPAddress.toString().c_str());
    ping_stream->printf(
      "    Packets: Sent = %lu, Received = %lu, Lost = %lu (%.2f%% loss),\r\n",
      response.TotalSentRequests,
      response.TotalReceivedResponses,
      response.TotalSentRequests - response.TotalReceivedResponses,
      loss);
    if(response.TotalReceivedResponses > 0) {
      ping_stream->printf("Approximate round trip times in milli-seconds:\r\n");
      ping_stream->printf(
        "    Minimum = %lums, Maximum = %lums, Average = %.2fms\r\n",
        response.MinResponseTime,
        response.MaxResponseTime,
        response.AvgResponseTime);
    }
    ping_stream->printf("Destination host data:\r\n");
    ping_stream->printf(
      "    IP address: %s\r\n",
      response.DestIPAddress.toString().c_str());
    if(response.DestMacAddress != nullptr) {
      ping_stream->printf(
        "    MAC address: " MACSTR "\r\n",
        MAC2STR(response.DestMacAddress->addr));
    }
    if(response.DestHostname != "") {
      ping_stream->printf(
        "    DNS name: %s\r\n",
        response.DestHostname.c_str());
    }
    return true;
  });
  
  ping_stream->printf("Pinging %s\r\n", host);
  if(pinger.Ping(host) == false) {
    ping_stream->println("Error during last ping command.\r\n");
  }
  delay(10000);
}
