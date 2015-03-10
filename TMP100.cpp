/* 
 * File:   TMP100.cpp
 * Author: https://github.com/delcibao
 * License: gpl v2.0
 * Created on March 3, 2015, 10:26 PM
 */

#include <Arduino.h>
#include <inttypes.h>
#include "TMP100.h"
#include "I2C.h"

/**
 * Load the pointer register.
 * @param reg_name Register name. 
 * Allowed values: TMP100_TEMP_REG, TMP100_CONF_REG, TMP100_TLOW_REG, TMP100_THIGH_REG.
 * @return 0 if successful.
 */
uint8_t TMP100::select_register(tmp100_register reg_name){
    return I2c.write((uint8_t)tmp100_address, (uint8_t)reg_name);
}

/**
 * Read data from a register.
 * @param reg_name Register name.
 * Allowed values: TMP100_CONF_REG, TMP100_TLOW_REG, TMP100_THIGH_REG.
 * @return data from register.
 */
uint8_t TMP100::read_register(tmp100_register reg_name){
    uint8_t reg_value = 0xFF;
    I2c.read((uint8_t)tmp100_address,(uint8_t) reg_name, (uint8_t)1);
    reg_value = I2c.receive();
    return reg_value;
}

/**
 * Write data to a register,
 * @param reg_name Register name
 * Allowed values: TMP100_CONF_REG, TMP100_TLOW_REG, TMP100_THIGH_REG.
 * @param reg_value data to register.
 * @return  0 if successful.
 */
uint8_t TMP100::write_register(tmp100_register reg_name, uint8_t reg_value){
    return I2c.write((uint8_t)tmp100_address, (uint8_t)reg_name, (uint8_t)reg_value);
}

/**
 * Class constructor.
 * Default address of 0x48. ADDR0 = 0, ADD1 = 1.
 */
TMP100::TMP100(void) {
    tmp100_address = 0x48;
}

/**
 * Class constructor.
 * @param slave_address of the TMP100 sensor.
 */
TMP100::TMP100(uint8_t slave_address){
    tmp100_address = slave_address;
}

/**
 * Initialize the i2c bus and select the temperature register.
 */
void TMP100::init(void){
    I2c.begin();
    select_register(TMP100_TEMP_REG);
}

/**
 * Initialize the i2c bus and select the temperature register.
 * @param res_value desired resolution for temperature reading.
 */
void TMP100::init(tmp100_resolution res_value){
    I2c.begin();
    resolution(res_value);
    select_register(TMP100_TEMP_REG);
}

/**
 * Initialize the i2c bus and select the temperature register.
 * @param res_value desired resolution for temperature reading.
 * TMP100_9bits = 0, TMP100_10bits, TMP100_11bits, TMP100_12bits.
 * @param mode_value shutdown mode (TMP100_SHUTDOWN) or normal mode (TMP100_NORMAL). 
 */
void TMP100::init(tmp100_resolution res_value, tmp100_sd mode_value ){
    I2c.begin();
    shutdown_mode_bit = mode_value;
    resolution(res_value);
    shutdown_mode(mode_value);
    select_register(TMP100_TEMP_REG);
}

/**
 * Read configuration register.
 * @return data of configuration register.
 */
uint8_t TMP100::configuration_register(void){
    uint8_t reg_value = 0xFF;
    I2c.read((uint8_t)tmp100_address,(uint8_t)TMP100_CONF_REG, (uint8_t)1);
    reg_value = I2c.receive();
    select_register(TMP100_TEMP_REG);
    return reg_value;
}

/**
 * Write to configuration register.
 * @param reg_value data
 * @return 0 if successful.
 */
uint8_t TMP100::configuration_register(uint8_t reg_value){
    I2c.write((uint8_t)tmp100_address, (uint8_t)TMP100_CONF_REG, (uint8_t)reg_value);   
    return select_register(TMP100_TEMP_REG);;
}

/**
 * Read current temperature.
 * @return temperature in degrees Fahrenheit.
 */
float TMP100::temperature(){
    if(shutdown_mode_bit){
        uint8_t conf_value = read_register(TMP100_CONF_REG);
        write_register(TMP100_CONF_REG, conf_value | 0x10000000);   //start a single temperature conversion
        delay(350);  // 350ms
    }
    uint8_t msb, lsb;
    int16_t temp;
    I2c.read((uint8_t)tmp100_address,(uint8_t)TMP100_TEMP_REG ,(uint8_t)2);
    msb = I2c.receive();
    lsb = I2c.receive();
    temp = ((msb<<8) | lsb) >> 4;
    //The tmp100 does twos compliment but has the negative bit in the wrong spot, so test for it and correct if needed.
    if(temp & (1<<11)){
            temp |= 0xF800; //Set bits 11 to 15 to 1s to get this reading into real twos compliment.
    }
    //But if we want, we can convert this directly to a Celsius temperature reading.
    //temp *= 0.0625; //This is the same as a divide by 16
    temp /= 16; 
    float fahrenheit = (1.8 * temp) + 32;
    return fahrenheit;
}

/**
 * Read current conversion resolution.
 * @return resolution bits.
 */
uint8_t TMP100::resolution(void){
    uint8_t conf_value = read_register(TMP100_CONF_REG);
    select_register(TMP100_TEMP_REG);       // set pointer to temperature register.
    return (((conf_value & 0b01100000) >> 5)+9);
    
}

/**
 * Set conversion resolution.
 * @param res_value Desired resolution for temperature reading.
 * TMP100_9bits = 0, TMP100_10bits, TMP100_11bits, TMP100_12bits.
 * @return 0 if successful
 */
uint8_t TMP100::resolution(tmp100_resolution res_value){
    uint8_t conf_value =  read_register(TMP100_CONF_REG);
    conf_value &= 0b10011111;   // clean R1 & R0 bits of configuration register.
    write_register(TMP100_CONF_REG, conf_value | ( res_value << 5 ));
    return select_register(TMP100_TEMP_REG);  // set pointer to temperature register.
}

/**
 * Read shutdown mode bit.
 * @return shutdown mode bit.
 */
uint8_t TMP100::shutdown_mode(){
    return (uint8_t)shutdown_mode_bit;
}

/**
 * Select shutdown mode.
 * @return 0 if successful.
 */
uint8_t TMP100::shutdown_mode(tmp100_sd mode_value){
    shutdown_mode_bit = mode_value;
    uint8_t conf_value =  read_register(TMP100_CONF_REG);
    conf_value &= 0b11111110;                   // clean SD bit of configuration register.
    write_register(TMP100_CONF_REG, conf_value | (uint8_t)mode_value);
    return select_register(TMP100_TEMP_REG);    // set pointer to temperature register.
}
