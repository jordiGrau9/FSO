/*****************************************************************************/
/*									                                         */
/*				     cocos0.c				                                 */
/*									                                         */
/*     Programa inicial d'exemple per a les practiques 2.1 i 2.2 de FSO.     */
/*     Es tracta del joc del menjacocos: es dibuixa un laberint amb una      */
/*     serie de punts (cocos), els quals han de ser "menjats" pel menja-     */
/*     cocos. Aquest menjacocos es representara amb el caracter '0', i el    */
/*     moura l'usuari amb les tecles 'w' (adalt), 's' (abaix), 'd' (dreta)   */
/*     i 'a' (esquerra). Simultaniament hi haura un conjunt de fantasmes,    */
/*     representats per numeros de l'1 al 9, que intentaran capturar al      */
/*     menjacocos. En la primera versio del programa, nomes hi ha un fan-    */
/*     tasma.								                                 */
/*     Evidentment, es tracta de menjar tots els punts abans que algun fan-  */
/*     tasma atrapi al menjacocos.					                         */
/*									                                         */
/*  Arguments del programa:						                             */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil1 n_col fit_tauler creq				                         */
/*		mc_f mc_c mc_d mc_r						                             */
/*		f1_f f1_c f1_d f1_r						                             */
/*									                                         */
/*     on 'n_fil1', 'n_col' son les dimensions del taulell de joc, mes una   */
/*     fila pels missatges de text a l'ultima linia. "fit_tauler" es el nom  */
/*     d'un fitxer de text que contindra el dibuix del laberint, amb num. de */
/*     files igual a 'n_fil1'-1 i num. de columnes igual a 'n_col'. Dins     */
/*     d'aquest fitxer, hi hauran caracter ASCCII que es representaran en    */
/*     pantalla tal qual, excepte el caracters iguals a 'creq', que es visua-*/
/*     litzaran invertits per representar la paret.			                 */
/*     Els parametres 'mc_f', 'mc_c' indiquen la posicio inicial de fila i   */
/*     columna del menjacocos, aixi com la direccio inicial de moviment      */
/*     (0 -> amunt, 1-> esquerra, 2-> avall, 3-> dreta). Els parametres	     */
/*     'f1_f', 'f1_c' i 'f1_d' corresponen a la mateixa informacio per al    */
/*     fantasma 1. El programa verifica que la primera posicio del menja-    */
/*     cocos o del fantasma no coincideixi amb un bloc de paret del laberint.*/
/*	   'mc_r' 'f1_r' son dos reals que multipliquen el retard del moviment.  */ 
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment del menjacocos i dels fantasmes (en ms);           */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).   */
/*									                                         */
/*  Compilar i executar:					  	                             */
/*     El programa invoca les funcions definides a 'winsuport.h', les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				                     */
/*									                                         */
/*	   $ gcc -Wall cocos0.c winsuport.o -o cocos0 -lcurses		             */
/*	   $ ./cocos0 fit_param [retard]				                         */
/*									                                         */
/*  Codis de retorn:						  	                             */
/*     El programa retorna algun dels seguents codis al SO:		             */
/*	0  ==>  funcionament normal					                             */
/*	1  ==>  numero d'arguments incorrecte 				                     */
/*	2  ==>  fitxer de configuracio no accessible			                 */
/*	3  ==>  dimensions del taulell incorrectes			                     */
/*	4  ==>  parametres del menjacocos incorrectes			                 */
/*	5  ==>  parametres d'algun fantasma incorrectes			                 */
/*	6  ==>  no s'ha pogut crear el camp de joc			                     */
/*	7  ==>  no s'ha pogut inicialitzar el joc			                     */
/*****************************************************************************/



#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>		/* per exit() */
#include <unistd.h>		/* per getpid() */
#include "winsuport.h"		/* incloure definicions de funcions propies */
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>		/* intptr_t per màquines de 64 bits */



#define MIN_FIL 7		/* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MAX_ELEMENTOS 10

				/* definir estructures d'informacio */
typedef struct {		/* per un objecte (menjacocos o fantasma) */
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
    float r;            /* per indicar un retard relati */
	char a;				/* caracter anterior en pos. actual */
} objecte;


/* variables globals */
int n_fil1, n_col;		/* dimensions del camp de joc */
char tauler[70];		/* nom del fitxer amb el laberint de joc */
char c_req;			    /* caracter de pared del laberint */

objecte mc;      		/* informacio del menjacocos */
objecte f1;			    /* informacio del fantasma 1 */

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int cocos;			/* numero restant de cocos per menjar */
int retard;		    /* valor del retard de moviment, en mil.lisegons */

/*
VAMOS A DEFINIR LA LISTA DE OBJETOS POSIBLES
LA LISTA RESERVARA MEMORIA PARA COMO MAXIMO 10 ELEMENTOS DE LOS CUALES SIMEPRE EL ULTIMO SERA EL COMECOCOS Y LOS DEMAS LOS FANTASMAS
*/
objecte elementos[MAX_ELEMENTOS];

/*
AHORA VAMOS A DEFINIR LOS THREADS AL IGUAL QUE LOS ELEMENTOS
*/
pthread_t threads[MAX_ELEMENTOS];

/*
ESTO SERA EL SEMAFORO QUE CONTROLARÁ LA EJECUCION Y LAS SECCIONES CRITICAS
*/
pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;	/* crea un sem. Global*/

/*
A CONTINUACION VAMOS A DEFINIR UNA VARIABLE GLOBAL QUE NOS SERVIRA PARA LO SIGUIENTE:
1) SERVIRA PARA SABER EL INDICE DEL COMECOCOS EN LAS ANTERIORES ESTRUCTURAS
2) PODER RECORRER LAS ESTRUCTURAS PARA INICIARLAS O USARLAS

ESTA SE INICIA A 0 POR DARLE UN VALOR CUALQUIERA YA QUE NADA MÁS EMPEZAR EL PROGRAMA SE CAMBIARA
*/
int totalElem = 0;

/*
LA SIGUIENTE VARIABLE NOS SERVIRA PARA DEFINIR SI SE ACABO EL JUEGO O NO Y PUEDE TENER 3 ESTADOS:

0 --> COMECOCOS GANAS
1 --> FANTASMAS GANAN
2 --> JUGADOR APRIETA RETURN
*/
/*Prueba*/
int condicion = -1;


/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins d'un fitxer de text, el nom del qual es passa per referencia a  */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris al principi del programa).		    */
void carrega_parametres(const char *nom_fit)
{
  FILE *fit;

  fit = fopen(nom_fit,"rt");		/* intenta obrir fitxer */
  if (fit == NULL)
  {	
    fprintf(stderr,"No s'ha pogut obrir el fitxer \'%s\'\n",nom_fit);
  	exit(2);
  }

  if (!feof(fit)) fscanf(fit,"%d %d %s %c\n",&n_fil1,&n_col,tauler,&c_req);
  else 
  {
	  fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
	  fclose(fit);
	  exit(2);
	}
  if ((n_fil1 < MIN_FIL) || (n_fil1 > MAX_FIL) || (n_col < MIN_COL) || (n_col > MAX_COL))
  {
	  fprintf(stderr,"Error: dimensions del camp de joc incorrectes:\n");
	  fprintf(stderr,"\t%d =< n_fil1 (%d) =< %d\n",MIN_FIL,n_fil1,MAX_FIL);
	  fprintf(stderr,"\t%d =< n_col (%d) =< %d\n",MIN_COL,n_col,MAX_COL);
	  fclose(fit);
	  exit(3);
  }

  int i = 0;
  
  if (!feof(fit))  fscanf(fit,"%d %d %d %f\n",&elementos[i].f,&elementos[i].c,&elementos[i].d,&elementos[i].r);
  else 
  {
	  fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
	  fclose(fit);
	  exit(2);
	}
 

  printf("Datos del comecocos");
  printf(" %d | %d | %d | %f \n", elementos[i].f, elementos[i].c, elementos[i].d, elementos[i].r);

  if ((elementos[i].f < 1) || (elementos[i].f > n_fil1-3) || (elementos[i].c < 1) || (elementos[i].c > n_col-2) || (elementos[i].d < 0) || (elementos[i].d > 3))
  {
	  fprintf(stderr,"Error: parametres menjacocos incorrectes:\n");
	  fprintf(stderr,"\t1 =< f1.f (%d) =< n_fil1-3 (%d)\n",elementos[i].f,(n_fil1-3));
	  fprintf(stderr,"\t1 =< f1.c (%d) =< n_col-2 (%d)\n",elementos[i].c,(n_col-2));
	  fprintf(stderr,"\t0 =< f1.d (%d) =< 3\n",elementos[i].d);
	  fclose(fit);
	  exit(4);
  }
  i++;
  if (!feof(fit))
  {
      while(!feof(fit))
      {
        fscanf(fit,"%d %d %d %f\n",&elementos[i].f,&elementos[i].c,&elementos[i].d,&elementos[i].r);
        
        if ((elementos[i].f < 1) || (elementos[i].f > n_fil1-3) || (elementos[i].c < 1) || (elementos[i].c > n_col-2) || (elementos[i].d < 0) || (elementos[i].d > 3))
        {
	        fprintf(stderr,"Error: parametres fantasma 1 incorrectes:\n");
	        fprintf(stderr,"\t1 =< f1.f (%d) =< n_fil1-3 (%d)\n",elementos[i].f,(n_fil1-3));
	        fprintf(stderr,"\t1 =< f1.c (%d) =< n_col-2 (%d)\n",elementos[i].c,(n_col-2));
	        fprintf(stderr,"\t0 =< f1.d (%d) =< 3\n",elementos[i].d);
	        fclose(fit);
	        exit(5);
        }
         printf("\nDatos del fantasma numero %d ", i);
          printf(" %d | %d | %d | %f \n", elementos[i].f, elementos[i].c, elementos[i].d, elementos[i].r);

        i++;
      }
  }else 
  {
	  fprintf(stderr,"Falten parametres al fitxer \'%s\'\n",nom_fit);
	  fclose(fit);
	  exit(2);
	}
  fclose(fit);			/* fitxer carregat: tot OK! */
  
  totalElem = i;
 
  printf("Elementos totales en juego %d | %d\n\n", totalElem, i);
  printf("Joc del MenjaCocos\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  printf("prem una tecla per continuar:\n");
  getchar();
}




/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
  int r,i,j;
  char strin[12];
  int aux=0;

  r = win_carregatauler(tauler,n_fil1-1,n_col,c_req);
  if (r == 0)
  {
  
    elementos[0].a = win_quincar(elementos[0].f,elementos[0].c);
    
    if (elementos[0].a == c_req) r = -6;		/* error: menjacocos sobre pared */
    else
    {
      for (int z=1; z<totalElem;z++)
      {
       
        elementos[z].a = win_quincar(elementos[z].f,elementos[z].c);
        
        if (elementos[z].a == c_req) aux = 1;
      }
       
      if (aux == 1) r = -7;	/* error: fantasma sobre pared */
      else
      {
	      cocos = 0;			/* compta el numero total de cocos */
	      for (i=0; i<n_fil1-1; i++)
	        for (j=0; j<n_col; j++)
	          if (win_quincar(i,j)=='.') cocos++;
              /*
              win_escricar(elementos[0].f,elementos[0].c,'0',NO_INV); 
              */
              
              for(int z=1; z<totalElem;z++)
              {
                printf("Numero de fantasma a pintar %d", z);
                win_escricar(elementos[z].f,elementos[z].c,'1',NO_INV);
              }
              
              
	            

            if (elementos[0].a == '.') cocos--;	/* menja primer coco */

	        sprintf(strin,"Cocos: %d", cocos); win_escristr(strin);
       }
    }
  }

  if (r != 0)
  {	win_fi();
	fprintf(stderr,"Error: no s'ha pogut inicialitzar el joc:\n");
	switch (r)
	{ case -1: fprintf(stderr,"  nom de fitxer erroni\n"); break;
	  case -2: fprintf(stderr,"  numero de columnes d'alguna fila no coincideix amb l'amplada del tauler de joc\n"); break;
	  case -3: fprintf(stderr,"  numero de columnes del laberint incorrecte\n"); break;
	  case -4: fprintf(stderr,"  numero de files del laberint incorrecte\n"); break;
	  case -5: fprintf(stderr,"  finestra de camp de joc no oberta\n"); break;
	  case -6: fprintf(stderr,"  posicio inicial del menjacocos damunt la pared del laberint\n"); break;
	  case -7: fprintf(stderr,"  posicio inicial del fantasma damunt la pared del laberint\n"); break;
	}
	exit(7);
  }
}




/* funcio per moure un fantasma una posicio; retorna 1 si el fantasma   */
/* captura al menjacocos, 0 altrament					*/
void *mou_fantasma(void *index)
{
  objecte seg;
  int k, vk, nd, vd[3];
  int indice = (intptr_t) index;
  
  fflush(stdout);
  while (condicion == -1)
  {
    nd = 0;
    for (k=-1; k<=1; k++)		/* provar direccio actual i dir. veines */
    {
      pthread_mutex_lock(&mutex);
      vk = (elementos[indice].d + k) % 4;		/* direccio veina */
      if (vk < 0) vk += 4;		/* corregeix negatius */
      seg.f = elementos[indice].f + df[vk]; /* calcular posicio en la nova dir.*/
      seg.c = elementos[indice].c + dc[vk];
      seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
      pthread_mutex_unlock(&mutex);	/* obre semafor */  
      if ((seg.a==' ') || (seg.a=='.') || (seg.a=='0'))
      { 
        vd[nd] = vk;			/* memoritza com a direccio possible */
        nd++;
      }
    }
    if (nd == 0)
    {
      pthread_mutex_lock(&mutex);	/* tanca semafor */
  	  elementos[indice].d = (elementos[indice].d + 2) % 4;		/* canvia totalment de sentit */
      pthread_mutex_unlock(&mutex);	/* obre semafor */  
    }
    else
    { 
      pthread_mutex_lock(&mutex);	/* tanca semafor */
      if (nd == 1)			/* si nomes pot en una direccio */
      {
  	    elementos[indice].d = vd[0];			/* li assigna aquesta */
      }
      else				/* altrament */
    	  elementos[indice].d = vd[rand() % nd];		/* segueix una dir. aleatoria */
      pthread_mutex_unlock(&mutex);	/* obre semafor */  
      seg.f = elementos[indice].f + df[elementos[indice].d];  /* calcular seguent posicio final */
      seg.c = elementos[indice].c + dc[elementos[indice].d];
      pthread_mutex_lock(&mutex);
      seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
      win_escricar(elementos[indice].f,elementos[indice].c,elementos[indice].a,NO_INV);	/* esborra posicio anterior */
      elementos[indice].f = seg.f; elementos[indice].c = seg.c; elementos[indice].a = seg.a;	/* actualitza posicio */
      win_escricar(elementos[indice].f,elementos[indice].c, (char)('0'+indice),NO_INV);		/* redibuixa fantasma */
      if (elementos[indice].a == '0') condicion = 1;		/* ha capturat menjacocos */
      pthread_mutex_unlock(&mutex);	/* obre semafor */ 
    }
    win_retard(retard);
  //printf("Valor de a -->%c\n", elementos[indice].a);
  }
  return ((void *) NULL);
}




/* funcio per moure el menjacocos una posicio, en funcio de la direccio de   */
/* moviment actual; retorna -1 si s'ha premut RETURN, 1 si s'ha menjat tots  */
/* els cocos, i 0 altrament */
void *mou_menjacocos(void *n)
{
  char strin[99];
  objecte seg;
  int tec;
  while (condicion == -1)
  {
    //fprintf(stderr,"Antes de escoger tecla\n");
    pthread_mutex_lock(&mutex);
    tec = win_gettec();
    pthread_mutex_unlock(&mutex);
    if (tec != 0)
    pthread_mutex_lock(&mutex);	/* tanca semafor */
    switch (tec)		/* modificar direccio menjacocos segons tecla */
    {
      case TEC_AMUNT:	  elementos[0].d = 0; break;
      case TEC_ESQUER:  elementos[0].d = 1; break;
      case TEC_AVALL:	  elementos[0].d = 2; break;
      case TEC_DRETA:	  elementos[0].d = 3; break;
      case TEC_RETURN:  condicion = 2; break;
    }
    pthread_mutex_unlock(&mutex);	/* obre semafor */  
    seg.f = elementos[0].f + df[elementos[0].d];	/* calcular seguent posicio */
    seg.c = elementos[0].c + dc[elementos[0].d];
    pthread_mutex_lock(&mutex);
    seg.a = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
    pthread_mutex_unlock(&mutex);
    if ((seg.a == ' ') || (seg.a == '.'))
    {
      pthread_mutex_lock(&mutex);	/* tanca semafor */
      win_escricar(elementos[0].f,elementos[0].c,' ',NO_INV);		/* esborra posicio anterior */
      elementos[0].f = seg.f; elementos[0].c = seg.c;			/* actualitza posicio */
      elementos[0].a = win_quincar(seg.f,seg.c); 
      win_escricar(elementos[0].f,elementos[0].c,'0',NO_INV);		/* redibuixa menjacocos */
      pthread_mutex_unlock(&mutex);	/* obre semafor */ 
      if (seg.a == '.')
      {
	      cocos--;
        pthread_mutex_lock(&mutex);
	      sprintf(strin,"Cocos: %d", cocos); win_escristr(strin);
	      if (cocos == 0) condicion = 0;
        pthread_mutex_unlock(&mutex);	
      }
    }
    win_retard(retard);
  }

  return ((void *) NULL);
}



/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int rc;		/* variables locals */
  int i = 0;
  srand(getpid());		/* inicialitza numeros aleatoris */

  if ((n_args != 2) && (n_args !=3))
  {	
    fprintf(stderr,"Comanda: cocos0 fit_param [retard] [numero fantasmas]\n");
  	exit(1);
  }

  /*
  ANTES DE INICIAR LOS PARAMETROS VAMOS A RECOGER EL NUMERO DE ELEMENTOS
  TENER EN CUENTA QUE DE POR PARAMETRO SE ENTRA EL NUMERO DE FANTASMAS POR ENDE SE LE HA DE SUMAR 1 TENIENDO EN CUENTA EL COMECOCOS
  */
  pthread_mutex_init(&mutex, NULL);	/* inicialitza el semafor */
  carrega_parametres(ll_args[1]);
  printf("El numero total de elementos sera de %d\n -Fantasmas: %d\n -Comecocos: 1\n", totalElem, totalElem-1);

  if (n_args == 3) retard = atoi(ll_args[2]);
  else retard = 100;

  rc = win_ini(&n_fil1,&n_col,'+',INVERS);	/* intenta crear taulell */
  if (rc == 0)		/* si aconsegueix accedir a l'entorn CURSES */
  {
    inicialitza_joc();

    /*
    JUSTO CUANDO SE INICIA EL JUEGO Y LOS ELEMENTOS ESTAN SOBRE EL TABLERO, SE INICIAN LOS THREADS PARA QUE COMIENCEN A EJECUTARSE 
    */
   printf("Hay %d elementos\n", totalElem);
   while(i<totalElem)
   {
    if (i == 0)
    {
      if (pthread_create(&threads[i],NULL,mou_menjacocos,(void *) NULL) == 0);
    }else
    {
        if(pthread_create(&threads[i],NULL,mou_fantasma,(void *) (intptr_t) i) == 0);
    }
    i++;
   }
  /*
   UNA VEZ CREADOS SE REALIZA EL JOIN PARA QUE ESPERAR A QUE ACABEN 
   */

  i=0;
  while(i<totalElem)
  {
    pthread_join(threads[i], (void *) NULL);
    i++;
  }

  win_fi();

  if (condicion == 0)
  {
    printf("EL JUGADOR GANO");
  }else if (condicion == 1)
  {
    printf("VAYA LOS FANTASMAS GANARON");
  }else if (condicion == 2)
  {
    printf("EL JUGADOR DETUVO EL JUEGO");
  }
}
else
{	
  fprintf(stderr,"Error: no s'ha pogut crear el taulell:\n");
	switch (rc)
	{ 
    case -1: fprintf(stderr,"camp de joc ja creat!\n");
		  break;
	  case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n");
		  break;
	  case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n");
		  break;
	  case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n");
		  break;
	}
	exit(6);
  }
  pthread_mutex_destroy(&mutex); 
  return(0);
}
