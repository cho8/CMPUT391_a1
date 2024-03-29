/*
Q8 (15 pts)

Write a C program, in a file called q8.c that prints the list of airports with commercial flights into (or out of) them, but yet cannot be reached flying from YEG regardless of the number of connections.

This query can take a while to finish execution.
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
  
    printf("This query may take a while to run, please wait...\n");
  
    // Find all the airports YEG can reach
  	char *sql_create_yeg =
      "CREATE TEMP TABLE YEGReach AS "\
      "WITH RECURSIVE connections(Source_airport, Destination_airport) AS "\
      "( "\
      "  SELECT r.source_airport, r.destination_airport "\
      "  FROM routes r "\
      "  UNION "\
      "  SELECT r.source_airport, c.destination_airport "\
      "  FROM connections c, routes r "\
      "  WHERE c.source_airport = r.destination_airport "\
      "  ORDER BY c.destination_airport "\
      ") "\
      "SELECT distinct destination_airport "\
      "FROM connections "\
      "WHERE source_airport='YEG';";

  	rc = sqlite3_prepare_v2(db, sql_create_yeg, -1, &stmt, 0);

    if (rc != SQLITE_OK) {  
        fprintf(stderr, "Preparation failed: YEGReach %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }    
      
    if ((rc = sqlite3_step(stmt)) != SQLITE_DONE){
      fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
    }
    sqlite3_finalize(stmt); //always finalize a statement
  
    // Find all airports with commercials incoming/outgoing
    char *sql_create_comflights=
      "CREATE TEMP VIEW CommercialAirports(airport) AS "\
      "SELECT r.source_airport "\
      "FROM airports a, airlines l, routes r "\
      "WHERE l.airline_id=r.airline_id "\
      "  AND (l.IATA<>'\\N' or l.ICAO<>'\\N') "\
      "  AND l.callsign<>'\\N' "\
      "  AND l.country<>'\\N' "\
      "  AND r.source_airport_id=a.airport_id "\
      "UNION "\
      "SELECT r.destination_airport "\
      "FROM airports a, airlines l, routes r "\
      "WHERE l.airline_id=r.airline_id "\
      "  AND (l.IATA<>'\\N' or l.ICAO<>'\\N') "\
      "  AND l.callsign<>'\\N' "\
      "  AND l.country<>'\\N'; "\
      "  AND r.destination_airport_id=a.airport_id;";
  
    rc = sqlite3_prepare_v2(db, sql_create_comflights, -1, &stmt, 0);

    if (rc != SQLITE_OK) {  
        fprintf(stderr, "Preparation failed: YEGReach %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }    
      
    if ((rc = sqlite3_step(stmt)) != SQLITE_DONE){
      fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
    }
    sqlite3_finalize(stmt); //always finalize a statement
  
    // Subtract YEG reachable airports from commercial airports
    char *sql_query=
      "SELECT * FROM CommercialAirports "\
      "EXCEPT "\
      "SELECT * FROM YEGReach;";
  
    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);

    if (rc != SQLITE_OK) {  
        fprintf(stderr, "Preparation failed: YEGReach %s\n", sqlite3_errmsg(db));
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
    sqlite3_finalize(stmt);
}



/*
-- SQL QUERY
-- All airports that YEG can reach
CREATE TABLE YEGReach AS
WITH RECURSIVE connections(Source_airport, Destination_airport) AS
  (
    SELECT r.source_airport, r.destination_airport
    FROM routes r
    UNION
    SELECT r.source_airport, c.destination_airport
    FROM connections c, routes r
    WHERE c.source_airport = r.destination_airport
    ORDER BY c.destination_airport
  )
  SELECT distinct destination_airport
  FROM connections
  WHERE source_airport='YEG';
 

-- Airports with commercial flights in and out
CREATE VIEW CommercialAirports(airport) AS
SELECT r.source_airport
FROM airports a, airlines l, routes r
WHERE l.airline_id=r.airline_id
  AND (l.IATA<>'\N' or l.ICAO<>'\N')
  AND l.callsign<>'\N'
  AND l.country<>'\N'
  and r.source_airport_id=a.airport_id
UNION
SELECT r.destination_airport
FROM airports a, airlines l, routes r
WHERE l.airline_id=r.airline_id
  AND (l.IATA<>'\N' or l.ICAO<>'\N')
  AND l.callsign<>'\N'
  AND l.country<>'\N'
  AND r.destination_airport_id=a.airport_id;
  
-- All airports minus those reachable by YEG
SELECT * FROM CommercialAirports
EXCEPT
SELECT * FROM YEGReach;

*/
