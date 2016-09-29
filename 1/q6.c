/*
Q6 (5 pts)

Write a C program, in a file called q6.c that prints the list of airports that can be reached from YEG with at most 3 connections.

This program might take a few minutes to run
*/
#include <stdio.h>
#include <sqlite3.h>

int main(int argc, char **argv){
	sqlite3 *db; //the database
	sqlite3_stmt *stmt; //the update statement

  int rc;

  if( argc!=2 ){
    fprintf(stderr, "Usage: %s <database file> \n", argv[0]);
    return(1);
  }

  rc = sqlite3_open(argv[1], &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  // Query to find all airports reachable from YEG w/ < 3 connections
  char *sql_stmt =
    "WITH RECURSIVE connections(Source_airport, Destination_airport, connects) AS " \
    "( "\
    "  SELECT source_airport, destination_airport, 0 AS connects "\
    "  FROM routes "\
    "  UNION "\
    "  SELECT r.source_airport, c.destination_airport, c.connects+r.stops+1 "\
    "  FROM connections c, routes r "\
    "  WHERE c.source_airport = r.destination_airport "\
    "  AND c.connects+r.stops+1<3 "\
    "	) "\
    "	SELECT distinct destination_airport "\
    "	FROM connections "\
    "	WHERE source_airport='YEG';";

  rc = sqlite3_prepare_v2(db, sql_stmt, -1, &stmt, 0);
  
  if (rc != SQLITE_OK) {
      fprintf(stderr, "Preparation failed: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
  }

  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      int col;
      for(col=0; col<sqlite3_column_count(stmt)-1; col++) {
        printf("%s|", sqlite3_column_text(stmt, col));
      }
      printf("%s", sqlite3_column_text(stmt, col));
      printf("\n");
  }
  sqlite3_finalize(stmt); //always finalize a statement
}

/*
--SQL Query

WITH RECURSIVE connections(Source_airport, Destination_airport, connects) AS
  (
    SELECT source_airport, destination_airport, 0 AS connects
    FROM routes
    UNION
    SELECT r.source_airport, c.destination_airport, c.connects+r.stops+1
    FROM connections c, routes r
    WHERE c.source_airport = r.destination_airport
    AND c.connects+r.stops+1<3
  )
  SELECT distinct destination_airport
  FROM connections
  WHERE source_airport='YEG';

*/
