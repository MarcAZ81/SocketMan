#include <unistd.h>
#include <resolv.h>
#include <netinet/tcp.h>
#include <time.h>
#include "monitor.h"
#include <curl/curl.h>
#include <json-c/json.h>
#include "options.h"
#include "network.h"
#include "system.h"
#include "helper.h"
#include "ct_iw.h"
#include "dbg.h"

time_t last_collect;
/* int wait = 30; */
int hb_interval = 60;
int collected;

int should_collect() {
  time_t now = time(NULL);
  int diff = now - last_collect;
  if (diff >= hb_interval) {
    last_collect = 0;
    return 1;
  } else {
    debug("Running now for %d seconds", diff);
  }
  return 0;
}

int post(json_object *json) {

  /* char url[strlen(options.url) + strlen(options.mac) + strlen(options.token) +16]; */

  /* strcpy(url, options.url); */
  /* strcat(url, "/"); */
  /* strcat(url, options.mac); */
  /* strcat(url, "?access_token="); */
  /* strcat(url, options.token); */

  char *url = options.url;

  CURL *curl;
  CURLcode res;

  struct curl_slist *headers = NULL;

  curl_global_init( CURL_GLOBAL_ALL );

  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");

  curl = curl_easy_init();

  if(curl)
  {
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cucumber Bot");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_object_to_json_string(json));
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

    res = curl_easy_perform(curl);
  }

  curl_easy_cleanup(curl);
  curl_global_cleanup();
  curl_slist_free_all(headers);
  if(res == CURLE_OK)
    return 1;

  return 0;
}

void *format_ssids(const struct iw_ops *iw,
    struct iw_ssid_entry *e,
    json_object *jssids)
{

  /* int noise, signal, quality, quality_max, bitrate, txpower, channel; */
  /* static char bssid[18] = { 0 }; */
  /* char ssid[ESSID_MAX_SIZE+1] = { 0 }; */
  /* char *interface = e->ifname; */

  /* json_object *jiface = json_object_new_string(interface); */
  /* json_object_object_add(jssids, "interface", jiface); */

  /* json_object *jchannel = json_object_new_int(e->channel); */
  /* json_object_object_add(jssids, "channel", jchannel); */

  /* if (iw->noise(interface, &noise)) { */
  /*   json_object *jnoise = json_object_new_int(noise); */
  /*   json_object_object_add(jssids, "noise", jnoise); */
  /* } */

  /* // Cant do this when no-one connected, causes memory leak */
  /* // Cant do this when no-one connected, causes memory leak */
  /* // Cant do this when no-one connected, causes memory leak */

  /* // This actually just averages all the signals from the clients */
  /* // I don't think we should do this way */
  /* if (iw->signal(interface, &signal)) { */
  /*   json_object *jsignal = json_object_new_int(signal); */
  /*   json_object_object_add(jssids, "signal", jsignal); */
  /* } */

  /* if (iw->bitrate(interface, &bitrate)) { */
  /*   json_object *jbitrate = json_object_new_int(bitrate); */
  /*   json_object_object_add(jssids, "bitrate", jbitrate); */
  /* } */

  /* // THE QUALITY CAN BE NULL SOMETIMES - IE. MON INTERFACE // */
  /* if (signal != 0 && iw->quality(signal, &quality)) { */
  /*   json_object *jquality = json_object_new_int(quality); */
  /*   json_object_object_add(jssids, "quality", jquality); */
  /* } */

  /* if (iw->quality_max(&quality_max)) { */
  /*   json_object *jquality_max = json_object_new_int(quality_max); */
  /*   json_object_object_add(jssids, "quality_max", jquality_max); */
  /* } */

  /* if (iw->bssid(interface, bssid)) { */
  /*   json_object *jbssid = json_object_new_string(bssid); */
  /*   json_object_object_add(jssids, "bssid", jbssid); */
  /* } */

  /* if (iw->ssid(interface, ssid)) { */
  /*   json_object *jssid = json_object_new_string(ssid); */
  /*   json_object_object_add(jssids, "ssid", jssid); */
  /* } */

  // Problem!
  /* if (iw->txpower(interface, &txpower)) { */
  /*   json_object *jtxpower = json_object_new_int(txpower); */
  /*   json_object_object_add(jssids, "txpower", jtxpower); */
  /* } */

  // Not complete
  /* if (iw->encryption(interface, encryption)) { */
  /*   json_object *jencryption = json_object_new_string(encryption); */
  /*   json_object_object_add(*jssids, "encryption", jencryption); */
  /* } */
}

void format_bssid(uint8_t *mac, char *buf)
{
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void *format_stations(const char *ssid,
    const char *ifname, struct iw_stationlist_entry *s, json_object *jstations
    )
{
  static char buf[18];

  json_object *jiface = json_object_new_string(ifname);
  json_object_object_add(jstations, "interface", jiface);

  json_object *jssid = json_object_new_string(ssid);
  json_object_object_add(jstations, "ssid", jssid);

  format_bssid(s->mac, buf);

  json_object *jmac = json_object_new_string(buf);
  json_object_object_add(jstations, "mac", jmac);

  json_object *jinactive = json_object_new_int(s->inactive_time);
  json_object_object_add(jstations, "inactive_time", jinactive);

  json_object *jrx_packets = json_object_new_double(s->rx_packets);
  json_object_object_add(jstations, "rx_packets", jrx_packets);

  json_object *jtx_packets = json_object_new_double(s->tx_packets);
  json_object_object_add(jstations, "tx_packets", jtx_packets);

  if (s->rx_bytes_64 >0) {
    json_object *jrx_bytes = json_object_new_double(s->rx_bytes_64);
    json_object_object_add(jstations, "rx_bytes", jrx_bytes);
  }
  else {
    json_object *jrx_bytes = json_object_new_double(s->rx_bytes);
    json_object_object_add(jstations, "rx_bytes", jrx_bytes);
  }

  if (s->tx_bytes_64 >0) {
    json_object *jtx_bytes = json_object_new_double(s->tx_bytes_64);
    json_object_object_add(jstations, "tx_bytes", jtx_bytes);
  } else {
    json_object *jtx_bytes = json_object_new_double(s->tx_bytes);
    json_object_object_add(jstations, "tx_bytes", jtx_bytes);
  }

  json_object *jbeacon_rx = json_object_new_double(s->beacon_rx);
  json_object_object_add(jstations, "beacon_rx", jbeacon_rx);

  json_object *jtx_retries = json_object_new_int(s->tx_retries);
  json_object_object_add(jstations, "tx_retries", jtx_retries);

  json_object *jtx_failed = json_object_new_int(s->tx_failed);
  json_object_object_add(jstations, "tx_failed", jtx_failed);

  json_object *jbeacon_loss = json_object_new_int(s->beacon_loss);
  json_object_object_add(jstations, "beacon_loss", jbeacon_loss);

  json_object *jsignal = json_object_new_int(s->signal);
  json_object_object_add(jstations, "signal", jsignal);

  json_object *jsignal_avg = json_object_new_int(s->signal_avg);
  json_object_object_add(jstations, "signal_avg", jsignal_avg);

  if (s->beacon_signal_avg > 0) {
    json_object *jbeacon_signal_avg = json_object_new_int(s->beacon_signal_avg);
    json_object_object_add(jstations, "beacon_signal_avg", jbeacon_signal_avg);
  }

  if (s->t_offset > 0) {
    json_object *jt_offset = json_object_new_double(s->t_offset);
    json_object_object_add(jstations, "t_offset", jt_offset);
  }

  json_object *jtx_bitrate = json_object_new_int(s->tx_bitrate);
  json_object_object_add(jstations, "tx_bitrate", jtx_bitrate);

  json_object *jrx_bitrate = json_object_new_int(s->rx_bitrate);
  json_object_object_add(jstations, "rx_bitrate", jrx_bitrate);

  json_object *jauthorized = json_object_new_boolean(s->authorized);
  json_object_object_add(jstations, "authorized", jauthorized);

  json_object *jauthenticated = json_object_new_boolean(s->authenticated);
  json_object_object_add(jstations, "authenticated", jauthenticated);

  json_object *jassociated = json_object_new_boolean(s->associated);
  json_object_object_add(jstations, "associated", jassociated);

  json_object *jwmm = json_object_new_boolean(s->wmm);
  json_object_object_add(jstations, "wmm", jwmm);

  json_object *jmfp = json_object_new_boolean(s->mfp);
  json_object_object_add(jstations, "mfp", jmfp);

  json_object *jtdls = json_object_new_boolean(s->tdls);
  json_object_object_add(jstations, "tdls", jtdls);

  json_object *jpreamble = json_object_new_int(s->preamble);
  json_object_object_add(jstations, "preamble", jpreamble);

  json_object *jconn_time = json_object_new_int(s->conn_time);
  json_object_object_add(jstations, "conn_time", jconn_time);

  json_object *jexpected_tput = json_object_new_int(s->expected_tput);
  json_object_object_add(jstations, "expected_tput", jexpected_tput);
}

void *format_scan(struct iw_scanlist_entry *s, json_object *jscan)
{
  char enc[512];
  static char buf[18];

  format_bssid(s->mac, buf);

  json_object *jbssid = json_object_new_string(buf);
  json_object_object_add(jscan, "mac", jbssid);

  json_object *jchannel = json_object_new_int(s->channel);
  json_object_object_add(jscan, "channel", jchannel);

  json_object *jessid = json_object_new_string(s->ssid);
  json_object_object_add(jscan, "ssid", jessid);

  format_encryption(&s->crypto, enc);

  json_object *jcrypto = json_object_new_string(enc);
  json_object_object_add(jscan, "encryption", jcrypto);

  if (s->signal > 0) {
    int signal = s->signal - 0x100;
    json_object *jsignal = json_object_new_int(signal);
    json_object_object_add(jscan, "signal", jsignal);

    json_object *jquality = json_object_new_int(s->quality);
    json_object_object_add(jscan, "quality", jquality);

    json_object *jquality_max = json_object_new_int(s->quality_max);
    json_object_object_add(jscan, "quality_max", jquality_max);
  }

  json_object *jage = json_object_new_int(s->age);
  json_object_object_add(jscan, "last_seen", jage);
}

void collect_data()
{
  debug("collecting the datas!");

  char rx[21], tx[21], wan_ip[21];
  json_object *jobj = json_object_new_object();
  json_object *jattr = json_object_new_object();

  if (strlen(gateway) == 0)
    route();

  interface_ip(wan_name, wan_ip);

  struct InterfaceStats istats = stats(wan_name);
  sprintf(tx, "%" PRIu64, istats.tx);
  sprintf(rx, "%" PRIu64, istats.rx);

  char machine[100];
  machine_type(machine);

  struct SystemInfo info = system_info();

  if (options.no_survey != 1)
  {
    debug("Running WiFi Interface Stats");

    int len, x, i, ii, len_a, jj, xx, len_s, xxx;
    char buf[1024];
    char buf_a[1024];
    char buf_s[1024];
    const struct iw_ops *iw;
    struct iw_ssid_entry *e;
    struct iw_stationlist_entry *st;
    struct iw_scanlist_entry *sc;

    // Check chipset / drivers required
    iw = &nl80211_exec;

    /* char bufff[1024]; */
    /* int llen; */
    /* iw->info(bufff, &llen); */

    iw->ssids(buf, &len);
    json_object *jiface_array = json_object_new_array();
    json_object *jstations_array = json_object_new_array();
    json_object *jscan_array = json_object_new_array();

    int scan_2, scan_5;

    for (i = 0, x = 1; i < len; i += sizeof(struct iw_ssid_entry), x++)
    {
      e = (struct iw_ssid_entry *) &buf[i];

      printf("Collecting Data for SSID: %s\n", e->ifname);

      json_object *jssids = json_object_new_object();
      format_ssids(iw, e, jssids);
      json_object_array_add(jiface_array, jssids);

      iw->stations(e->ifname, buf_a, &len_a);
      for (ii = 0, xx = 1; ii < len_a; ii += sizeof(struct iw_stationlist_entry), xx++)
      {
        st = (struct iw_stationlist_entry *) &buf_a[ii];
        json_object *jstations = json_object_new_object();
        format_stations(e->ssid, e->ifname, st, jstations);
        json_object_array_add(jstations_array, jstations);
      }

      int ret = strcmp(e->ifname, "mon0");
      if (ret != 0 && options.no_scan != 1 && iw->scan(e->ifname, buf_s, &len_s)) {
        for (jj = 0, xxx = 1; jj < len_s; jj += sizeof(struct iw_scanlist_entry), xxx++)
        {
          int channel = e->channel;
          if ((scan_2 == 0 && channel <= 14) || (scan_5 == 0 && channel > 14)) {
            sc = (struct iw_scanlist_entry *) &buf_s[jj];
            json_object *jscan = json_object_new_object();
            format_scan(sc, jscan);
            json_object_array_add(jscan_array, jscan);
            if (channel <= 14)
              scan_2 = 1;
            else if (channel>14)
              scan_5 = 1;
          }
        }
        debug("There are %d SSIDs in the area", xxx-1);
      }
    }

    json_object_object_add(jobj, "ssids", jiface_array);
    json_object_object_add(jobj, "survey", jscan_array);
    json_object_object_add(jobj, "stations", jstations_array);
  }

#ifdef __OPENWRT__
  char firmware[20];
  readlineToBuffer("/etc/openwrt_version", firmware);

  // Should we save to options ? //
  json_object *jfirmware = json_object_new_string(firmware);
  json_object_object_add(jattr, "firmware", jfirmware);
#endif

  json_object *jserial = json_object_new_string("simon says");
  json_object_object_add(jattr, "serial", jserial);

  json_object *jwan_name = json_object_new_string(wan_name);
  json_object_object_add(jattr, "wan_name", jwan_name);

  json_object *jgateway = json_object_new_string(gateway);
  json_object_object_add(jattr, "wan_gateway", jgateway);

  if (strlen(wan_ip) > 0) {
    json_object *jwanip = json_object_new_string(wan_ip);
    json_object_object_add(jattr, "wan_ip", jwanip);
  }

  if (strlen(machine) > 0) {
    // Save to the options to prevent future lookups
    strcpy(options.machine, machine);
    json_object *jmachine = json_object_new_string(machine);
    json_object_object_add(jattr, "machine_type", jmachine);
  }

  json_object *jtx = json_object_new_string(tx);
  json_object_object_add(jattr, "tx_bytes", jtx);

  json_object *jrx = json_object_new_string(rx);
  json_object_object_add(jattr, "rx_bytes", jrx);

  json_object *juptime = json_object_new_int(info.uptime);
  json_object_object_add(jattr, "uptime", juptime);

  json_object *jtotalram = json_object_new_double(info.totalram);
  json_object_object_add(jattr, "total_ram", jtotalram);

  json_object *jfreeram = json_object_new_double(info.freeram);
  json_object_object_add(jattr, "free_ram", jfreeram);

  json_object *jprocs = json_object_new_int(info.procs);
  json_object_object_add(jattr, "procs", jprocs);

  // MISSING!!!!!!!
  // LOAD AVERAGE
  // INTERFACES
  // CAPS

  json_object_object_add(jobj, "device", jattr);

  if (post(jobj)) {

  } else {
    // Do something here about the curl issue, init
  }
  json_object_put(jobj);
}

void cache_data() {
  debug("save the datas!");
}

void post_data() {
  debug("sending the datas!");
}

void collect_and_send_data()
{
  if (should_collect()) {
    collect_data();
    if (online) {
      post_data();
    } else {
      cache_data();
    }
    /* post_data(); */
  }
}