/* Play an OGG from HTTP, triggered by websocket message

*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/ringbuf.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "http_stream.h"
#include "i2s_stream.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

#include "esp_peripherals.h"
#include "periph_wifi.h"
#include "board.h"

#include "opus_decoder.h"
static const char *TAG = "WEBSOCKET_OGG";
static const esp_websocket_client_config_t ws_cfg = {
    .uri = "wss://beacon.eosfrontier.space/socket.io/?transport=websocket"
};

static const char *playfile_prefix = "https://beacon.eosfrontier.space/sounds";
static const char *init_playfile = "https://beacon.eosfrontier.space/sounds/example.opus";
RingbufHandle_t playbuffer;
// static char playfile[512];

static inline bool startswith(const char *str, const char *prefix)
{
  return (!strncmp(str, prefix, strlen(prefix)));
}

static void play_audio(esp_periph_set_handle_t set, const char *uri)
{
    ESP_LOGI(TAG, "[1.0] Free memory before play: %d bytes", xPortGetFreeHeapSize());
    ESP_LOGI(TAG, "[2.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    audio_pipeline_handle_t pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[2.1] Create http stream to read data");
    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    ESP_LOGI(TAG, "[2.1] HTTP out rb size was 0x%x", http_cfg.out_rb_size);
    http_cfg.out_rb_size = 0x20000;
    ESP_LOGI(TAG, "[2.1] HTTP out rb size set to 0x%x", http_cfg.out_rb_size);
    audio_element_handle_t http_stream_reader = http_stream_init(&http_cfg);

    ESP_LOGI(TAG, "[2.2] Create decoder");

    opus_decoder_cfg_t opus_cfg = DEFAULT_OPUS_DECODER_CONFIG();
    audio_element_handle_t decoder_stream = decoder_opus_init(&opus_cfg);

    ESP_LOGI(TAG, "[2.3] Create i2s stream to write data to internal DAC");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_INTERNAL_DAC_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    audio_element_handle_t i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[2.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, http_stream_reader, "http");
    audio_pipeline_register(pipeline, decoder_stream,     "opus");
    audio_pipeline_register(pipeline, i2s_stream_writer,  "i2s");

    ESP_LOGI(TAG, "[2.5] Link it together http_stream-->opus_decoder-->i2s_stream-->[codec_chip]");
    audio_pipeline_link(pipeline, (const char *[]) {"http", "opus", "i2s"}, 3);

    ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    audio_element_set_uri(http_stream_reader, uri);
    vRingbufferReturnItem(playbuffer, (void *)uri);
    audio_pipeline_run(pipeline);

    while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT) {
            const char *source = NULL;
            if (msg.source == (void *) http_stream_reader) {
                source = "HTTP";
            }
            if (msg.source == (void *) decoder_stream) {
                source = "OPUS";
            }
            if (msg.source == (void *) i2s_stream_writer) {
                source = "I2S";
            }

            ESP_LOGI(TAG, "[ * ] %s event, cmd = %x", source, msg.cmd);
            if (msg.cmd == AEL_MSG_CMD_REPORT_STATUS) {
                audio_element_info_t music_info = {0};
                audio_element_getinfo(msg.source, &music_info);
                ESP_LOGI(TAG, "[ * ] Receive music info from %s, sample_rates=%d, bits=%d, ch=%d",
                        source, music_info.sample_rates, music_info.bits, music_info.channels);
                if (music_info.uri) {
                    ESP_LOGI(TAG, "[ * ]       uri = %s", music_info.uri);
                }
            }
            if (msg.cmd == AEL_MSG_CMD_REPORT_CODEC_FMT) {
                audio_element_info_t music_info = {0};
                audio_element_getinfo(msg.source, &music_info);
                ESP_LOGI(TAG, "[ * ] Receive codec fmt from %s, codec=%x",
                        source, music_info.codec_fmt);
            }
            if (msg.cmd == AEL_MSG_CMD_REPORT_POSITION) {
                audio_element_info_t music_info = {0};
                audio_element_getinfo(msg.source, &music_info);
                ESP_LOGI(TAG, "[ * ] Receive position from %s, pos=%llx",
                        source, music_info.byte_pos);
            }
        }
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            && msg.source == (void *) decoder_stream
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(decoder_stream, &music_info);

            ESP_LOGI(TAG, "[ * ] Receive music info from opus decoder, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);

            audio_element_setinfo(i2s_stream_writer, &music_info);
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
            continue;
        }

        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
    }

    ESP_LOGI(TAG, "[ 6 ] Stop audio_pipeline and release all resources");
    ESP_LOGI(TAG, "[6.1] Free memory: %d bytes", xPortGetFreeHeapSize());
    audio_pipeline_terminate(pipeline);
    audio_pipeline_unregister(pipeline, http_stream_reader);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, decoder_stream);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);
    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);
    ESP_LOGI(TAG, "[6.2] Free memory after deinit: %d bytes", xPortGetFreeHeapSize());
    ESP_LOGI(TAG, "[6.2] Lowest free memory: %d bytes", xPortGetMinimumEverFreeHeapSize());
}

static void ws_handler(void *args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    const char *msg;
    switch(id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Websocket Connected");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Websocket Disonnected");
            break;
        case WEBSOCKET_EVENT_DATA:
            msg = data->data_ptr;
            ESP_LOGI(TAG, "Websocket Data");
            ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
            ESP_LOGW(TAG, "Received=%.*s", data->data_len, msg);
            ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
            if (data->op_code == 1 && data->data_len > 2 && msg[0] == '4') {
                if (msg[1] == '0') {

                } else if (msg[1] == '2') {
                    if (startswith(msg+2, "[\"playAudioFile\"")) {
                        const char *fn = msg+20;
                        const char *fne = strchr(fn, '"');
                        if (fne-fn > 255) {
                            ESP_LOGW(TAG, "Audio filename too long: %s", fn);
                        } else {
                            size_t sz = strlen(playfile_prefix)+(fne-fn);
                            char playfile[sz+1];
                            memcpy(playfile, playfile_prefix, strlen(playfile_prefix));
                            memcpy(playfile+strlen(playfile_prefix), fn, fne-fn);
                            playfile[sz] = 0;
                            ESP_LOGI(TAG, "Playing broadcast: <%s>", playfile);
                            BaseType_t res = xRingbufferSend(playbuffer, (void *)playfile, sz+1, pdMS_TO_TICKS(0));
                            if (res != pdTRUE) {
                                ESP_LOGW(TAG, "Queue full when playing broadcast");
                            }
                        }
                    }
                }
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "Websocket ERROR");
            break;
    }
}

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    tcpip_adapter_init();

    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    playbuffer = xRingbufferCreate(512, RINGBUF_TYPE_NOSPLIT);
    mem_assert(playbuffer);

    xRingbufferSend(playbuffer, (void *)init_playfile, strlen(init_playfile)+1, pdMS_TO_TICKS(0));

    /*
    ESP_LOGI(TAG, "[ 1 ] Start audio codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
    */

    ESP_LOGI(TAG, "[ 3 ] Start and wait for Wi-Fi network");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);
    periph_wifi_cfg_t wifi_cfg = {
        .ssid = "Airy",
        .password = "Landryssa",
    };

    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);
    esp_periph_start(set, wifi_handle);
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);

    ESP_LOGI(TAG, "Connecting to %s...", ws_cfg.uri);
    esp_websocket_client_handle_t ws = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(ws, WEBSOCKET_EVENT_ANY, ws_handler, (void *)ws);
    esp_websocket_client_start(ws);

    while (1) {
        size_t urisize;
        const char *uri = (const char *)xRingbufferReceive(playbuffer, &urisize, pdMS_TO_TICKS(1000));
        if (uri) {
            play_audio(set, uri);
        }
    }

    /* Stop all peripherals before removing the listener */
    /*
    esp_periph_set_stop_all(set);
    */

    /* Release all resources */
    // esp_periph_set_destroy(set);
}
