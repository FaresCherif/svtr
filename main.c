/*
 \file		deadreck.c
 \author	${user}
 \date		${date}
 \brief		Simple Hello World! for the Ev3
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <ev3.h>
#include <ev3_tacho.h>
#include <ev3_port.h>
#include <ev3_sensor.h>
#include <pthread.h>
#include <time.h>
#include "time_util.h"
#include "mdd.h"
#include "communication.h"
#include "myev3.h"
#include "workers.h"
#include "mailbox.h"

// Shared data and mailboxes
volatile MDD_int MDD_quit;
volatile MDD_int MDD_power;
volatile MDD_int MDD_status;
volatile mailbox mb_command;
volatile MDD_pos MDD_reset;
volatile MDD_pos MDD_position;


// TODO: declare the rest
//volatile int power;// declarer ca locallement, mettre ca dans un mdd ( module de donnees ) : MDD_wirte, MDDpower
pthread_t threadDirectCommand;
pthread_t threadAutoCommand;
pthread_t threadDeadReckoning;
pthread_t threadSend;


void init_comms() {
	MDD_quit = MDD_int_init(0);
	MDD_power = MDD_int_init(0);
	MDD_status = MDD_int_init(0);
	MDD_position = MDD_pos_init(0.0,0.0,0.0);
	MDD_reset = MDD_pos_init(0.0,0.0,0.0);

	// TODO: initialize the rest
}

/**
 * Thread sending information to the ground station
 * It should read the status, and, if modified, fprintf it on outStream
 * It should read the position and angle, and fprintf this information on outStream
 * Protocol format from robot to ground station:
 * Status: s status\n
 * Position: p x y a\n where x, y and a are integers!
 * Do not forget to fflush outStream at the end of each iteration, or data will be buffered but not sent
 * This thread should end when the application quits, and fclose the outStream socket
 */
void *sendThread(FILE * outStream) {
	int x, y, a;
	int status;
	struct timespec horloge;
	clock_gettime(CLOCK_REALTIME, &horloge);
	while (!MDD_int_read(MDD_quit)) {
		// TODO : complete this
		fflush(outStream);
		add_ms(&horloge, 200);
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &horloge, 0);
	}
	fclose(outStream);
	return 0;
}

/**
 * Thread commanding the robot in direct mode, it should wait everytime for a new command
 * As long as the application does not quit, this thread should apply the
 * received direct commands, using the current power, while updating the status
 * of the application (STATUS_STANDBY, STATUS_DIRECT_MOVE)
 * ev3dev-c functions that will be useful are:
 * set_tacho_command_inx (for commands TACHO_STOP and TACHO_RUN_DIRECT),
 * set_tacho_duty_cycle_sp in order to configure the power between -100 and 100
 */
void *directThread(void*dummy) { 
	// TODO : this is a bit long of a switch/case structure but it's fun

	int quit=0;
	int cmd = 0;
	while (!quit) {
		printf("direct\n");
		fflush(stdout); 
		mb_receive(mb_command, &cmd);
		int power = MDD_int_read(MDD_power);
		MDD_int_write(MDD_status,STATUS_DIRECT_MOVE);

		switch (cmd) 
		{
			
			case CMD_FORWARD :
				set_tacho_command_inx(MY_RIGHT_TACHO, TACHO_RUN_DIRECT);
				set_tacho_command_inx(MY_LEFT_TACHO, TACHO_RUN_DIRECT);

				set_tacho_duty_cycle_sp(MY_LEFT_TACHO,power);
				set_tacho_duty_cycle_sp(MY_RIGHT_TACHO,power);
				printf("devant\n");
				fflush(stdout); 
				break;
			case CMD_BACKWARD :
				set_tacho_command_inx(MY_RIGHT_TACHO, TACHO_RUN_DIRECT);
				set_tacho_command_inx(MY_LEFT_TACHO, TACHO_RUN_DIRECT);

				set_tacho_duty_cycle_sp(MY_LEFT_TACHO,-power);
				set_tacho_duty_cycle_sp(MY_RIGHT_TACHO,-power);
				printf("derriere\n");
				fflush(stdout); 
				break;
			case CMD_RIGHT :
				set_tacho_command_inx(MY_RIGHT_TACHO, TACHO_RUN_DIRECT);
				set_tacho_command_inx(MY_LEFT_TACHO, TACHO_RUN_DIRECT);

				set_tacho_duty_cycle_sp(MY_LEFT_TACHO,power);
				set_tacho_duty_cycle_sp(MY_RIGHT_TACHO,-power);
				printf("droite\n");
				fflush(stdout); 
				break;
			case CMD_LEFT :
				set_tacho_command_inx(MY_RIGHT_TACHO, TACHO_RUN_DIRECT);
				set_tacho_command_inx(MY_LEFT_TACHO, TACHO_RUN_DIRECT);

				set_tacho_duty_cycle_sp(MY_LEFT_TACHO,-power);
				set_tacho_duty_cycle_sp(MY_RIGHT_TACHO,power);
				printf("gauche\n");
				fflush(stdout); 
				break;
			case CMD_STOP :
				set_tacho_command_inx(MY_RIGHT_TACHO, TACHO_STOP);
				set_tacho_command_inx(MY_LEFT_TACHO, TACHO_STOP); 
				printf("stop\n");
				fflush(stdout); 
				break;
				
			// TODO: add every command treatment, think about using sscanf on buf to extract arguments
			default:
				printf("Unrecognized command: %i\n", cmd);
				
		}
		quit = MDD_int_read(MDD_quit);
	}
	
	return 0;
}

double degtorad(double deg){
	return deg * M_PI / 180.0;
}

double radtodeg(double rad){
	return rad * 180.0 / M_PI;
}
/**
 * deadrecknoning thread, should first initialize (see deadRWorkerInit)
 * and then periodically, until quit:
 * if reset has been required, reset the coords
 * call its worker (see deadRWorker)
 * and then update the shared variable of the coords
 * Note: careful, the deadRWorker assumes angles in radian, while
 * ground station assumes angles in degrees (reset, and data sent)
 */
void * deadreckoningThread(void *dummy) {
	// TODO : all by yourself
	deadRWorkerInit();
	int quit=0;
	struct timespec horloge;
	double* position;;

	double x;
	double y;
	double ang;

	while (!quit) {
		if(MDD_reset->dirty==1){
			printf("being reset %f %f %f\n",MDD_reset->x,MDD_reset->y,MDD_reset->ang);
			fflush(stdout); 
			MDD_reset->dirty=0;
		}
		position = MDD_pos_read(MDD_position);

		deadRWorker(position[0],position[1],degtorad(position[2]),&x,&y,&ang);
		MDD_pos_write(MDD_position,x,y,radtodeg(ang));

		//printf("val : %f %f %f\n",x,y,ang);
		printf("nouvelle pos : %f %f %f\n",position[0],position[1],position[2]);

		fflush(stdout); 

		quit = MDD_int_read(MDD_quit);
		
		add_ms(&horloge, 30);
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &horloge, 0);
	}
	return 0;
}

/**
 * auto move thread
 * when a target is defined, sets its mode to running (and updates the MDD status),
 * until STOP is received, or quit is received, or the target is reached with an acceptable error.
 */
void * autoThread(void *dummy) {
	// TODO : keep this as the bonus question, at the end
	return 0;
}

/**
 * The main function will be used as one of the application thread: readGroundstationThread
 * It will be in charge of initializing the ev3: ev3_init ev3_port_init ev3_tacho_init ev3_sensor_init
 * then initializing our specific ev3 ports: my_init_ev3
 * Then it will initialize every shared data and mailbox,
 * Then it will wait for ground station connection:  WaitClient
 * Then it will create the threads
 * Then it will read the ground station orders using fscanf on the inStream
 * Protocol:
 * q\n -> quit
 * p pow\n -> current power to be set to pow
 * r x y alpha\n -> current position and heading should be reset to (x,y,alpha)
 * m 0|1\n -> set the mode to MODE_DIRECT (0) or MODE_AUTO (1)
 * S\n -> stop
 * F\n -> direct Forward
 * B\n -> direct Backward
 * L\n -> direct Left
 * R\n -> direct Right
 * g x y\n -> auto goto (x,y)
 * Note : when fscanf returns EOF, this means the ground station closed the connection, and
 * the application should stop.
 * Before exiting, we should send the quitting information to the other threads,
 * in order to cleanup (stop engines, close sockets, etc.)
 * Cleanup made in this thread, after making sure that every other thread is finished (pthread_join):
 * Every motor should be stopped, inStream should be closed
 */
int main(void) {
	char cmd;


	mb_command = mb_init();
	char buf[256];
	int mode = MODE_DIRECT;
	FILE *inStream, *outStream;
	ev3_init();
	ev3_port_init();
	ev3_tacho_init();
	ev3_sensor_init();
	if (my_init_ev3()) {
		return 1;
	}
	init_comms();
	printf("Ready and waiting for incoming connection...\n");
	if (WaitClient(&outStream, &inStream)) {
		return 1;
	}
	// TODO: run the threads
	int quit = 0;
	int commande = 0;
	int power =0;
	int x;
	int y;
	int ang;
	pthread_create(&threadDirectCommand, NULL, directThread, NULL);
	pthread_create(&threadAutoCommand, NULL, autoThread, NULL);
	pthread_create(&threadDeadReckoning, NULL, deadreckoningThread, NULL);

	MDD_int_write(MDD_status,STATUS_STANDBY);

	//pthread_create(&threadSend, NULL, sendThread,  outStream);
	while (!quit) {
		if (fgets(buf,256,inStream)) {
			cmd = buf[0];
			switch (cmd) 
			{
				case 'r' :
					sscanf(buf, "r %i %i %i", &x,&y,&ang);
					printf("init position : %f %f %f\n", (double)x,(double)y,(double)ang);
					fflush(stdout);
					MDD_pos_write(MDD_reset,(double)x,(double)y,(double)ang);
					break;
				
				case 'm' :
					printf("mode");


				case 'p' :
					//printf("%i",buf[2]);
					sscanf(buf, "p %i", &power);
					printf("%i\n", power);
					fflush(stdout);
					MDD_int_write(MDD_power,power);
					break;
					// ecrire dans la boite au lettre que la commande 
					// TODO: add every command treatment, think about using sscanf on buf to extract arguments
				case 'F' :
					commande = CMD_FORWARD;
					mb_send(mb_command, commande);
					break;
				case 'B' :
					commande = CMD_BACKWARD;
					mb_send(mb_command, commande);
					break;
				case 'R' :
					commande = CMD_RIGHT;
					mb_send(mb_command, commande);
					break;
				case 'L' :
					commande = CMD_LEFT;
					mb_send(mb_command, commande);
					break;
				case 'S' :
					commande = CMD_STOP;
					mb_send(mb_command, commande);
					break;
				
				case 'q' :
					MDD_int_write(MDD_quit, 1);
					break;
				default:
					printf("Unrecognized command: %s\n", buf);
			}

		} else {
			// Connection closed
			quit = 1;
		}
	}
	MDD_int_write(MDD_quit, 1);
	// TODO: wait for threads termination (pthread_join)
	fclose(inStream);
	mb_destroy(mb_command);
	set_tacho_command_inx(MY_RIGHT_TACHO, TACHO_STOP);
	set_tacho_command_inx(MY_LEFT_TACHO, TACHO_STOP);
	ev3_uninit();
	CloseSockets();
	return 0;
}

//auto thread : va en auto a un endroit
//dead reckonning :  veut mettre à jour la pos du robot ,  lorsqu'il y a un reset, reset la position du robot. position: x,y,orientation
// thread regarde s'il  y a un reset : si oui il fait un maj en fonction de cette position, ensuite fonction d'odometry dans le fichier worker

//boite noire : entre 2 mutex