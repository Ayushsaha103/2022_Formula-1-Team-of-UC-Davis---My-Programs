
// This program generates a csv file with estimates of these parameters for a
// vehicle's Battery w/ respect to t (time, in s):
//		the actual SOC (state of charge)
//		Vc (voltage across capacitor, in Coulombs)
//		V (measured voltage, in Coulombs)
//		I (current, in Amps)
//
// (Data generation method follows Kirchoff's Voltage Law).
// Time duration of data: 100s. Time interval of data: every 0.01s.
// Created by Ayush Saha and Andrey Yakymovych


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#pragma warning (disable:4996)

// time variable declarations
double dt = 0.01;
int num_seconds = 100+1; // max duration of sim data (+ 1 added second for safety)
double t = 0;

// circuit model parameters (constant)
double Cc = 2400;
double R0 = 0.01;
double Rc = 0.015;
double Cbat = 18000; // units in Amp * s
double alpha = 0.7; // slope of VOC(SOC) function
// open-circuit voltage (measured)
// needs to match expected Voc according to initial SOC
long double voc0 = 3.435;

// State variables
long double SOCk = 1.0;

// v_c: voltage across battery dynamics
long double vck = 0;
long double vk = 0;

// next values of state variables
long double SOCkp1;
long double vckp1;
long double vkp1;

// current load function options
double get_It_constant(double t);
double get_It_toggle(double t);
double get_It_piecewise(double t);
// current load function and value
double (*func_It)(double) = &get_It_toggle; // choose current function here
double It;


// Linked list struct
struct node {
	double t;
	long double soc;
	long double vc;
	long double v;
	double i;
	struct node* next;
};

// WE MUST HAVE A POINTER TO HEAD OF LIST AND POINTER TO CURRENT ELEM
struct node* head = NULL;
struct node* current = NULL;


// INSERT LINK AT THE FIRST LOCATION
void insertFirst(double t_val, long double soc_val, long double vc_val, long double v_val, double i_val) {
	//create a link
	struct node* link = (struct node*)malloc(sizeof(struct node));

	link->t = t_val;
	link->soc = soc_val;
	link->vc = vc_val;
	link->v = v_val;
	link->i = i_val;

	current = head = link;
	link->next = NULL;
}

// Insert link at end
struct node* insertAtEnd(double t_val, long double soc_val, long double vc_val, long double v_val, double i_val) {
	//create a link
	struct node* link = (struct node*)malloc(sizeof(struct node));
	link->t = t_val;
	link->soc = soc_val;
	link->vc = vc_val;
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
		printf("%.4f\t%.9Lf\t%.9Lf\t%.9Lf\t%.9f\n", ptr->t, ptr->soc, ptr->vc, ptr->v, ptr->i);
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
		fprintf(*fp, "%.9f,%.9Lf,%.9Lf,%.9Lf,%.9f\n", ptr->t, ptr->soc, ptr->vc, ptr->v, ptr->i);
		ptr = ptr->next;
	}
	return 0;
}

// Discrete solution to
//
//     d/dt(soc) = - It / Cbat
//     d/dt(vc) = (It / Cc) - vc / (Cc * Rc)
//
// Using finite difference (x_{k+1} - x_{k})/dt approx d/dt(x)
long double func_SOCkp1(long double SOCk, double dt, double It, double Cbat) {
	long double SOCkp1 = SOCk - dt * (It / Cbat);
	return SOCkp1;
}
long double func_vckp1(long double vck, double dt, double It, double Cc, double Rc) {
	long double vckp1 = vck + dt * ((It / Cc) - (vck / (Cc * Rc)));
	return vckp1;
}
long double func_vkp1(long double SOCk, long double vck, double It) {
	long double voc_soc = alpha * SOCk + voc0; // could be LUT as well
	long double vkp1 = voc_soc - vck - R0 * It;
	return vkp1;
}


// SECTION: options for getting current load: It
// all should have int t as an argument

// Constant
double get_It_constant(double t) {
	return 100;
}

// Toggle on/off
double get_It_toggle(double t) {
	t = (int)t % 10;
	if (t < 5) {
		return 100;
	}
	else {
		return 0;
	}
}

// Piecewise function
double get_It_piecewise(double t) {
	t = ((int)t % 100);
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

// END SECTION: options for getting current load: It


// DRIVER CODE
int main() {
	bool start_new_list = false;
	bool done_csv_printing = false;
	int while_loop_iterations_count = 0, linked_list_len = 50;

	// Very first value
	It = (*func_It)(0);
	insertFirst(t, SOCk, vck, voc0, It);
	t += dt;

	// Opening the csv file
	FILE* fp;
	fp = fopen("SOC_data_sim.csv", "w");
	if (fp == NULL) {
		printf("Can't be opened");
		return 1;
	}

	// THE BIG WHILE LOOP
	while (t <= num_seconds) {

		//----------------GENERATING Y-COOR.----------------

		// get current value at time t
		It = (*func_It)(t);

		// Calculate next SOC and Vc using discrete solution
		SOCkp1 = func_SOCkp1(SOCk, dt, It, Cbat);
		vckp1 = func_vckp1(vck, dt, It, Cc, Rc);
		vkp1 = func_vkp1(SOCkp1, vckp1, It);

		// add noise to It and vc
		// KF should get imperfect data
		It += (rand() % 100) / 5.0 - 10;
		vkp1 += (rand() % 100) / 500.0 - 0.1;


		//-----------------DEALING W/ LINKED LIST----------------

		// Inserting the simulated values for t,soc,Vc,V,I into linked list
		if (start_new_list == true) {
			insertFirst(t, SOCkp1, vckp1, vkp1, It);
			start_new_list = false;
		}
		else {
			insertAtEnd(t, SOCkp1, vckp1, vkp1, It);
		}

		// Inserting the linked list into the csv
		if (while_loop_iterations_count % linked_list_len == 0) {
			print_list_into_csv(&fp);
			printList();
			if (t == num_seconds) {
				done_csv_printing = true;
			}
			start_new_list = true;
			current = head;
		}

		//---------------------Variable Updates--------------------
		SOCk = SOCkp1;
		vck = vckp1;
		vk = vkp1;
		t += dt;
		while_loop_iterations_count += 1;
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
