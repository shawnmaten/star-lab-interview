#define STR_HELPER(s) #s
#define STR(s) STR_HELPER(s)

#define SIMPLE_DB_MSG_LEN 2561
#define SIMPLE_DB_TYP_LEN 1
#define SIMPLE_DB_KEY_LEN 512
#define SIMPLE_DB_VAL_LEN 2048

//['S'][KEY][VAL]
//['R'][KEY]
//[VAL]
