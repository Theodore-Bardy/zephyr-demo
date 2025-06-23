/**
 * @file wifi_agent.h
 * @author Theodore Bardy
 *
 * @note This file is part of the Zephyr Demo project from Witekio
 * @brief Agent for managing Wi-Fi connections
 */

#ifndef WIFI_AGENT_H
#define WIFI_AGENT_H

/**
 * @brief Initializes the Wi-Fi agent
 */
void wifi_agent_init(void);

/**
 * @brief Connects to the Wi-Fi network
 * @return true if the connection was initiated successfully, false otherwise
 */
bool wifi_agent_connect(void);

/**
 * @brief Disconnects from the Wi-Fi network
 * @return true if the disconnection was initiated successfully, false otherwise
 */
bool wifi_agent_disconnect(void);

/**
 * @brief Checks if the Wi-Fi agent is connected to a network
 * @param delay_ms Delay in milliseconds to wait for connection status
 * @return true if connected, false otherwise
 */
bool wifi_agent_is_connected(size_t delay_ms);

/**
 * @brief Gets the MAC address of the Wi-Fi interface
 * @param mac_address Pointer to a buffer where the MAC address will be stored
 */
void wifi_agent_get_mac_address(char *mac_address);

#endif // WIFI_AGENT_H
