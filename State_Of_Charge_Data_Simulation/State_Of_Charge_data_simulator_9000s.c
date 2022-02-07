
// This program generates a csv file with data on a simulated state of charge (SOC)
// estimate for a vehicle. It also prints the data in the console (output).
// Time duration of data: 9000s
// Time interval of data: every 10s (this can be modified)
// Created by Ayush Saha, 2/7/2022


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#pragma warning (disable:4996)

// Linked list struct
struct node {
	int x;
	double y;
	struct node* next;
};

// WE MUST HAVE A POINTER TO HEAD OF LIST AND POINTER TO CURRENT ELEM
struct node* head = NULL;
struct node* current = NULL;


// INSERT LINK AT THE FIRST LOCATION
void insertFirst(int x_val, double y_val) {
	//create a link
	struct node* link = (struct node*)malloc(sizeof(struct node));

	link->x = x_val;
	link->y = y_val;

	current = head = link;
	link->next = NULL;
}

// Insert link at end
struct node* insertAtEnd(int x_val, double y_val) {
	//create a link
	struct node* link = (struct node*)malloc(sizeof(struct node));
	link->x = x_val;
	link->y = y_val;
	link->next = NULL;

	current->next = link;
	current = link;
	return link;
}

// PRINTING THE LIST
void printList() {
	struct node* ptr = head;

	while (ptr != NULL) {
		printf("%d\t%.6lf\n", ptr->x, ptr->y);
		ptr = ptr->next;
	}
}

// FINDING LENGTH OF LINKED LIST
int length() {
	int length = 0;
	struct node* current;

	for (current = head; current != NULL; current = current->next) {
		length++;
	}
	return length;
}


// Print the linked list into csv file
int print_list_into_csv(FILE** fp) {
	struct node* ptr = head;
	while (ptr != NULL) {
		fprintf(*fp,"%d,%.6lf\n", ptr->x, ptr->y);
		ptr = ptr->next;
	}
	return 0;
}

// DRIVER CODE
int main() {
	// Opening the csv file
	FILE* fp;
	fp = fopen("data_sim.csv", "w");
	if (fp == NULL) {
		printf("Can't be opened");
		return 1;
	}
	
	// Variable declarations
	int num_seconds = 9000;			// YOU CAN MODIFY num of seconds at this line!
	int t = 0, t_interval = 10;		// YOU CAN MODIFY time interval at this line!
	int t_interval_50 = t_interval * 50;
	double y = 1.0, y_prev = 1.0;
	bool start_new_list = false;

	srand(time(NULL));		// This and few lines below are for random data generator algorithm
	int random_direction;
	double random_slope, random_slope_magnification, supposed_slope;

	bool one_direction = false;		// This and few lines below are for random direction chooser algorithm
	int memorized_direction;
	int stored_t_val = t;
	int tot_num_times_one_direc_deployed = 25;
	int one_direc_rand_chooser_lim = (num_seconds / t_interval) / tot_num_times_one_direc_deployed;
	int one_direc_rand_chosen = one_direc_rand_chooser_lim / 2;

	bool done_csv_printing = false;

	// Very first value
	insertFirst(t, y);
	t += t_interval;

	// THE BIG WHILE LOOP
	while (t <= num_seconds) {
		
		// sometimes we send the data all positive for some time. This is the algorithm for that
		// randomly chose a set direction to follow
		if (rand() % (one_direc_rand_chooser_lim + 1) == one_direc_rand_chosen) {
			one_direction = true;
			stored_t_val = t;
			memorized_direction = rand() % 2;
			if (memorized_direction == 0)
				memorized_direction = -1;
		}
		// stick with that set value
		if (one_direction == true) {
			random_direction = memorized_direction;
			//printf("start");
			if (t - stored_t_val > 120) {
				one_direction = false;
				//printf("end\n");
			}
		}
		// or if not, just choose a new random direction
		else {
			random_direction = rand() % 2;
			if (random_direction == 0)
				random_direction = -1;
		}
		
		//----------------GENERATING Y-COOR.----------------

		random_slope_magnification = ((double)rand() / RAND_MAX * 1.0 - 1.0) * (-1);//float in range 0 to 1
		//printf("%.2lf\t", random_slope_magnification);
		//printf("%d\n", random_direction);

		// First interval
		if ((0 <= t && t < 1000) || (3000<=t && t<4000) || (6000<= t && t < 7000)) {
			//printf("%d", t);
			supposed_slope = -0.0003;
			random_slope = supposed_slope * ((double)(random_direction + random_slope_magnification));
			random_slope -= 0.0001;
			if (rand() % 11 == 3 && random_slope > 0.000)
				random_slope *= 1.7;
			y = (double) y_prev + (random_slope * t_interval);
		}
		// Second interval
		else if ((1000 <= t && t < 2000) || (4000<= t && t < 5000) || (7000 <= t && t < 8000)) {
			//printf("%d", t);
			supposed_slope = 0;
			random_slope = ((double)(random_direction * random_slope_magnification * 0.00015));
			if (rand() % 11 == 3 && random_slope >= -0.0003)
				random_slope *= 1.7;
			y = (double)y_prev + (random_slope * t_interval);
		}
		// Third interval
		else if ((2000 <= t && t < 2500) || (5000 <= t && t< 5500) || (8000<=t && t < 8500)) {
			//printf("%d", t);
			supposed_slope = 0.00041;
			random_slope = supposed_slope * ((double)(random_direction + random_slope_magnification));
			//random_slope -= 0.0001;
			if (rand() % 11 == 3 && random_slope <= 0.0003)
				random_slope *= 1.7;
			y = (double)y_prev + (random_slope * t_interval);
		}
		// Fourth interval
		else if ((2500 <= t && t < 3000) || (5500 <= t && t < 6000) || (8500<= t && t <9000)) {
			//printf("%d", t);
			supposed_slope = 0;
			random_slope = ((double)(random_direction * random_slope_magnification * 0.00015));
			if (rand() % 11 == 3 && random_slope >= -0.0003)
				random_slope *= 1.7;
			y = (double)y_prev + (random_slope * t_interval);
		}

		// Inserting the (t,y) coordinate into linked list
		if (start_new_list == true) {
			insertFirst(t, y);
			start_new_list = false;
		}
		else {
			insertAtEnd(t, y);
		}
		// Inserting the linked list into the csv
		if (t % (t_interval_50) == 0) {
			print_list_into_csv(&fp);
			printList();
			if (t == num_seconds) {
				done_csv_printing = true;
			}
			start_new_list = true;
			current = head;
		}
		// Important variable changes for each run of loop
		y_prev = y;
		t += t_interval;
	}
	// print last few elements into csv after while loop is over
	if (done_csv_printing == false) {
		print_list_into_csv(&fp);
		printList();
	}

	// Close csv
	fclose(fp);
	fp = NULL;
	return 0;
}
