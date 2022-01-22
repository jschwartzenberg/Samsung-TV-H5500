#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/prctl.h>

#define VD_DBG_UNDEFINED				0x80000000
#define VD_DBG_NONE					0x00000000

#define REL_DBG_PTHREAD_COND_TIMEDWAIT_RELATIVE		0b00000001
#define REL_DBG_SEM_TIMEDWAIT_RELATIVE			0b00000010
#define REL_DBG_PTHREAD_MUTEX_TIMEDLOCK_RELATIVE	0b00000100
#define REL_DBG_PTHREAD_RWLOCK_TIMEDRDLOCK_RELATIVE	0b00001000
#define REL_DBG_PTHREAD_RWLOCK_TIMEDWRLOCK_RELATIVE	0b00010000
#define REL_DBG_PTHREAD_TIMEDJOIN_NP_RELATIVE		0b00100000
#define REL_DBG_MQ_TIMEDRECEIVE_RELATIVE		0b01000000
#define REL_DBG_MQ_TIMEDSEND_RELATIVE			0b10000000

#define PRINT_DBG_VD_VFPRINTF				0b00001
#define PRINT_DBG_VD_VPRINTF				0b00010
#define PRINT_DBG_VD_FPRINTF				0b00100
#define PRINT_DBG_VD_PRINTF				0b01000
#define PRINT_DBG_VD_PUTS				0b10000

inline static void vd_check_debug (int* debug_field, const char* debug_env_var) {

  if (*debug_field == VD_DBG_UNDEFINED)
  {
      char buf[9];

      /* Set initial value */
      *debug_field = VD_DBG_NONE;

      int errno_backup = errno;
      if ( getenv_r (debug_env_var, buf, sizeof (buf)))
      {
          /* Do not corrupt "errno" in case of getenv_r fails */
          __set_errno (errno_backup);
          return;
      }
      else
      {
          /* Convert buf string to bit field */
          *debug_field = strtol(buf, NULL, 2);
      }
  } /* else - no check again, if *debug_field has been defined */
  return;
}

inline static void vd_out_debug (int* debug_field, const int debug_mask, const char* func_name)
{
  if ( *debug_field & debug_mask )
  {
      pid_t pid;
      char tname[16];

      pid = getpid();
      prctl (PR_GET_NAME, (unsigned long)&tname);
      fprintf(stdout, "\n##### VD-GLIBC message, %s was called :%s, pid:%d\n", func_name, tname, pid);
  }
}
