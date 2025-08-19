/**
 * @file      ota_agent.c
 * @author    Theodore Bardy
 *
 * @note      This file is part of Witekio's Zephyr Demo project
 * @brief     Agent managing OTA updates
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ota_agent);

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/net/tls_credentials.h>

#include <mender/utils.h>
#include <mender/client.h>
#include <mender/inventory.h>

#include "ota_agent.h"
#include "wifi_agent.h"

// Ensure Mender inventory feature is enabled
#ifdef CONFIG_MENDER_CLIENT_INVENTORY_DISABLE
#error Mender MCU integration app requires the inventory feature
#endif

// TLS credentials
#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
static const unsigned char primary_certificate[] = {
#include "PrimaryCertificate.cer.inc"
};
#ifdef CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_SECONDARY_ENABLED
static const unsigned char secondary_certificate[] = {
#include "SecondaryCertificate.cer.inc"
};
#endif
#endif

static char              device_mac_address[18] = { 0 };
static mender_identity_t mender_identity        = {
           .name  = "mac",
           .value = device_mac_address,
};

#define OTA_AGENT_THREAD_STACK_SIZE (4096)
#define OTA_AGENT_THREAD_PRIORITY   (3)

K_SEM_DEFINE(ota_agent_initialized, 0, 1);

void
ota_agent_init (void)
{
    if (!wifi_agent_connect())
    {
        LOG_ERR("Failed to connect to Wi-Fi");
        return;
    }

    LOG_INF("OTA agent initialized");
    k_sem_give(&ota_agent_initialized);
}

/**
 * @brief Install TLS credentials for Hosted Mender setup
 * @return return 0 on success, -EACCES, -ENOMEM or -EEXIST on error
 * @note See
 * https://docs.zephyrproject.org/3.7.0/doxygen/html/group__tls__credentials.html#ga640ff6dd3eb4d5017feaab6fab2bb2f7
 */
int
prvCertsAddCredentials (void)
{
    int ret = 0;

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
    ret = tls_credential_add(CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_PRIMARY,
                             TLS_CREDENTIAL_CA_CERTIFICATE,
                             primary_certificate,
                             sizeof(primary_certificate));
    if (0 != ret)
    {
        return ret;
    }
#ifdef CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_SECONDARY_ENABLED
    ret = tls_credential_add(CONFIG_MENDER_NET_CA_CERTIFICATE_TAG_SECONDARY,
                             TLS_CREDENTIAL_CA_CERTIFICATE,
                             secondary_certificate,
                             sizeof(secondary_certificate));
#endif
#endif

    return ret;
}

MENDER_FUNC_WEAK mender_err_t
prvMenderNetworkConnectCb (void)
{
    LOG_DBG("prvMenderNetworkConnectCb");
    return wifi_agent_is_connected(500) ? MENDER_OK : MENDER_FAIL;
}

MENDER_FUNC_WEAK mender_err_t
prvMenderNetworkReleaseCb (void)
{
    LOG_DBG("prvMenderNetworkReleaseCb");
    return MENDER_OK;
}

MENDER_FUNC_WEAK mender_err_t
prvMenderDeploymentStatusCb (mender_deployment_status_t status,
                             const char                *desc)
{
    LOG_DBG("prvMenderDeploymentStatusCb: %s", desc);
    return MENDER_OK;
}

MENDER_FUNC_WEAK mender_err_t
prvMenderRestartCb (void)
{
    LOG_DBG("prvMenderRestartCb");
    sys_reboot(SYS_REBOOT_WARM);
    return MENDER_OK;
}

MENDER_FUNC_WEAK mender_err_t
prvMenderGetIdentityCb (const mender_identity_t **identity)
{
    LOG_DBG("prvMenderGetIdentityCb");
    if (NULL != identity)
    {
        *identity = &mender_identity;
        return MENDER_OK;
    }
    return MENDER_FAIL;
}

static mender_err_t
prvPersistentInventoryCb (mender_keystore_t **keystore, uint8_t *keystore_len)
{
    static mender_keystore_t inventory[]
        = { { .name = "App", .value = "Zephyr" } };
    *keystore     = inventory;
    *keystore_len = 1;
    return MENDER_OK;
}

static void
prvOtaAgentThread (void *arg1, void *arg2, void *arg3)
{
    // Wait for the OTA agent to be initialized
    k_sem_take(&ota_agent_initialized, K_FOREVER);

    LOG_INF("OTA agent thread started");

    while (true)
    {
        // Wait for Wi-Fi to be connected
        if (wifi_agent_is_connected(10000))
        {
            wifi_agent_get_mac_address(mender_identity.value);

            // Add TLS credentials
            switch (prvCertsAddCredentials())
            {
                case 0:
                    LOG_INF("TLS credentials added successfully");
                    break;

                case -EACCES:
                    LOG_ERR("Failed to add TLS credentials: Access denied");
                    break;

                case -ENOMEM:
                    LOG_ERR("Failed to add TLS credentials: Out of memory");
                    break;

                case -EEXIST:
                    LOG_WRN("TLS credentials already exist, continuing");
                    break;

                default:
                    LOG_ERR("Failed to add TLS credentials: Unknown error");
                    break;
            }

            // Initialize mender-client
            mender_client_config_t mender_client_config
                = { .device_type     = CONFIG_MENDER_DEVICE_TYPE,
                    .recommissioning = false };
            mender_client_callbacks_t mender_client_callbacks
                = { .network_connect        = prvMenderNetworkConnectCb,
                    .network_release        = prvMenderNetworkReleaseCb,
                    .deployment_status      = prvMenderDeploymentStatusCb,
                    .restart                = prvMenderRestartCb,
                    .get_identity           = prvMenderGetIdentityCb,
                    .get_user_provided_keys = NULL };

            LOG_INF("Initializing Mender Client with:");
            LOG_INF("   Device type:   '%s'", mender_client_config.device_type);
            LOG_INF("   Identity:      '{\"%s\": \"%s\"}'",
                    mender_identity.name,
                    mender_identity.value);

            if (MENDER_OK
                != mender_client_init(&mender_client_config,
                                      &mender_client_callbacks))
            {
                LOG_ERR("Failed to initialize Mender Client");
                goto END;
            }
            LOG_INF("Mender client initialized");

#ifdef CONFIG_MENDER_ZEPHYR_IMAGE_UPDATE_MODULE
            if (MENDER_OK != mender_zephyr_image_register_update_module())
            {
                LOG_ERR("Failed to register the zephyr-image Update Module");
                goto END;
            }
            LOG_INF("Update Module 'zephyr-image' initialized");
#endif /* CONFIG_MENDER_ZEPHYR_IMAGE_UPDATE_MODULE */

            if (MENDER_OK
                != mender_inventory_add_callback(prvPersistentInventoryCb,
                                                 true))
            {
                LOG_ERR("Failed to add persistent inventory callback");
                goto END;
            }
            LOG_INF("Persistent inventory callback added");

            // Start the Mender client
            if (MENDER_OK != mender_client_activate())
            {
                LOG_ERR("Failed to start Mender Client");
                goto END;
            }
            LOG_INF("Mender client started");

        END:
            k_sleep(K_FOREVER);
        }
    }
}
K_THREAD_DEFINE(ota_agent_thread_id,
                OTA_AGENT_THREAD_STACK_SIZE,
                prvOtaAgentThread,
                NULL,
                NULL,
                NULL,
                OTA_AGENT_THREAD_PRIORITY,
                0,
                0);
