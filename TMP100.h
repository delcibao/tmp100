/* 
 * File:   TMP100.h
 * Author: https://github.com/delcibao
 * license: gpl v2.0
 * Created on March 3, 2015, 10:26 PM
 */
#ifndef TMP100_H
#define	TMP100_H

#include <stdint.h>

typedef enum {TMP100_9bits = 0, TMP100_10bits, TMP100_11bits, TMP100_12bits } tmp100_resolution;

typedef enum {TMP100_TEMP_REG = 0, TMP100_CONF_REG, TMP100_TLOW_REG, TMP100_THIGH_REG} tmp100_register;

typedef enum { TMP100_normal = 0, TMP100_shutdown} tmp100_sd;

//#define TMP100_ADDRESS      0x48

class TMP100 {
private:
    uint8_t tmp100_address;
    tmp100_sd shutdown_mode_bit; 
    uint8_t select_register(tmp100_register reg_name);
    uint8_t read_register(tmp100_register reg_name);
    uint8_t write_register(tmp100_register reg_name, uint8_t reg_value);
public:       
    TMP100(void);
    TMP100(uint8_t slave_address);
    void init(void);
    void init(tmp100_resolution res_value);
    void init(tmp100_resolution res_value, tmp100_sd mode_value);
    uint8_t configuration_register(void);
    uint8_t configuration_register(uint8_t reg_value);
    float temperature(void);
    uint8_t resolution(void);
    uint8_t resolution(tmp100_resolution res_value);
    uint8_t shutdown_mode(void);
    uint8_t shutdown_mode(tmp100_sd mode_vale);
};

#endif	/* TMP100_H */

