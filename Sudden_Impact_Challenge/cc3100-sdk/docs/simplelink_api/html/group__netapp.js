var group__netapp =
[
    [ "SlPingReport_t", "group__netapp.html#struct_sl_ping_report__t", [
      [ "AvgRoundTime", "group__netapp.html#a5d610bdde4039525984224ee0f35b6d9", null ],
      [ "MaxRoundTime", "group__netapp.html#abb8b750ec3b99bcac2b1a84c611c6d2a", null ],
      [ "MinRoundTime", "group__netapp.html#a10437a22b3604a1fbe4cb91bd711077f", null ],
      [ "PacketsReceived", "group__netapp.html#ac8e9ea566ef6bd2acb54a6af9677486a", null ],
      [ "PacketsSent", "group__netapp.html#a92813cdd58af8c15929270ccf61b26a2", null ],
      [ "TestTime", "group__netapp.html#af7da9681c2b355a3261a476f353bfe93", null ]
    ] ],
    [ "SlPingStartCommand_t", "group__netapp.html#struct_sl_ping_start_command__t", [
      [ "Flags", "group__netapp.html#abc512772e263735c06e9072f94c92952", null ],
      [ "Ip", "group__netapp.html#a16ea88b3a07d38c6e26af86e4d9e04aa", null ],
      [ "Ip1OrPaadding", "group__netapp.html#aba8221b8c6c4e83f843e7cf2b63ec649", null ],
      [ "Ip2OrPaadding", "group__netapp.html#a5e17ea66dfb99beb88bd64c847954444", null ],
      [ "Ip3OrPaadding", "group__netapp.html#af95d2469f8867016b655574752e3f905", null ],
      [ "PingIntervalTime", "group__netapp.html#a0b36eb660aeeb08cc0e280ee657b7192", null ],
      [ "PingRequestTimeout", "group__netapp.html#a15c82a992940a12e86afd8b3b34436ed", null ],
      [ "PingSize", "group__netapp.html#a4323564c51aa12dd8930e39566fd8625", null ],
      [ "TotalNumberOfAttempts", "group__netapp.html#a994b4b990c3ea36ca117cf3fd0943f0d", null ]
    ] ],
    [ "_slHttpServerString_t", "group__netapp.html#struct__sl_http_server_string__t", [
      [ "data", "group__netapp.html#ae2f1a2294cac364e7901e04f5821ede8", null ],
      [ "len", "group__netapp.html#acc9fcad2930f408ce8147134702a4ff3", null ]
    ] ],
    [ "_slHttpServerData_t", "group__netapp.html#struct__sl_http_server_data__t", [
      [ "name_len", "group__netapp.html#a295865b2d6f6091bc35d8de6e8cae731", null ],
      [ "token_name", "group__netapp.html#adff3bc96f451530f4776b011f36a3c2a", null ],
      [ "token_value", "group__netapp.html#a2283b20c750664ec3b4301cb8e64f198", null ],
      [ "value_len", "group__netapp.html#ae6d3f66fb64a31cc12909a1ec028387b", null ]
    ] ],
    [ "_slHttpServerPostData_t", "group__netapp.html#struct__sl_http_server_post_data__t", [
      [ "action", "group__netapp.html#a76122764e78b080056125b9ef28ada7b", null ],
      [ "token_name", "group__netapp.html#ac32e60a343288e97e368ae665adf7442", null ],
      [ "token_value", "group__netapp.html#ac17cbf485a7c72aa811cfda94d8649c6", null ]
    ] ],
    [ "SlHttpServerEventData_u", "group__netapp.html#union_sl_http_server_event_data__u", [
      [ "httpPostData", "group__netapp.html#a4c697e1d747be7f899ab3593a57fb575", null ],
      [ "httpTokenName", "group__netapp.html#afe08fe6236a178fb1bd225b511f64ab9", null ]
    ] ],
    [ "SlHttpServerResponsedata_u", "group__netapp.html#union_sl_http_server_responsedata__u", [
      [ "token_value", "group__netapp.html#ac17cbf485a7c72aa811cfda94d8649c6", null ]
    ] ],
    [ "SlHttpServerEvent_t", "group__netapp.html#struct_sl_http_server_event__t", [
      [ "Event", "group__netapp.html#adeedbaaa252b969fc66e151eef37ea62", null ],
      [ "EventData", "group__netapp.html#aea6d012a43dcb8ded6b90686ceaef0f7", null ]
    ] ],
    [ "SlHttpServerResponse_t", "group__netapp.html#struct_sl_http_server_response__t", [
      [ "Response", "group__netapp.html#acc4e0dc6756b696c4e2bbdd3f75d1123", null ],
      [ "ResponseData", "group__netapp.html#a79b6c5114e9f6da69c3113d4be87a943", null ]
    ] ],
    [ "SlNetAppDhcpServerBasicOpt_t", "group__netapp.html#struct_sl_net_app_dhcp_server_basic_opt__t", [
      [ "ipv4_addr_last", "group__netapp.html#a3658ee49e477ac75294c4dcb44e9469b", null ],
      [ "ipv4_addr_start", "group__netapp.html#ad89c28578421c014b62f5edd796760b1", null ],
      [ "lease_time", "group__netapp.html#ae870c09512e5404d8fd6a94d899d52b5", null ]
    ] ],
    [ "SlNetAppGetShortServiceIpv4List_t", "group__netapp.html#struct_sl_net_app_get_short_service_ipv4_list__t", [
      [ "Reserved", "group__netapp.html#a3f5363b14f728fe990328585ccbc70e1", null ],
      [ "service_ipv4", "group__netapp.html#a1a2075c35d52286cb696f878738d30be", null ],
      [ "service_port", "group__netapp.html#a526174fd4b7f339328e315dbb01c19f7", null ]
    ] ],
    [ "SlNetAppGetFullServiceIpv4List_t", "group__netapp.html#struct_sl_net_app_get_full_service_ipv4_list__t", [
      [ "Reserved", "group__netapp.html#a3f5363b14f728fe990328585ccbc70e1", null ],
      [ "service_host", "group__netapp.html#aeb85c9d6321692e2622077406052c2c2", null ],
      [ "service_ipv4", "group__netapp.html#a1a2075c35d52286cb696f878738d30be", null ],
      [ "service_name", "group__netapp.html#a14a68e63be446395d1bdc960d9cd46bb", null ],
      [ "service_port", "group__netapp.html#a526174fd4b7f339328e315dbb01c19f7", null ]
    ] ],
    [ "SlNetAppGetFullServiceWithTextIpv4List_t", "group__netapp.html#struct_sl_net_app_get_full_service_with_text_ipv4_list__t", [
      [ "Reserved", "group__netapp.html#a3f5363b14f728fe990328585ccbc70e1", null ],
      [ "service_host", "group__netapp.html#aeb85c9d6321692e2622077406052c2c2", null ],
      [ "service_ipv4", "group__netapp.html#a1a2075c35d52286cb696f878738d30be", null ],
      [ "service_name", "group__netapp.html#a14a68e63be446395d1bdc960d9cd46bb", null ],
      [ "service_port", "group__netapp.html#a526174fd4b7f339328e315dbb01c19f7", null ],
      [ "service_text", "group__netapp.html#a6b9cac633579a01d5f6e03c9a3dde9e0", null ]
    ] ],
    [ "SlNetAppServiceAdvertiseTimingParameters_t", "group__netapp.html#struct_sl_net_app_service_advertise_timing_parameters__t", [
      [ "k", "group__netapp.html#a788ae2e5459feba3d646ffa71980f6db", null ],
      [ "max_time", "group__netapp.html#a1cd71a7ab31d4352d718a5707d3a969b", null ],
      [ "Maxinterval", "group__netapp.html#a9f1818d1132e3399c3f8a79e453d5b56", null ],
      [ "p", "group__netapp.html#a010529614c86bbaa26654186b52b800d", null ],
      [ "RetransInterval", "group__netapp.html#a5f05e30c41182b96d2c4691cfd75f138", null ],
      [ "t", "group__netapp.html#a95b98eb3e4b7b7e456cc4ebfe9282536", null ]
    ] ]
];