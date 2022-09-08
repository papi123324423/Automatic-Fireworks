//NOTE: The ATtiny85 must be set to use the default 1-MHz internal clock

// ATtiny85 RF Transmitter
// Transmits commands to an Etekcity RF outlet
// Must know the specific codes being transmitted
// 0 bit: 188us
// 1 bit: 3x zero bit length
// Sync (start) time: 30x zero bit
// Message length: 24 data bits, 1 stop bit
#include <avr/sleep.h>

#define Serial_RF 0
#define Button1 1
#define Button2 2
#define Button3 3
#define Button4 4
#define DelayShort 188
#define DelayLong 564
#define DelaySync 5640
#define num_msgs 5 // number of messages to transmit

byte temp;

void setup ()
{
  pinMode(Serial_RF, OUTPUT);
  digitalWrite(Serial_RF, 0);
  digitalWrite (Button1, HIGH);  // enable pull-up
  digitalWrite (Button2, HIGH);  // enable pull-up
  digitalWrite (Button3, HIGH);  // enable pull-up
  digitalWrite (Button4, HIGH);  // enable pull-up

  // Set interrupt-on-change registers for buttons
  bitClear(GIMSK, INT0); // disable INT0 external interrupt
  bitSet(GIMSK, PCIE); // enable interrupt-on-change
  bitSet(PCMSK, Button1);
  bitSet(PCMSK, Button2);
  bitSet(PCMSK, Button3);
  bitSet(PCMSK, Button4);

  bitClear(ADCSRA, ADEN); // disable ADC for low power mode
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);

}  // end of setup

void loop ()
{
  sleep_enable();

  // Do not interrupt before we go to sleep, or the
  // ISR will detach interrupts and we won't wake.
  noInterrupts();

  // We are guaranteed that the sleep_cpu call will be done
  // as the processor executes the next instruction after
  // interrupts are turned on.
  interrupts();  // one cycle
  sleep_cpu();   // one cycle
} // end of loop

void Transmit1 ()
{
  temp = num_msgs;

  while (temp > 0) {
    Write_RF(0x44);
    Write_RF(0x15);
    Write_RF(0x33); // turn on outlet 1
    Stop_Bit();
    delayMicroseconds(DelaySync);
    temp--;
  }
}

void Transmit2 ()
{
  temp = num_msgs;

  while (temp > 0) {
    Write_RF(0x44);
    Write_RF(0x15);
    Write_RF(0x3C); // turn off outlet 1
    Stop_Bit();
    delayMicroseconds(DelaySync);
    temp--;
  }
}

void Transmit3 ()
{
  temp = num_msgs;

  while (temp > 0) {
    Write_RF(0x44);
    Write_RF(0x15);
    Write_RF(0xC3); // turn on outlet 2
    Stop_Bit();
    delayMicroseconds(DelaySync);
    temp--;
  }
}

void Transmit4 ()
{
  temp = num_msgs;

  while (temp > 0) {
    Write_RF(0x44);
    Write_RF(0x15);
    Write_RF(0xCC); // turn off outlet 2
    Stop_Bit();
    delayMicroseconds(DelaySync);
    temp--;
  }
}

void Write_RF (byte Data_Byte)
{
  byte i = 8; // send MSB first

  while (i > 0)
  {
    i--;
    digitalWrite(Serial_RF, 1);

    if (bitRead(Data_Byte, i) == 0)
    {
      delayMicroseconds(DelayShort);
      digitalWrite(Serial_RF, 0);
      delayMicroseconds(DelayLong);
    }
    else
    {
      delayMicroseconds(DelayLong);
      digitalWrite(Serial_RF, 0);
      delayMicroseconds(DelayShort);
    }
  }
}

void Stop_Bit ()
{
  digitalWrite(Serial_RF, 1);
  delayMicroseconds(DelayShort);
  digitalWrite(Serial_RF, 0);
  delayMicroseconds(DelayLong);
}

// Interrupt-on-change handler
ISR (PCINT0_vect)
{
  delayMicroseconds(16000); // debounce switch
  delayMicroseconds(16000); // debounce switch
  delayMicroseconds(16000); // debounce switch

  if (digitalRead(Button1) == LOW)
    Transmit1();
  else if (digitalRead(Button2) == LOW)
    Transmit2();
  else if (digitalRead(Button3) == LOW)
    Transmit3();
  else if (digitalRead(Button4) == LOW)
    Transmit4();
}