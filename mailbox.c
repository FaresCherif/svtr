#include <pthread.h>
#include <malloc.h>
#include "mailbox.h"

mailbox mb_init() { /* Initialisation de la boîte */
 mailbox mb=(mailbox)malloc(sizeof(struct s_mailbox));
 mb->vide = 1;
 pthread_mutex_init(&mb->s_mb,0);
 pthread_cond_init(&mb->c_mb, 0);
 return mb;
}

void mb_send(mailbox mb, int command) { /* envoi d’un command dans la boîte mb */
 pthread_mutex_lock(&mb->s_mb); /* exclusion mutuelle d’accès au moniteur */
 mb->mb = command; mb->vide=0;
 pthread_cond_broadcast(&mb->c_mb); /*Réveil de l’éventuelle tâche en attente de command*/
 pthread_mutex_unlock(&mb->s_mb); /* fin de l’exclusion mutuelle */
}

void mb_receive(mailbox mb, int *command) { /* attente d’un command dans la boîte mb */
 pthread_mutex_lock(&mb->s_mb); /* exclusion mutuelle d’accès au moniteur */
 while (mb->vide) {pthread_cond_wait(&mb->c_mb,&mb->s_mb);} /* Opération ATOMIQUE : attente de la
condition et libération du mutex, puis attente du mutex sur signal */
 *command = mb->mb; mb->vide = 1;
 pthread_mutex_unlock(&mb->s_mb); /* fin de l’exclusion mutuelle */
}

void mb_destroy(mailbox mb){
    free(mb);
}