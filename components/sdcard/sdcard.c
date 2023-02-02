#include <stdbool.h>
#include "esp_err.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

#include "sdcard.h"

const char mount_point[] = "/sdcard";
static const char *TAG = "sdCard";

esp_err_t sdcard_init()
{
    esp_err_t result = ESP_FAIL;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        // if configured and mounting failed, format sdCard
#ifdef CONFIG_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
        .max_files = 3,
        .allocation_unit_size = 16 * 1024};
    sdmmc_card_t *card;

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // Set bus width to use if configured
#ifdef CONFIG_SDMMC_BUS_WIDTH_4
    slot_config.width = 4;
#else
    slot_config.width = 1;
#endif

    result = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (result != ESP_OK)
    {
        LOGE(TAG, "Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(result));
    }

    return result;
}

esp_err_t sdcard_saveImage(camera_fb_t *fb)
{
    return ESP_OK;
}