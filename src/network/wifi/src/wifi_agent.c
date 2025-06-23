/**
 * @file wifi_agent.c
 * @author Theodore Bardy
 *
 * @note This file is part of the Zephyr Demo project from Witekio
 * @brief Agent for managing Wi-Fi connections
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(wifi_agent);

#include <assert.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/wifi_mgmt.h>

#include "wifi_agent.h"

// Nubmer of attempts to connect to Wi-Fi and sleep time between attempts
#define WIFI_NB_TRIES (5)
#define WIFI_SLEEP_BTW_TRIES (500)

// Ensure the Wi-Fi SSID and password are defined
#if !defined(CONFIG_WIFI_SSID)
#error Wi-Fi SSID is not defined. Please set CONFIG_WIFI_SSID in your project configuration.
#else
_Static_assert(strcmp(CONFIG_WIFI_SSID, "") != 0, "Wi-Fi SSID must not be empty");
_Static_assert(strlen(CONFIG_WIFI_SSID) <= 32, "Wi-Fi SSID must be 32 characters or less");
#endif

#if !defined(CONFIG_WIFI_PASSWORD)
#error Wi-Fi password is not defined. Please set CONFIG_WIFI_PASSWORD in your project configuration.
#else
_Static_assert(strcmp(CONFIG_WIFI_PASSWORD, "") != 0, "Wi-Fi password must not be empty");
_Static_assert(strlen(CONFIG_WIFI_PASSWORD) >= 8 && strlen(CONFIG_WIFI_PASSWORD) <= 64, "Wi-Fi password must be between 8 and 64 characters");
#endif

#define WIFI_AGENT_THREAD_STACK_SIZE (1024)
#define WIFI_AGENT_THREAD_PRIORITY   (3)

K_SEM_DEFINE(wifi_agent_initialized, 0, 1);
K_SEM_DEFINE(sem_wifi_agent_connect, 0, 1);
K_SEM_DEFINE(sem_wifi_agent_connected, 0, 1);
K_SEM_DEFINE(sem_wifi_agent_disconnected, 0, 1);

#define NET_EVENT_WIFI_MASK \
    (NET_EVENT_WIFI_CONNECT_RESULT | \
     NET_EVENT_WIFI_DISCONNECT_RESULT)

static struct net_if *wifi_iface = NULL;
static struct wifi_connect_req_params wifi_config = {
    .ssid = (const uint8_t *)CONFIG_WIFI_SSID,
    .ssid_length = strlen(CONFIG_WIFI_SSID),
    .psk = (const uint8_t *)CONFIG_WIFI_PASSWORD,
    .psk_length = strlen(CONFIG_WIFI_PASSWORD),
    .channel = WIFI_CHANNEL_ANY,
    .security = WIFI_SECURITY_TYPE_PSK,
};

// Wi-Fi agent state machine enumeration
enum wifi_agent_state {
    WIFI_AGENT_STATE_IDLE,
    WIFI_AGENT_STATE_CONNECTING,
    WIFI_AGENT_STATE_CONNECTED
};
static enum wifi_agent_state current_state = WIFI_AGENT_STATE_IDLE;

static struct net_mgmt_event_callback cb;
static void prvWifiEventHandler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface)
{
    switch (mgmt_event) {
        case NET_EVENT_WIFI_CONNECT_RESULT:
            k_sem_give(&sem_wifi_agent_connected);
            LOG_INF("Wi-Fi connected to %s", CONFIG_WIFI_SSID);
            break;

        case NET_EVENT_WIFI_DISCONNECT_RESULT:
            k_sem_give(&sem_wifi_agent_disconnected);
            LOG_INF("Wi-Fi disconnected from %s", CONFIG_WIFI_SSID);
            break;

        default:
            LOG_WRN("Unhandled Wi-Fi event: %u", mgmt_event);
            break;
    }
}

void wifi_agent_init(void)
{
    net_mgmt_init_event_callback(&cb, prvWifiEventHandler, NET_EVENT_WIFI_MASK);
    net_mgmt_add_event_callback(&cb);

    LOG_INF("Wi-Fi agent initialized");
    k_sem_give(&wifi_agent_initialized);
}

bool wifi_agent_connect(void)
{
    if (current_state == WIFI_AGENT_STATE_CONNECTED)
    {
        LOG_INF("Wi-Fi agent is already connected");
        return true;
    }

    if (current_state != WIFI_AGENT_STATE_IDLE) {
        LOG_WRN("Wi-Fi agent is not idle, cannot connect");
        return false;
    }

    k_sem_give(&sem_wifi_agent_connect);
    return true;
}

bool wifi_agent_disconnect(void)
{
    if (current_state != WIFI_AGENT_STATE_CONNECTED) {
        LOG_WRN("Wi-Fi agent is not connected, cannot disconnect");
        return false;
    }

    if (net_mgmt(NET_REQUEST_WIFI_DISCONNECT, wifi_iface, NULL, 0)) {
        LOG_ERR("Failed to initiate Wi-Fi disconnection");
        return false;
    }

    return true;
}

bool wifi_agent_is_connected(size_t delay_ms)
{
    while(delay_ms > 0 && current_state != WIFI_AGENT_STATE_CONNECTED) {
        k_msleep((100));
        delay_ms -= 100;
    }
    return (current_state == WIFI_AGENT_STATE_CONNECTED) ? true : false;
}

static bool prvWifiConnect(void)
{
    wifi_iface = net_if_get_wifi_sta();
    if (wifi_iface == NULL) {
        LOG_ERR("Failed to get Wi-Fi interface");
        return false;
    }

    uint8_t nb_tries = WIFI_NB_TRIES;
    while (nb_tries-- > 0)
    {
        if (net_mgmt(NET_REQUEST_WIFI_CONNECT, wifi_iface, &wifi_config, sizeof(wifi_config))) {
            LOG_ERR("Connect request failed. Retrying...");
            k_msleep(WIFI_SLEEP_BTW_TRIES);
        }
        else {
            break;
        }
    }

    net_dhcpv4_start(wifi_iface);

    return (nb_tries > 0) ? true : false;
}

void wifi_agent_get_mac_address(char *mac_address)
{
    assert(NULL != mac_address);

    struct net_if *iface = net_if_get_first_up();
    assert(NULL != iface);

    struct net_linkaddr *linkaddr = net_if_get_link_addr(iface);
    assert(NULL != linkaddr);

    snprintf(mac_address,
             18,
             "%02x:%02x:%02x:%02x:%02x:%02x",
             linkaddr->addr[0],
             linkaddr->addr[1],
             linkaddr->addr[2],
             linkaddr->addr[3],
             linkaddr->addr[4],
             linkaddr->addr[5]);
}

static void prvWifiAgentThread(void *arg1, void *arg2, void *arg3)
{
    // Wait for the Wi-Fi agent to be initialized
    k_sem_take(&wifi_agent_initialized, K_FOREVER);

    LOG_INF("Wi-Fi agent thread started");

    while (true) {
        switch (current_state)
        {
            case WIFI_AGENT_STATE_IDLE:
                // Wait for a connection request
                k_sem_take(&sem_wifi_agent_connect, K_FOREVER);
                current_state = WIFI_AGENT_STATE_CONNECTING;
                break;

            case WIFI_AGENT_STATE_CONNECTING:
                // Attempt to connect to Wi-Fi
                LOG_INF("Attempting to connect to Wi-Fi...");

                if (!prvWifiConnect()) {
                    LOG_ERR("Failed to connect to Wi-Fi");
                    current_state = WIFI_AGENT_STATE_IDLE;
                    continue;
                }

                k_sem_take(&sem_wifi_agent_connected, K_FOREVER);
                current_state = WIFI_AGENT_STATE_CONNECTED;
                break;

            case WIFI_AGENT_STATE_CONNECTED:
                // Handle connected state
                LOG_INF("Wi-Fi connected to the AP");
                k_sem_take(&sem_wifi_agent_disconnected, K_FOREVER);
                current_state = WIFI_AGENT_STATE_IDLE;
                break;

            default:
                LOG_ERR("Unknown Wi-Fi agent state: %d", current_state);
                current_state = WIFI_AGENT_STATE_IDLE;
                break;
        }
    }
}
K_THREAD_DEFINE(wifi_agent_thread_id, WIFI_AGENT_THREAD_STACK_SIZE, prvWifiAgentThread, NULL, NULL, NULL, WIFI_AGENT_THREAD_PRIORITY, 0, 0);
