/*
 *    The headers need to be added here
 */

#include "include.h"
#include <mysql.h>


/*
 *  Here, there need to be externs to a NEW variable set in merc.h, to check and see if the
 *  mySQL server is open for connections. And a way to check to see if the mysql server is
 *  up and running at any one time.
 *  The variable would be named like: bool sql_on, if it's true, then it works, if it's 
 *  false, then it's not working
 *  Also, there needs to be a variable in the directories include to tell which code we're
 *  running.
 *  Now, here's the functions and how to use them
 *  sql_connect: this is the raw entrance, don't worry too much about it
 *  sql_disconnect: closes connection
 *  boot_database, void function to start the mysql thread
 *  connect_ping: not sure, it was from jason's code, seems useful
 *  sql_parse: parses a value to allow for special characters.
 *  sql_status: returns sql status as a string
 *  
 *  This is the update of an integer field
 *  void sql_update_int(table name,field name,table key name,integer value,row id)
 *
 *  This is the update of a character field
 *  void sql_update_char(table name,field name,table key name,character string,row id)
 *  
 *  This inserts a new row into a table:
 *  void sql_insert_row(table name,table key field,row)
 *
 *  This removes a row from a table:
 *  void sql_delete_row(table name,table key field,row id)
 *
 *  This is the counting routine for locating how many rows are in a table
 *  int sql_count_table(table name,table key)
 * 
 *  This counts the number of returned matches for a search
 *  int sql_count_search_field(table name,field to search,the match string)
 *
 *  This returns the value of an integer
 *  char sql_search_int_field(table name,field name,search number,table key,the match number to return)
 *
 *  This requires an example: 
 *  sql_search_int_field(players,age,17,p_id,2) 
 *  would return the second match (it starts at 1 and goes to the number of return searches)
 *
 *  Same as above, cept returns char value
 *  char sql_search_char_field(table,field,search string,key,match number)
 *
 *  This replaces the char value of a field, with the new string
 *  void sql_update_char_field(table,field,new string,table key,row id)
 *
 *  This replaces the int value of a field, with the new int
 *  void sql_update_int_field(table,field,new integer,table key,row id)
 *
 *  This is the retrieve command for integers
 *  int sql_get_int_field(table,field,table key,row id)
 *
 *  This is the retrieve command for strings
 *  char sql_get_int_field(table,field,table key,row id)
*/

MYSQL * sql_connect( void )
{
    	MYSQL *conn; /* pointer to connection handler */
	char stderr[MSL];
	char buf[MSL];
	
	conn = mysql_init (NULL); /* allocate, initialize connection handler */
	
	if (conn == NULL) 
	{
		sprintf(stderr,"mysql_init() failed (probably out of memory)\r\n");
		log_string(stderr);
		return (NULL);
	}
	
	if (mysql_real_connect (conn, db_host, db_user, db_pass,
		db_name, 0, NULL, 0) == NULL) 
	{
		sprintf(stderr,"mysql_real_connect() failed:\r\nError %u (%s)\r\n",
			mysql_errno (conn), mysql_error (conn));
		log_string(stderr);
		return (NULL);
	}

	sprintf(buf,"MySQL: Connected to the sql server [Thread: %ld]",	mysql_thread_id(conn));
	log_string(buf);

	return (conn);
}

void sql_disconnect(MYSQL *conn)
{
	mysql_close(conn);

	log_string("Disconnected from the sql server.");
}

MYSQL   	*conn;
MYSQL_RES  *res_set;

bool sql_ping(void)
{
	if (conn == NULL)
	{
		log_string("MySQL: Ping.");
		mysql_ping(conn);
		return TRUE;
	}
	
	return FALSE;
}

char *sql_parse(char *name)
{
   char *buf;

	buf = NULL;
	(void) mysql_escape_string (buf, name, strlen(name));
	return buf;
}

void sql_update_char(char *table, char *field,char *key, char *value, long id)
{
   char buf[MSL];

	sprintf(buf, "UPDATE %s SET %s='%s' WHERE %s=%ld",table,field,sql_parse(value),key, id);
	mysql_query(conn, buf);
	return;

}

void sql_update_int(char *table,char *field,char *key, int value, long id)
{
   char buf[MSL];
	sprintf(buf, "UPDATE %s SET %s=%d WHERE %s=%ld",table,field,value,key,id);
	mysql_query(conn, buf);
	return;
}

void sql_insert_row(char *table, char *key, long id)
{
    char buf[MSL];
        sprintf(buf,"INSERT INTO %s (%s) VALUES(%ld)",table,key,id);
        mysql_query(conn,buf);
}

void sql_delete_row(char *table, char *key, long id)
{
    char buf[MSL];
        sprintf(buf,"DELETE FROM %s WHERE %s=%ld",table,key,id);
        mysql_query(conn,buf);
}

int sql_count_table(char *table, char *key)
{
    char buf[MSL];

    int count;
    sprintf(buf,"SELECT COUNT(%s) FROM %s",key,table);
    mysql_query(conn,buf);
    res_set = mysql_store_result(conn);
    count = mysql_num_rows(res_set);
    mysql_free_result(res_set);
    return count;

}
int sql_count_search_field(char *table, char *field, char *search, char *key)
{
    char buf[MSL];

    int count;
    sprintf(buf,"SELECT COUNT(%s) FROM %s WHERE %s='%s'",key,table,field,sql_parse(search));
    mysql_query(conn,buf);
    res_set = mysql_store_result(conn);
    count = mysql_num_rows(res_set);
    mysql_free_result(res_set);
    return count;

}

int sql_search_int_field(char *table, char *field, char *value, char *key, int place)
{
    MYSQL_ROW row;
    char buf[MSL];

    int result;
    int total = 0;
    int i;
    if (place > total)
        place = total;
    if (place < 1)
        place = 1;
    place--;
    sprintf(buf,"SELECT %s FROM %s WHERE %s='%s' ORDER BY %s",field,table,field,sql_parse(value),key);
    mysql_query(conn,buf);
    res_set = mysql_store_result(conn);
    total = mysql_num_rows(res_set);
    for(i = 0; i < total; i++) {
        row = mysql_fetch_row (res_set);
        if (i == place)
            sprintf(buf,"%s",row[0]);
    }
    mysql_free_result(res_set);
    if (!buf)
        sprintf(buf,"0");
    result = atoi(buf);
    return result;
}

char *sql_search_char_field(char *table, char *field, char *value, char *key, int place)
{
    MYSQL_ROW row;
    char buf[MSL];
    char *result = '\0';
    int total = 0;
    int i;

    if (place > total)
        place = total;
    if (place < 1)
        place = 1;
    place--;
    sprintf(buf,"SELECT %s FROM %s WHERE %s='%s' ORDER BY %s",field,table,field,key,sql_parse(value));
    mysql_query(conn,buf);
    res_set = mysql_store_result(conn);
    total = mysql_num_rows(res_set);
    for(i = 0; i < total; i++) {
        row = mysql_fetch_row (res_set);
        if (i == place)
            sprintf(result,"%s",row[0]);
    }
    mysql_free_result(res_set);
    return result;
}

void sql_update_char_field(char *table, char *field, char *value, char *key, long id)
{
    char buf[MSL];

	sprintf(buf, "UPDATE %s SET %s='%s' WHERE %s='%ld'",table, field,sql_parse(value),key,id);
	mysql_query(conn,buf);
	return;
}
void sql_update_int_field(char *table, char *field, int value, char *key, long id)
{
    char buf[MSL];

    sprintf(buf, "UPDATE %s SET %s='%d' WHERE %s='%ld'",table, field,value,key,id);
    mysql_query(conn,buf);
    return;
}

int sql_get_int_field(char *table, char *field, char *key, long id)
{
    MYSQL_ROW row;
    char buf[MSL];

	int ret;
	
	sprintf(buf, "SELECT %s FROM %s WHERE %s=%ld",table,field,key,id);
	mysql_query(conn,buf);
	res_set = mysql_store_result(conn);
	row = mysql_fetch_row(res_set);
	ret = atoi(row[0]);
	mysql_free_result(res_set);
	return (ret);

}

char *sql_get_char_field(char *table, char *field, char *key, long id)
{
    char buf[MSL];
    MYSQL_ROW row;

	char *ret;
	
	sprintf(buf, "SELECT %s FROM %s WHERE %s=%ld",table,field,key,id);
	mysql_query(conn, buf);
	res_set = mysql_store_result(conn);
	row = mysql_fetch_row(res_set);
	if (row[0] != NULL) {
		ret = str_dup(row[0]);
	} else {
	ret = "NULL";
	}
	
	mysql_free_result(res_set);
	
	return ret;
}

char *sql_status( void )
{
	if (conn == NULL)
		return "No Connection";


	return mysql_stat(conn);
}

/* In progress
bool sql_player_full( CHAR_DATA *ch )
{
    if ( !sql_player_save(ch) )
	return;

    if ( !sql_clan_update(ch) )
	return;




bool sql_player_save( CHAR_DATA *ch )
{
    if (conn == NULL)
	return FALSE;

    
*/
