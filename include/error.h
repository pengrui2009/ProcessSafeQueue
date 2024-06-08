#ifndef ERROR_H
#define ERROR_H

#define ERR_OK       0      /* no fatal */
/*************************************************
  defination of error code
*************************************************/
#define ERR_SYS             1          /* system fatal */
#define ERR_SYS_SHMGET      10         /* system api:shmget fatal */
#define ERR_SYS_SHMAT       11         /* system api:shmat fatal */
#define ERR_SYS_SHMCTRL     12         /* system api:shmctrl fatal */
#define ERR_SYS_SHMDT       13         /* system api:shmdt fatal */
#define ERR_SYS_SEMOPEN     14         /* system api:semopen fatal */
#define ERR_SYS_SEMWAIT     15         /* system api:semwait fatal */
#define ERR_SYS_SEMTIMEWAIT 16         /* system api:semtimewait fatal */
#define ERR_SYS_SEMPOST     17         /* system api:sempost fatal */
#define ERR_SYS_SEMCLOSE    18         /* system api:semclose fatal */
#define ERR_SYS_SEMDESTROY  19         /* system api:semdestroy fatal */

#define ERR_QUEUE           2          /* queue fatal */
#define ERR_QUEUE_UNDERFLOW 21         /* queue under flow fatal */
#define ERR_QUEUE_OVERFLOW  22         /* queue over flow fatal */

#define ERR_TIMEOUT         3          /* time out fatal */

#define ERR_INVAL           4          /* paramters invalid fatal */
#define ERR_INVAL_NULLPOINT 40         /* point is NULL */
#define ERR_INVAL_MEMSIZE   41         /* params size  */

#define ERR_NOINIT          5          /* no initialize fatal */
#define ERR_NOMEM           6          /* no memory fatal */
#define ERR_NODISK          7          /* no disk fatal*/
#define ERR_CFG             8          /* config fatal */

#define ERR_OTHER           80         /* others error */

#endif /* ERROR_H */