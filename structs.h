#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#pragma once
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Macro for handling error codes
#define DIE(assertion, call_description)                                       \
	do {                                                                       \
		if (assertion) {                                                       \
			fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
			perror(call_description);                                          \
			exit(errno);                                                       \
			}                                                                  \
		} while (0)

enum sensor_type {
	TIRE,
	PMU
};

typedef struct {
	enum sensor_type sensor_type;
	void *sensor_data;
	int nr_operations;
	int *operations_idxs;
} sensor;

typedef struct __attribute__((__packed__)) {
	float voltage;
	float current;
	float power_consumption;
	int energy_regen;
	int energy_storage;
} power_management_unit;

typedef struct __attribute__((__packed__)) {
	float pressure;
	float temperature;
	int wear_level;
	int performace_score;
} tire_sensor;

void get_operations(void **operations);

#endif // _STRUCTS_H_
