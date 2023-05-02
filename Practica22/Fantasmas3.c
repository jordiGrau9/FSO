
/*
Proceso de los fantasmas
*/

#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>		/* per exit() */
#include <unistd.h>		/* per getpid() */
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>		/* intptr_t per màquines de 64 bits */
#include "memoria.h"
/*
Estructura
*/

				/* definir estructures d'informacio */
typedef struct {		/* per un objecte (menjacocos o fantasma) */
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
    float r;            /* per indicar un retard relati */
	char a;				/* caracter anterior en pos. actual */
} objecte;

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int retard;		    /* valor del retard de moviment, en mil.lisegons */

/* funcio per moure un fantasma una posicio; retorna 1 si el fantasma   */
/* captura al menjacocos, 0 altrament					*/
int main(int n_args, char *ll_args[])
{
  objecte elementos[indice];
  objecte seg;
  int k, vk, nd, vd[3];
  int *p_sharedMemory, id_sharedMemory;

  /*
  SE CONECTA A LA MEMORIA COMPARTIDA Y COMPRUEBA SI ES CORRECTA
  */
  id_sharedMemory = atoi(ll_args[3]);
  *p_sharedMemory = map_mem(id_sharedMemory);	/* obtenir adres. de mem. compartida */
  if (p_sharedMemory == (int*) -1)
  {   fprintf(stderr,"proces (%d): error en identificador de memoria\n",(int)getpid());
	exit(0);
  }
  
  /*
  RECUPERAMOS EL OBJETO
  */
  sscanf(ll_args[1], "%d,%d,%d,%f,%c", &elementos[indice].f, &elementos[indice].c, &elementos[indice].d, &elementos[indice].r, &elementos[indice].a);
  fflush(stdout);
  while (*p_sharedMemory == -1)
  {
    nd = 0;
    for (k=-1; k<=1; k++)		/* provar direccio actual i dir. veines */
    {
     
      vk = (elementos[indice].d + k) % 4;		/* direccio veina */
      if (vk < 0) vk += 4;		/* corregeix negatius */
      seg.f = elementos[indice].f + df[vk]; /* calcular posicio en la nova dir.*/
      seg.c = elementos[indice].c + dc[vk];
      seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
     
      if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0'))
      { 
        vd[nd] = vk;			/* memoritza com a direccio possible */
        nd++;
      }
    }
    if (nd == 0)
    {
     
  	  elementos[indice].d = (elementos[indice].d + 2) % 4;		/* canvia totalment de sentit */
     
    }
    else
    { 
     
      if (nd == 1)			/* si nomes pot en una direccio */
      {
  	    elementos[indice].d = vd[0];			/* li assigna aquesta */
      }
      else				/* altrament */
    	  elementos[indice].d = vd[rand() % nd];		/* segueix una dir. aleatoria */
     
      seg.f = elementos[indice].f + df[elementos[indice].d];  /* calcular seguent posicio final */
      seg.c = elementos[indice].c + dc[elementos[indice].d];
      
      seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
      win_escricar(elementos[indice].f,elementos[indice].c,elementos[indice].a,NO_INV);	/* esborra posicio anterior */
      elementos[indice].f = seg.f; elementos[indice].c = seg.c; elementos[indice].a = seg.a;	/* actualitza posicio */
      win_escricar(elementos[indice].f,elementos[indice].c, (char)('1'),NO_INV);		/* redibuixa fantasma */
      if (elementos[indice].a == '0') *p_sharedMemory = 1;		/* ha capturat menjacocos */
      
    }
    win_retard(retard);
  }
  return ((void *) NULL);
}
