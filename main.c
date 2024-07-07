#include "structs.h"

void sensor_sort(sensor **array, int n)
{
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n - i - 1; j++) {
			if (array[j]->sensor_type < array[j + 1]->sensor_type) {
				sensor *temp = array[j];
				array[j] = array[j + 1];
				array[j + 1] = temp;
			}
		}
	}
}

void print(sensor **array, int index, int n)
{
	if (index < 0 || index > n - 1) {
		printf("Index not in range!\n");
		return;
	}

	if (!array[index]->sensor_type) {
			tire_sensor *temp = (tire_sensor *)array[index]->sensor_data;
			printf("Tire Sensor\n");
			printf("Pressure: %.2f\n", temp->pressure);
			printf("Temperature: %.2f\n", temp->temperature);
			printf("Wear Level: %d%%\n", temp->wear_level);
			if (temp->performace_score) {
				printf("Performance Score: %d\n", temp->performace_score);
			} else {
				printf("Performance Score: Not Calculated\n");
			}

	} else {
			power_management_unit *temp = (power_management_unit *)
										   array[index]->sensor_data;
			printf("Power Management Unit\n");
			printf("Voltage: %.2f\n", temp->voltage);
			printf("Current: %.2f\n", temp->current);
			printf("Power Consumption: %.2f\n", temp->power_consumption);
			printf("Energy Regen: %d%%\n", temp->energy_regen);
			printf("Energy Storage: %d%%\n", temp->energy_storage);
	}
}

void analyze(sensor **array, int index, int n)
{
	if (index < 0 || index > n - 1) {
		printf("Index not in range!");
		return;
	}

	void *operations[8];
	get_operations(operations);

	for (int i = 0; i < array[index]->nr_operations; i++) {
		// Extract pointer to function
		void (*function)() = (void (*)())
							  operations[array[index]->operations_idxs[i]];
		function(array[index]->sensor_data); // Calling the function
	}

}

void clear(sensor ***array, int *n)
{
	for (int i = 0; i < *n; i++) {
		if (!(*array)[i]->sensor_type) { // TIRE
			tire_sensor *t = (tire_sensor *)(*array)[i]->sensor_data;
			if (t->wear_level < 0 || t->wear_level > 100 ||
				t->temperature < 0 || t->temperature > 120 || t->pressure < 19
				|| t->pressure > 28) {
				free((*array)[i]->sensor_data);
				free((*array)[i]->operations_idxs);
				free((*array)[i]);
				for (int j = i; j < *n - 1; j++) {
					(*array)[j] = (*array)[j + 1];
				}
				(*n)--;
				(*array) = realloc((*array), (*n) * sizeof(sensor *));
				i--;
			}
		} else { // PMU
			power_management_unit *t = (power_management_unit *)
										   (*array)[i]->sensor_data;
			if (t->voltage < 10 || t->voltage > 20 || t->current < -100 ||
				t->current > 100 || t->power_consumption < 0 ||
				t->power_consumption > 1000 || t->energy_regen < 0 ||
				t->energy_regen > 100 || t->energy_storage < 0 ||
				t->energy_storage > 100) {
				free((*array)[i]->sensor_data);
				free((*array)[i]->operations_idxs);
				free((*array)[i]);
				for (int j = i; j < *n - 1; j++) {
					(*array)[j] = (*array)[j + 1];
				}
				(*n)--;
				(*array) = realloc((*array), (*n) * sizeof(sensor *));
				i--;
			}
		}
	}
}

void dealloc_all(sensor **array, int n)
{
	for (int i = 0; i < n; i++) {
		free(array[i]->sensor_data);
		free(array[i]->operations_idxs);
		free(array[i]);
	}
	free(array);
}

int main(int argc, char const *argv[])
{
	DIE(argc != 2, "Invalid number of parameters in the command line");

	// Opens binary file with read permissions
	FILE *in = fopen(argv[1], "rb");
	DIE(!in, "Can't open the binary file");

	int n;
	// Reads the number of sensors and allocate a vector of so many pointers
	fread(&n, sizeof(int), 1, in);
	sensor **array = malloc(n * sizeof(sensor *));
	DIE(!array, "Malloc for the array of sensors failed");

	// Allocates a sensor structure for each pointer
	for (int i = 0; i < n; i++) {
		array[i] = malloc(sizeof(sensor));
		DIE(!array[i], "Malloc for the sensor structs failed");
	}

	for (int i = 0; i < n; i++) {
		// Reads the sensor type and puts it in the socket
		fread(&(array[i]->sensor_type), sizeof(int), 1, in);

		if (array[i]->sensor_type) { // PMU
			array[i]->sensor_data = malloc(sizeof(power_management_unit));
			DIE(!array[i]->sensor_data, "Malloc for PMU structure failed");
			power_management_unit *temp = (power_management_unit *)
										   array[i]->sensor_data;

			fread(&(temp->voltage), sizeof(float), 1, in);
			fread(&(temp->current), sizeof(float), 1, in);
			fread(&(temp->power_consumption), sizeof(float), 1, in);
			fread(&(temp->energy_regen), sizeof(int), 1, in);
			fread(&(temp->energy_storage), sizeof(int), 1, in);

		} else { // TIRE
			array[i]->sensor_data = malloc(sizeof(tire_sensor));
			DIE(!array[i]->sensor_data, "Malloc for TIRE structure failed");
			tire_sensor *temp = (tire_sensor *)array[i]->sensor_data;

			fread(&(temp->pressure), sizeof(float), 1, in);
			fread(&(temp->temperature), sizeof(float), 1, in);
			fread(&(temp->wear_level), sizeof(int), 1, in);
			fread(&(temp->performace_score), sizeof(int), 1, in);
		}

		// Reads the operation number and assigns it in the structure
		fread(&array[i]->nr_operations, sizeof(int), 1, in);
		// Reads the operations and allocate space for them
		array[i]->operations_idxs = malloc(array[i]->nr_operations
										   * sizeof(int));
		DIE(!array[i]->operations_idxs, "Malloc for the operations failed");
		fread(array[i]->operations_idxs, sizeof(int),
			  array[i]->nr_operations, in);
	}
	fclose(in);

	// Sort sensor vector
	sensor_sort(array, n);

	int index, dealloc = 0;
	char *command;
	while (!dealloc) {
		scanf("%ms", &command);
		if (!strcmp(command, "print")) {
			scanf("%d", &index);
			print(array, index, n);
		} else if (!strcmp(command, "analyze")) {
			scanf("%d", &index);
			analyze(array, index, n);
		} else if (!strcmp(command, "clear")) {
			clear(&array, &n);
		} else { // exit
			dealloc_all(array, n);
			dealloc = 1;
		}
		free(command);
	}

	return 0;
}
