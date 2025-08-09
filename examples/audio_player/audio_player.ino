#include "audio.h"
#include <stdio.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "IOT47_audio_play.h"
void setup() 
{
  Serial.begin(115200);

  Serial.print("Free Heap start: "); Serial.println(ESP.getFreeHeap());

  IOT47_audio_init();

  Serial.print("Free Heap end: "); Serial.println(ESP.getFreeHeap());
}

void loop() 
{
    if(!IOT47_mp3_isPlaying())
    {
       IOT47_mp3_play((uint8_t *)tieng_chuong,sizeof(tieng_chuong));
    }
    if(Serial.available())
    {
      delay(1);
      String s="";
      while(Serial.available())s+=(char)Serial.read();
      int volume=s.toInt();
      IOT47_audio_set_volume(volume);
      Serial.printf("Set volum to %i\n",volume);
    }
}
