# TWI/I2C Library for AVR

## Description

This library is tested on ATMega328p, but it should work with any AVR compatible microcontroller. To use with other type of ATMega, change the I2C pin definition in `twi_master.h` according the datasheet. In ATMega328p, I2C pins are describe as follows:

```c
#define TW_SCL_PIN			PORTC5
#define TW_SDA_PIN			PORTC4
```



## Usage

There are 3 functions available for this library:

```c
void tw_init(twi_freq_mode_t twi_freq, bool pullup_en);
```

Initialize I2C with predefined frequency and enable internal pull-up resistors. There are 3 mode of frequency to choose from: `TW_FREQ_100K`, `TW_FREQ_250K` and `TW_FREQ_400K`. Then set `pullup_en` bit to `true` to enable internal pull-up resistors on `SCL` and `SDA` pin, or set it to `false` if external pull-up is used.



```c
ret_code_t tw_master_transmit(uint8_t slave_addr, uint8_t* p_data, uint8_t len, bool repeat_start);
```

This function transmits data bytes to a desired slave address.  If `repeat_start`  bit is set to `true`, I2C module will send REPEATED START condition instead of STOP condition. This is useful in multi-master environment where one Master sends a command and wait for response from slave without having to release the Bus, thus preventing the other Master from taking control of the Bus in between the transaction.

`ret_code_t` is return to the user to handle the state of transmission such as transmission success, arbitration lost, slave acknowledge, etc. 



```c
ret_code_t tw_master_receive(uint8_t slave_addr, uint8_t* p_data, uint8_t len);
```

This functions receives the data bytes from the desired slave address and return `ret_code_t` for error handling as have been discussed in `tw_master_transmit`.

## Example

The example provided with this library is tested with MPU6050 to read the value of acceleration in X, Y and Z axis. This use this [UART Library](https://github.com/Sovichea/avr-uart-library) to print the result to the terminal but you can use any library and modified the library accordingly.

In order to activate MPU6050, we have to write `0` to `PWR_MGMT_1` register as shown below:

```c
void mpu_init(void)
{
	ret_code_t error_code;
	puts("Write 0 to PWR_MGMT_1 reg to wakeup MPU.");
	uint8_t data[2] = {PWR_MGMT_1, 0};
	error_code = tw_master_transmit(MPU6050_ADDR, data, sizeof(data), false);
	ERROR_CHECK(error_code);
}
```

In this situation, we want to write the value but don't read back the response; therefore, `repeat_start` bit is set to `false`. 

To read the value of accelerometer,  we have to transmit the register that we want to read, which is `ACCEL_XOUT_H` then wait for the response using `tw_master_receive` as shown below:

```c
void mpu_get_accel_raw(mpu_data_t* mpu_data)
{
	ret_code_t error_code;
	/* 2 registers for each of accel x, y and z data */
	uint8_t data[6];
	
	data[0] = ACCEL_XOUT_H;
	error_code = tw_master_transmit(MPU6050_ADDR, data, 1, true);
	ERROR_CHECK(error_code);
	
	error_code = tw_master_receive(MPU6050_ADDR, data, sizeof(data));
	ERROR_CHECK(error_code);
	
	/* Default accel config +/- 2g */
	mpu_data->x = (int16_t)(data[0] << 8 | data[1]) / 16384.0;
	mpu_data->y = (int16_t)(data[2] << 8 | data[3]) / 16384.0;
	mpu_data->z = (int16_t)(data[4] << 8 | data[5]) / 16384.0;
}
```

In this situation, since we write and then want to wait for the response from MPU, `repeat_start` bit is set to `true` to prevent other Master from pulling the Bus in between write and read process. This step is, however, not necessary if there is only one Master on the Bus.



## Debugging I2C Transaction

Found in `tw_master.h`, you can set `DEBUG_LOG` variable to `1` or `0` to enable or disable debug functionality of the library. This will print each and every I2C transaction and current state of I2C Bus.