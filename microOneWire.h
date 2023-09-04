bool oneWire_reset(uint8_t pin) { 
    pinMode(pin, 1);
    delayMicroseconds(520);
    pinMode(pin, 0);
    delayMicroseconds(2);

    unsigned long DSTimer = micros();
    for (uint8_t c = 255; c; c--) {
        if (!digitalRead(pin)) {
            while (micros() - DSTimer < 500) {delayMicroseconds(100);}
            return true;
        }
        delayMicroseconds(1);
    }
    return false;
}

void oneWire_write(uint8_t data, uint8_t pin) {
    for (uint8_t p = 8; p; p--) {        
        pinMode(pin, 1);
        if (data & 1) {
            delayMicroseconds(5);
            pinMode(pin, 0);
            delayMicroseconds(60);
        } else {
            delayMicroseconds(60);
            pinMode(pin, 0);
            delayMicroseconds(5);
        }
        data >>= 1;
    }
}

uint8_t oneWire_read(uint8_t pin) {
    uint8_t data = 0;
    for (uint8_t p = 8; p; p--) {
        data >>= 1;
        pinMode(pin, 1);
        delayMicroseconds(2);
        pinMode(pin, 0);
        delayMicroseconds(8);
        if (digitalRead(pin)) data |= 1 << 7;
        delayMicroseconds(60);
    }
    return data;
}