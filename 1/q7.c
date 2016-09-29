/*
Q7 (15 pts)

Write a C program, in a file called q7.c that prints the highest airport(s) that one can reach from YEG, regardless of the number of connections.
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

		//Query for recursively recording only the higher altitude of airports
  	char *sql_stmt =
			"WITH RECURSIVE connections(Source_airport, Source_airport_id, "\
			"	Destination_airport, destination_airport_id, highest) AS "\
			"	( "\
			"  	SELECT r.source_airport, r.source_airport_id, r.destination_airport, "\
			"  	r.destination_airport_id,  a.altitude as highest "\
			"  	FROM routes r, airports a "\
			"  	WHERE r.destination_airport_id=a.airport_id "\
			"  	UNION "\
			"  	SELECT r.source_airport, r.source_airport_id, c.destination_airport, "\
			"			c.destination_airport_id,  a.altitude "\
			"  	FROM connections c, routes r, airports a "\
			"  	WHERE c.source_airport = r.destination_airport "\
			" 	  AND r.destination_airport_id=a.airport_id "\
			"     AND r.source_airport_id=a.airport_id "\
			"     AND a.altitude>c.highest "\
		  " ) "\
			"	SELECT distinct destination_airport, max(highest) "\
			" FROM connections "\
			" WHERE source_airport='YEG'; ";

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

WITH RECURSIVE connections(Source_airport, Source_airport_id,
	Destination_airport, destination_airport_id, highest) AS
  (
    SELECT r.source_airport, r.source_airport_id, r.destination_airport,
      r.destination_airport_id,  a.altitude as highest
    FROM routes r, airports a
    WHERE r.destination_airport_id=a.airport_id
    UNION
    SELECT r.source_airport, r.source_airport_id, c.destination_airport, 
      c.destination_airport_id,  a.altitude
    FROM connections c, routes r, airports a
    WHERE c.source_airport = r.destination_airport
      AND r.destination_airport_id=a.airport_id
      AND r.source_airport_id=a.airport_id
      AND a.altitude>c.highest
  )
  SELECT distinct destination_airport, max(highest)
  FROM connections
  WHERE source_airport='YEG';

*/
