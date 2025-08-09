#ifndef _IO47_AUDIO_PLAY_H
#define _IO47_AUDIO_PLAY_H

#include <stdio.h>
#include "driver/i2s.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "MP3DecoderHelix.h"  //libhelix, lưu ý thư viện này đã có chỉnh sửa

using namespace libhelix;

static int audio_level = 256;
static i2s_port_t m_i2s_port = I2S_NUM_0;
#define SAMPLE_RATE 16000

// ------------------- Init I2S -------------------
static void i2s_init(uint32_t sample_rate,i2s_pin_config_t * m_i2s_pins)
{
    // i2s config for writing both channels of I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_PDM),
        .sample_rate = sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0};
    //install and start i2s driver
    i2s_driver_install(m_i2s_port, &i2s_config, 0, NULL);
    // set up the i2s pins
    i2s_set_pin(m_i2s_port, m_i2s_pins);
    // clear the DMA buffers
    i2s_zero_dma_buffer(m_i2s_port);

    i2s_start(m_i2s_port);
}
// level: 0 = mute, 256 = giữ nguyên, >256 = to hơn
void adjustVolumeMono16_Q8_8(int16_t *pcm, size_t len, uint16_t level) {
    if (!pcm || len == 0) return;
    if((level > 200) && (level < 300))return;

    for (size_t i = 0; i < len; ++i) {
        int32_t v = (int32_t)pcm[i] * level; // nhân hệ số Q8.8
        v >>= 8;                             // chia 256 để về giá trị thực
        if (v > 32767) v = 32767;            // chống tràn
        else if (v < -32768) v = -32768;
        pcm[i] = (int16_t)v;
    }
}
static void write_pcm_to_i2s(int16_t *pcm_buff,int count)
{
    size_t bytes_written = 0;
    adjustVolumeMono16_Q8_8(pcm_buff,count,audio_level);
    esp_err_t res = i2s_write(m_i2s_port, pcm_buff, count * sizeof(int16_t), &bytes_written, 1000 / portTICK_PERIOD_MS);
    //printf("bytes_written %i  count %i\n",bytes_written,count);
    if (res != ESP_OK)
    {
      //ESP_LOGE(TAG, "Error sending audio data: %d", res);
    }
    if (bytes_written != count * sizeof(int16_t))
    {
      //ESP_LOGE(TAG, "Did not write all bytes");
    }
}
static void dataCallback(MP3FrameInfo &info, int16_t *pcm_buffer, size_t len, void*) 
{
	write_pcm_to_i2s(pcm_buffer,len);
}

static MP3DecoderHelix *mp3;

typedef struct {
    uint8_t  *mp3;
    uint32_t len;
} mp3_data_t;

static void taskAudio(void *arg)
{
  mp3_data_t *info = (mp3_data_t *)arg;

  mp3->begin();
  mp3->write(info->mp3, info->len);   
  vTaskDelete(NULL);
}

void IOT47_audio_init()
{
  i2s_pin_config_t i2s_pdm_pins = {
  // no bck for PDM
  .bck_io_num = I2S_PIN_NO_CHANGE,
  // use a dummy pin for the LR clock - 45 or 46 is a good options
  // as these are normally not connected
  .ws_io_num = 2,
  // where should we send the PDM data
  .data_out_num = 1,
  // no data to read
  .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_init(SAMPLE_RATE,&i2s_pdm_pins);
  mp3 = new MP3DecoderHelix(dataCallback);
  mp3->setDataCallback(dataCallback);
}

void IOT47_mp3_play(uint8_t *audio, uint32_t len)
{
  mp3_data_t myMp3Info;
  myMp3Info.mp3 = audio;
  myMp3Info.len = len;

  xTaskCreate(
      taskAudio,       // Hàm task
      "AudioTask",     // Tên task (debug)
      1024,            // Stack size (tính theo word 4 byte -> 4096 = 16KB)
      &myMp3Info,      // Tham số truyền vào hàm task
      5,               // Priority (cao hơn idle = 1)
      NULL             // Handle (NULL nếu không cần quản lý)
  );
}

bool IOT47_mp3_isPlaying()
{
  return mp3->isPlaying();
}
void IOT47_mp3_stop()
{
  mp3->stop();
}
void IOT47_audio_set_volume(int volume)
{
	audio_level = volume;
}

#endif