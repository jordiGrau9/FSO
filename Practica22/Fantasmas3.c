
/*
Proceso de los fantasmas
*/

#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>		/* per exit() */
#include <unistd.h>		/* per getpid() */
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include <stdbool.h>
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



/* funcio per moure un fantasma una posicio; retorna 1 si el fantasma   */
/* captura al menjacocos, 0 altrament					*/
int main(int n_args, char *ll_args[])
{
  objecte elementos;
  objecte seg;
  int k, vk, nd, vd[3], id_win;
  int *p_sharedMemory, id_sharedMemory;
  void *p_win;
  int retard = atoi(ll_args[2]);

  /*
  SE CONECTA A LA MEMORIA COMPARTIDA Y COMPRUEBA SI ES CORRECTA
  */
  id_sharedMemory = atoi(ll_args[3]);
  p_sharedMemory = map_mem(id_sharedMemory);	/* obtenir adres. de mem. compartida */
  if (p_sharedMemory == (int*) -1)
  {   fprintf(stderr,"proces (%d): error en identificador de memoria\n",(int)getpid());
	exit(0);
  }
  //mapa
  int n_fill = atoi(ll_args[5]);
  int n_col = atoi(ll_args[6]);
  //
  id_win = atoi(ll_args[4]);
  p_win = map_mem(id_win);	/* obtenir adres. de mem. compartida */

  if (p_win  == (int*) -1)
  {   fprintf(stderr,"proces (%d): error en identificador de memoria\n",(int)getpid());
	exit(0);
  }

  win_set(p_win,n_fill,n_col);		/* crea acces a finestra oberta */
  
  /*
  RECUPERAMOS EL OBJETO
  */
  sscanf(ll_args[1], "%d,%d,%d,%f,%c", &elementos.f, &elementos.c, &elementos.d, &elementos.r, &elementos.a);
  fflush(stdout);
  while (*p_sharedMemory == -1)
  {
    nd = 0;
    for (k=-1; k<=1; k++)		/* provar direccio actual i dir. veines */
    {
     
      vk = (elementos.d + k) % 4;		/* direccio veina */
      if (vk < 0) vk += 4;		/* corregeix negatius */
      seg.f = elementos.f + df[vk]; /* calcular posicio en la nova dir.*/
      seg.c = elementos.c + dc[vk];
      seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
     
      if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0'))
      { 
        vd[nd] = vk;			/* memoritza com a direccio possible */
        nd++;
      }
    }
    if (nd == 0)
    {
     
  	  elementos.d = (elementos.d + 2) % 4;		/* canvia totalment de sentit */
     
    }
    else
    { 
     
      if (nd == 1)			/* si nomes pot en una direccio */
      {
  	    elementos.d = vd[0];			/* li assigna aquesta */
      }
      else				/* altrament */
    	  elementos.d = vd[rand() % nd];		/* segueix una dir. aleatoria */
     
      seg.f = elementos.f + df[elementos.d];  /* calcular seguent posicio final */
      seg.c = elementos.c + dc[elementos.d];
      
      seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
      win_escricar(elementos.f,elementos.c,elementos.a,NO_INV);	/* esborra posicio anterior */
      elementos.f = seg.f; elementos.c = seg.c; elementos.a = seg.a;	/* actualitza posicio */
      win_escricar(elementos.f,elementos.c, (char)('1'),NO_INV);		/* redibuixa fantasma */
      if (elementos.a == '0') *p_sharedMemory = 1;		/* ha capturat menjacocos */
      
    }
    win_retard(retard);
  }
  exit(0);
}
