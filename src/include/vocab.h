/*
	<vocab.h>
	19jul90 jm  Original code.
	22jul93 jm  Changed HEAD, BUFFER, and ENDIF into extern statements.
*/

#ifdef WIP_VOCAB 
#undef WIP_VOCAB

typedef struct pcmacros {
      char *line;
      struct pcmacros *next;
} PCMACRO;

typedef struct commands {
      char *name;	/* Name of command (lower case only!!) */
      int ncom;		/* Number of macro commands */
      LOGICAL insert;	/* TRUE means item can be inserted in command file */
      LOGICAL macro;	/* TRUE means item is a macro */
      LOGICAL delete;	/* TRUE means item can be deleted */
      PCMACRO *pcmac;	/* Pointer to macro struct if macro is TRUE */
      PCMACRO *help;	/* Pointer to macro struct for help items */
      struct commands *next; /* pointer to next command in list */
} COMMAND;

extern COMMAND *HEAD;
extern COMMAND *BUFFER;
extern COMMAND *ENDIF;

#else

typedef char PCMACRO;
typedef char COMMAND;

#endif /* WIP_VOCAB */
