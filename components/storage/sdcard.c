#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

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
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_card_t *card;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    result = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(result));
        return result;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    return result;
}

esp_err_t sdcard_saveImage(unsigned char *buffer, size_t size)
{
    int64_t timestamp = esp_timer_get_time();

    char *filename = malloc(20 + sizeof(int64_t));
    sprintf(filename, "%s/pic_%lli.jpg", mount_point, timestamp);

    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }

    fwrite(buffer, 1, size, file);
    ESP_LOGI(TAG, "Picture saved as: %s", filename);

    fclose(file);
    free(filename);

    return ESP_OK;
}

void sdcard_deleteImage(const char *filename)
{
    char *file = (char *)malloc((strlen(mount_point) + strlen(filename)) * sizeof(char));
    strcpy(file, mount_point);
    strcat(file, filename);

    // Check if destination file exists and delete it
    struct stat st;
    if (stat(file, &st) == 0)
    {
        unlink(file);
    }
    free(file);
}