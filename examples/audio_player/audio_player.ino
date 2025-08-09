#include "audio.h"
#include <stdio.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "IOT47_audio_play.h"
void setup() 
{
  Serial.begin(115200);
  IOT47_audio_init(1); //xuất âm thanh ra chân 1 ( nên gọi hàm này đầu tiên trong setup để tránh nó vô hiệu hóa 1 số chân chức năng khác mà đã được khởi tạo bên trên)
}

void loop() 
{
    if(!IOT47_mp3_isPlaying())
    {
       IOT47_mp3_play((uint8_t *)tieng_chuong,sizeof(tieng_chuong));
    }
    if(Serial.available()) //set volume (256 = nguyên bản) <256 sẽ bé hơn, >256 to hơn
    {
      delay(1);
      String s="";
      while(Serial.available())s+=(char)Serial.read();
      int volume=s.toInt();
      IOT47_audio_set_volume(volume);
      Serial.printf("Set volum to %i\n",volume);
    }
}



