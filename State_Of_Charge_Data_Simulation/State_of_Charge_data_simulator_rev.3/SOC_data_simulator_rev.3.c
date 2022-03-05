
// This program generates a csv file with estimates of these parameters for a
// vehicle's Battery w/ respect to time: state of charge (SOC), voltage (V),
// current (I(t)).
// It also prints the data in the console (output).
// Time duration of data: 100s. Time interval of data: every 1s.
// Created by Ayush Saha, 2/14/2022, edited by Andrey Yakymovych


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#pragma warning (disable:4996)

// time variable declarations
int dt = 1;
int num_seconds = 100; // max duration of sim data
int t = 0;

// circuit model parameters
double Cc = 2400;
double Rc = 0.015;
double Cbat = 18000; // units in Amp * s

// State variables
long double SOCk = 1.0;
long double vk = 0.0;
// next values of state variables
long double SOCkp1;
long double vkp1;

// current load functions
double get_It_constant(int t);
double get_It_toggle(int t);
double get_It_piecewise(int t);
// current load function and value
double (*func_It)(int) = &get_It_piecewise; // choose current function here
double It;

// Linked list struct
struct node {
	int t;
	long double soc;
	long double v;
	double i;
	struct node* next;
};

// WE MUST HAVE A POINTER TO HEAD OF LIST AND POINTER TO CURRENT ELEM
struct node* head = NULL;
struct node* current = NULL;


// INSERT LINK AT THE FIRST LOCATION
void insertFirst(int t_val, long double soc_val, long double v_val, double i_val) {
	//create a link
	struct node* link = (struct node*)malloc(sizeof(struct node));

	link->t = t_val;
	link->soc = soc_val;
	link->v = v_val;
	link->i = i_val;

	current = head = link;
	link->next = NULL;
}

// Insert link at end
struct node* insertAtEnd(int t_val, long double soc_val, long double v_val, double i_val) {
	//create a link
	struct node* link = (struct node*)malloc(sizeof(struct node));
	link->t = t_val;
	link->soc = soc_val;
	link->v = v_val;
	link->i = i_val;
	link->next = NULL;

	current->next = link;
	current = link;
	return link;
}

// PRINTING THE LIST
void printList() {
	struct node* ptr = head;

	while (ptr != NULL) {
		printf("%d\t\t%.9Lf\t%.9Lf\t%.9f\n", ptr->t, ptr->soc, ptr->v, ptr->i);
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
		fprintf(*fp, "%d,%.9Lf,%.9Lf,%.9f\n", ptr->t, ptr->soc, ptr->v, ptr->i);
		ptr = ptr->next;
	}
	return 0;
}

// The below SECTION estimates the change in SOC and V for each second
// Discrete solution to
//
//     d/dt(soc) = - I(t) / Cbat
//     d/dt(v) = (It / Cc) - v / (Cc * Rc)
//
// Using finite difference (x_{k+1} - x_{k})/dt approx d/dt(x)
long double func_SOCkp1(long double SOCk, int dt, double It, double Cbat) {
	long double SOCkp1 = SOCk - dt * (It / Cbat);
	return SOCkp1;
}

long double func_vkp1(long double vk, int dt, double It, double Cc, double Rc) {
	long double vkp1 = vk + dt * ((It / Cc) - (vk / (Cc * Rc)));
	return vkp1;
}
// END of section


// SECTION below: the options for simulating current load: I(t)
// All of these functions should have int t as an argument

// Return constant I(t)
double get_It_constant(int t) {
	return 100;
}

// Toggle I(t) on/off
double get_It_toggle(int t) {
	t = t % 10;
	if (t < 5) {
		return 100;
	}
	else {
		return 0;
	}
}

// Piecewise function to find I(t)
double get_It_piecewise(int t) {
	// First interval
	if ((0 <= t && t < 10) || (30 <= t && t < 40) || (60 <= t && t < 70)) {
		return 300;
	}
	// Second interval
	else if ((10 <= t && t < 20) || (40 <= t && t < 50) || (70 <= t && t < 80)) {
		return 0;
	}
	// Third interval
	else if ((20 <= t && t < 25) || (50 <= t && t < 55) || (80 <= t && t < 85)) {
		return -80;
	}
	// Fourth interval
	else if ((25 <= t && t < 30) || (55 <= t && t < 60) || (85 <= t && t < 100)) {
		return 0;
	}
	return 0;
}

// END SECTION: options for getting current load: I(t)


// DRIVER CODE
int main() {
	bool start_new_list = false;
	bool done_csv_printing = false;

	// Very first value
	It = (*func_It)(0);
	insertFirst(t, SOCk, vk, It);
	t += dt;

	// Opening the csv file
	FILE* fp;
	fp = fopen("data_sim.csv", "w");
	if (fp == NULL) {
		printf("Can't be opened");
		return 1;
	}

	printf("time (s)\tSOC ((%%)/100)\tV (volts)\tI(t) (Amps)\n");

	// THE BIG WHILE LOOP
	while (t <= num_seconds) {

		//----------------GENERATING Y-COOR.----------------

		// get current value at time t
		It = (*func_It)(t);

		// Calculate next SOC and Vc using discrete solution
		SOCkp1 = func_SOCkp1(SOCk, dt, It, Cbat);
		vkp1 = func_vkp1(vk, dt, It, Cc, Rc);

		// Inserting the (t,soc,v) coordinate into linked list
		if (start_new_list == true) {
			insertFirst(t, SOCkp1, vkp1, It);
			start_new_list = false;
		}
		else {
			insertAtEnd(t, SOCkp1, vkp1, It);
		}
		// Inserting the linked list into the csv
		// only take every 5th value to reduce file size
		if (t % (dt * 5) == 0) {
			print_list_into_csv(&fp);
			printList();
			if (t == num_seconds) {
				done_csv_printing = true;
			}
			start_new_list = true;
			current = head;
		}
		// Important variable changes for each run of loop
		vk = vkp1;
		SOCk = SOCkp1;
		t += dt;
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