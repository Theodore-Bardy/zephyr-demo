/**
 * @file      ota_agent.h
 * @author    Theodore Bardy
 *
 * @note      This file is part of Witekio's Zephyr Demo project
 * @brief     Agent managing OTA updates
 */

#ifndef OTA_AGENT_H
#define OTA_AGENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /**
     * @brief Initializes the OTA agent
     * @return true if the OTA agent initialized successfully, false otherwise
     */
    bool ota_agent_init(void);

    /**
     * @brief Starts the OTA agent
     * @return true if the OTA agent started successfully, false otherwise
     */
    bool ota_agent_start(void);
    
    /**
     * @brief Stops the OTA agent
     * @return true if the OTA agent stopped successfully, false otherwise
     */
    bool ota_agent_stop(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // OTA_AGENT_H
