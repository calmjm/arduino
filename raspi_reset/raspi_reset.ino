const int reset_pin = 2;
const int int_pin = 8;
volatile boolean int_state = false;

ISR (PCINT0_vect)
{
  int_state = true;
}

void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

void setup()
{
  Serial.setTimeout(1000);
  Serial.begin(9600);
  Serial.write("Start\r\n");
  pciSetup(int_pin);
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, LOW);
}

void loop()
{
  unsigned long time=0, prev_time=0, wd_time=0;
  unsigned long interval=1*1000L, wd_interval=60*10*1000L;
  byte command=0;
  while (1)
  {
    time = millis();
    if (time < prev_time)
    {
      // overflow has happened, ignore.
      prev_time = time;
    }
    if (time - prev_time >= interval)
    {
      prev_time = time;
      if (int_state)
      {
        wd_time = time;
        Serial.write("Raspi is alive via pin change!\r\n");
        int_state = false;
      }

      if (Serial.available() > 0)
      {
        Serial.write("Command received!\r\n");
        command = Serial.read();
        switch (command)
        {
          case '1':
            wd_time = time;
            Serial.write("Raspi is alive via serial!\r\n");
            break;
          case '0':
            wd_time = time;
            Serial.write("Raspi requested reset...\r\n");
            reset();
            break;
          default:
            Serial.write("Unknown command!\r\n");
        }
      }
      else
      {
        // Serial.write("Nothing received...\r\n");
        if (time - wd_time >= wd_interval)
        {
          wd_time = time;
          Serial.write("Watchdog timeout, resetting...\r\n");
          reset();
        }
      }
    }
  }
}

void reset()
{
  digitalWrite(reset_pin, HIGH);
  delay(200);
  digitalWrite(reset_pin, LOW);
}
