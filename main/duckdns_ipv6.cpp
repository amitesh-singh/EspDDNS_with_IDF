// esp32 example
//#define LWIP_IPV6 1
#define LWIP_IPV6_DUP_DETECT_ATTEMPTS 0
#include <EspDDNS.h>
#include <lwip/dns.h>


static constexpr const auto ssid { "XXXX" };
static constexpr const auto passwd { "XXXX" };

bool waitFlag = true;
esp_netif_t* get_esp_interface_netif(esp_interface_t interface);

void WiFiEvent(WiFiEvent_t event)
{
    switch(event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            //set sta hostname here
            WiFi.setHostname("espddns-ipv6test");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            //enable sta ipv6 here
            //esp_netif_create_ip6_linklocal(get_esp_interface_netif(ESP_IF_WIFI_STA));
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println("call it when we get the ipv4");
            Serial.println(WiFi.localIP());
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.print("STA IPv6: ");
            Serial.println(WiFi.localIPv6());
            waitFlag = false;
            break;
        default:
            break;
    }
}

void set_dnsserver_google()
{
    //set dns server
    ip_addr_t ip6_dns;
    // GOOGLE EXTERNAL - 2001:4860:4860::8888
    IP_ADDR6(&ip6_dns,PP_HTONL(0x20014860UL), PP_HTONL(0x48600000UL), PP_HTONL(0x00000000UL), PP_HTONL(0x00008888UL));
    dns_setserver(0, &ip6_dns);
}

ddns::client ddns_client(ddns::type::DUCKDNS, "XXXXX.duckdns.org", 
                         "duckdns token id", ddns::ip_type::IPv6);

void print_global_ipv6()
{
    String ipv6 = ddns_client.get_ipv6();
    Serial.printf("global ipv6: %s", ipv6.c_str());
}

void setup()
{
    Serial.begin(115200);

    WiFi.persistent(false);
    WiFi.disconnect(false);
    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_STA);    
    WiFi.begin(ssid, passwd);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(100);
      Serial.print(".");
    }

    WiFi.enableIpV6();
    Serial.println("Connected..");

    ddns_client.onUpdate([](String old_ip, String new_ip) {
        Serial.printf("Ip address is changed to %s from %s", new_ip.c_str(), old_ip.c_str());
    });

    while (waitFlag)
    {
        Serial.println("waiting...");
        delay(1000);
    }
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.localIPv6());

    // For esp32, this is required. The global ipv6 gets assigned after few seconds since link ipv6 is assigned
    delay(1000);
    //just check once on boot only.
    ddns_client.update();
    Serial.printf("interface up: %d\r\n", esp_netif_is_netif_up( get_esp_interface_netif(ESP_IF_WIFI_STA)));
}

void loop()
{
    print_global_ipv6();
    Serial.print("\r\nlocal ipv6: ");
    Serial.println(WiFi.localIPv6());
    delay(1000);
}
