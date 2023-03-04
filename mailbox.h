/*
 * mailbox.h
 *
 *  Created on: 09 fevri. 2023
 *      Author: us(lol)
 */

#ifndef MAILBOX_H_
#define MAILBOX_H_

typedef struct s_mailbox{
    volatile int mb; /* emplacement du message */
    volatile unsigned char vide;
    pthread_mutex_t s_mb; /* mutex protégeant la boîte */
    pthread_cond_t c_mb; /* sert à bloquer/réveiller une tâche en attente de message */
}* mailbox;

mailbox mb_init();

void mb_send(mailbox mb, int command);

void mb_receive(mailbox mb, int *command);

void mb_destroy(mailbox mb);

#endif
    