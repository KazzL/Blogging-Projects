var group__wlan =
[
    [ "slSmartConfigStartAsyncResponse_t", "group__wlan.html#structsl_smart_config_start_async_response__t", [
      [ "private_token", "group__wlan.html#ae788e671c21e21c7913ac4b439887785", null ],
      [ "private_token_len", "group__wlan.html#aac6bab1ba54c9d01c548a5971fe18a95", null ],
      [ "ssid", "group__wlan.html#aaf9ed7d9e9d6c2bdd7b7fc7b768b81de", null ],
      [ "ssid_len", "group__wlan.html#a4fd951e04acb1b6941b85533d248ba27", null ],
      [ "status", "group__wlan.html#a9bd457bdee1c8059b6cf88ac0647d0e1", null ]
    ] ],
    [ "slSmartConfigStopAsyncResponse_t", "group__wlan.html#structsl_smart_config_stop_async_response__t", [
      [ "padding", "group__wlan.html#aee74651e918d4c23f3eabe25fbbf8142", null ],
      [ "status", "group__wlan.html#a0fcf01673166445f62de27571ae41090", null ]
    ] ],
    [ "slWlanConnFailureAsyncResponse_t", "group__wlan.html#structsl_wlan_conn_failure_async_response__t", [
      [ "padding", "group__wlan.html#aee74651e918d4c23f3eabe25fbbf8142", null ],
      [ "status", "group__wlan.html#a0fcf01673166445f62de27571ae41090", null ]
    ] ],
    [ "slWlanConnectAsyncResponse_t", "group__wlan.html#structsl_wlan_connect_async_response__t", [
      [ "bssid", "group__wlan.html#a26584d70e5f1888864c8c3f3d43e21c7", null ],
      [ "connection_type", "group__wlan.html#a02de4ebfc0ae3ef0524521fea6cdba7a", null ],
      [ "go_peer_device_name", "group__wlan.html#a134cf4c828e548efdc5febe2b8b826ae", null ],
      [ "go_peer_device_name_len", "group__wlan.html#ad58b0d3c676c06221fa491ec4b384cad", null ],
      [ "padding", "group__wlan.html#a591a340cf7a3bc8395dc554fc01910b7", null ],
      [ "reason_code", "group__wlan.html#aef3e37e4643200170981d36287a9bbf5", null ],
      [ "ssid_len", "group__wlan.html#a2e45fb530d9c89d3673ea6e05f07844d", null ],
      [ "ssid_name", "group__wlan.html#a89e1ad338d707b4182bd0a8bf5a15bc6", null ]
    ] ],
    [ "slPeerInfoAsyncResponse_t", "group__wlan.html#structsl_peer_info_async_response__t", [
      [ "go_peer_device_name", "group__wlan.html#a134cf4c828e548efdc5febe2b8b826ae", null ],
      [ "go_peer_device_name_len", "group__wlan.html#ad58b0d3c676c06221fa491ec4b384cad", null ],
      [ "mac", "group__wlan.html#a51fa48efb76fa1995446db52ac06a46f", null ],
      [ "own_ssid", "group__wlan.html#af3d92d2c6ec560caf5c705fa487d9622", null ],
      [ "own_ssid_len", "group__wlan.html#a8e4b7daa24f45b8112386655cc2c5c61", null ],
      [ "padding", "group__wlan.html#a46d3c053c50ca746d761db91f590bb60", null ],
      [ "wps_dev_password_id", "group__wlan.html#a3b1181843fd7fc28edccb34fe78934cc", null ]
    ] ],
    [ "SlWlanEventData_u", "group__wlan.html#union_sl_wlan_event_data__u", [
      [ "APModeStaConnected", "group__wlan.html#a8a4b774d3fea5bd36d46e36326f11ed4", null ],
      [ "APModestaDisconnected", "group__wlan.html#a5e285baad857f73a0167f73e4b17ae50", null ],
      [ "P2PModeDevFound", "group__wlan.html#a8b490ba7a54396f6e289b5789644de5f", null ],
      [ "P2PModeNegReqReceived", "group__wlan.html#aee4d526e0489f6384555d71c6a4c2ff3", null ],
      [ "P2PModewlanConnectionFailure", "group__wlan.html#a2211e22e568ee4349eebd7ed5b6f9b60", null ],
      [ "smartConfigStartResponse", "group__wlan.html#a3e0a9eb580163ca72c178d96a5edba79", null ],
      [ "smartConfigStopResponse", "group__wlan.html#a962b4e0845dd71daaf9929aea34183fc", null ],
      [ "STAandP2PModeDisconnected", "group__wlan.html#abf61e2e1e81c5eda10f6ff84a7197b31", null ],
      [ "STAandP2PModeWlanConnected", "group__wlan.html#a406b4e8670796f9cdc1746646fab6588", null ]
    ] ],
    [ "SlWlanEvent_t", "group__wlan.html#struct_sl_wlan_event__t", [
      [ "Event", "group__wlan.html#adeedbaaa252b969fc66e151eef37ea62", null ],
      [ "EventData", "group__wlan.html#ae587c51197255d4e4ef20cc90d73825f", null ]
    ] ],
    [ "SlGetRxStatResponse_t", "group__wlan.html#struct_sl_get_rx_stat_response__t", [
      [ "AvarageDataCtrlRssi", "group__wlan.html#af3660978617c803a996f041215004a50", null ],
      [ "AvarageMgMntRssi", "group__wlan.html#a2050deb55011de7a472c8d5bcd6099f7", null ],
      [ "GetTimeStamp", "group__wlan.html#a85c0324d0b3de70a327e185173309a5c", null ],
      [ "RateHistogram", "group__wlan.html#ad18c6011a86553bd786ffae0f7b87538", null ],
      [ "ReceivedFcsErrorPacketsNumber", "group__wlan.html#a6e06f9a2b719028a084d2fc2333cd309", null ],
      [ "ReceivedPlcpErrorPacketsNumber", "group__wlan.html#ae0b83e8cbdae8b17ae333f5684bd5708", null ],
      [ "ReceivedValidPacketsNumber", "group__wlan.html#a0b84e628b2501fbf4a814b2f678a63ef", null ],
      [ "RssiHistogram", "group__wlan.html#a11b977458ccb1ceade91d3f2ae733307", null ],
      [ "StartTimeStamp", "group__wlan.html#ac40e3ad339a01bee80c412d99eb1a128", null ]
    ] ],
    [ "Sl_WlanNetworkEntry_t", "group__wlan.html#struct_sl___wlan_network_entry__t", [
      [ "bssid", "group__wlan.html#a1625b9ac8d0d51aa89df83295c9a5de2", null ],
      [ "reserved", "group__wlan.html#aa1d4d52e9a684f2a28c9c8b89573af18", null ],
      [ "rssi", "group__wlan.html#a80c3df13ed7cf0b1a5e5639811c82f34", null ],
      [ "sec_type", "group__wlan.html#af791c098aa0b08d0ee0126843b1fe855", null ],
      [ "ssid", "group__wlan.html#ad097bad1e9bb0c4e70de8748263b76da", null ],
      [ "ssid_len", "group__wlan.html#a2e45fb530d9c89d3673ea6e05f07844d", null ]
    ] ],
    [ "SlSecParams_t", "group__wlan.html#struct_sl_sec_params__t", [
      [ "Key", "group__wlan.html#af943e0f6d7ca78a5b795c8da294c5d1e", null ],
      [ "KeyLen", "group__wlan.html#a9b049837934488d32481cf8d616e12af", null ],
      [ "Type", "group__wlan.html#a1d58ad89ed5b340d15c354b769f8ecc2", null ]
    ] ],
    [ "SlSecParamsExt_t", "group__wlan.html#struct_sl_sec_params_ext__t", [
      [ "AnonUser", "group__wlan.html#a882de2171333187a5cded2550accce18", null ],
      [ "AnonUserLen", "group__wlan.html#ac62b273b2e1b1c60eca7ef61a29b0aa5", null ],
      [ "CertIndex", "group__wlan.html#a55a132bbb3126099cb8f12cb6d174876", null ],
      [ "EapMethod", "group__wlan.html#a4f18f173d08eff5ae05fa940c60df4c0", null ],
      [ "User", "group__wlan.html#afc4f533307e2aee2c6c114f8941aa499", null ],
      [ "UserLen", "group__wlan.html#a24af689142eda26860754c5e9c0f9e60", null ]
    ] ],
    [ "SlGetSecParamsExt_t", "group__wlan.html#struct_sl_get_sec_params_ext__t", [
      [ "AnonUser", "group__wlan.html#a402a97b0e8257ae2f2b928c7590d0b03", null ],
      [ "AnonUserLen", "group__wlan.html#ac62b273b2e1b1c60eca7ef61a29b0aa5", null ],
      [ "CertIndex", "group__wlan.html#a55a132bbb3126099cb8f12cb6d174876", null ],
      [ "EapMethod", "group__wlan.html#a4f18f173d08eff5ae05fa940c60df4c0", null ],
      [ "User", "group__wlan.html#ac3b3c772e1d98758cc674e2f6f0658d5", null ],
      [ "UserLen", "group__wlan.html#a24af689142eda26860754c5e9c0f9e60", null ]
    ] ],
    [ "slWlanScanParamCommand_t", "group__wlan.html#structsl_wlan_scan_param_command__t", [
      [ "G_Channels_mask", "group__wlan.html#a1401545f73ec1aeb0f1caff176a49877", null ],
      [ "rssiThershold", "group__wlan.html#a12b902a2708b47b806a727a2604f9c4f", null ]
    ] ],
    [ "sl_protocol_InfoElement_t", "group__wlan.html#structsl__protocol___info_element__t", [
      [ "data", "group__wlan.html#a27e0ca8e8af28ff69c2df264599f5625", null ],
      [ "id", "group__wlan.html#a2888afcbf466934238f060ea0b8300ed", null ],
      [ "length", "group__wlan.html#a128a630f6d2121a0106add0f03f1cab9", null ],
      [ "oui", "group__wlan.html#aa7158dccffcbe48160a5c4ac033466ff", null ]
    ] ],
    [ "sl_protocol_WlanSetInfoElement_t", "group__wlan.html#structsl__protocol___wlan_set_info_element__t", [
      [ "ie", "group__wlan.html#ac1029492d0e3b06663e3dfa879773e2a", null ],
      [ "index", "group__wlan.html#a1b7d00023fd5674c4bd44bc179294390", null ],
      [ "role", "group__wlan.html#a838542fa5c0baf0d55b638d8906ec18e", null ]
    ] ]
];