/**
 * @file      ota_agent.h
 * @author    Theodore Bardy
 *
 * @note      This file is part of Witekio's Zephyr Demo project
 * @brief     Agent managing OTA updates
 */

#ifndef OTA_AGENT_H
#define OTA_AGENT_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /**
     * @brief Initializes the OTA agent
     */
    void ota_agent_init(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // OTA_AGENT_H
