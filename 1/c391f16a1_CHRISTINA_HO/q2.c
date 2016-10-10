/*
 Q2 (5 pts)

Write a C program, in a file called q2.c that prints a list of all international
flights by active and commercial (i.e., not user-added) airlines that do not 
have a reciprocal return. (A reciprocal return flight for YEG->LHR would be 
LHR->YEG by the same airline).
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

  	char *sql_stmt =
      "SELECT source_airport, destination_airport, a.name "\
      "FROM "\
      "  ( "\
      "    SELECT r1.source_airport, r1.destination_airport, r1.airline, r1.airline_id "\
      "    FROM routes r1 "\
      "    EXCEPT "\
      "    SELECT r2.source_airport, r2.destination_airport, r2.airline, r2.airline_id "\
      "    FROM routes r1, routes r2 "\
      "    WHERE r2.source_airport=r1.destination_airport "\
      "      AND r2.destination_airport=r1.source_airport "\
      "      AND r1.airline_id=r2.airline_id "\
      "  ) fl, "\
      "airlines a "\
      "WHERE (fl.airline=a.iata or fl.airline=icao) "\
      " AND fl.airline_id=a.airline_id "\
      "  AND (a.IATA<>'\\N' or a.ICAO<>'\\N') "\
      "  AND a.callsign<>'\\N' "\
      "  AND a.country<>'\\N' "\
      "  AND a.Active='Y' "\
      "  AND (fl.Airline=a.IATA or fl.Airline=a.ICAO); ";

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
-- SQL QUERY
-- Find all one way flights (all flights minus flights-with-return
-- Find subset of flights where airlines that are active=Y 
--  and non-null codes/callsigns/countries
SELECT source_airport, destination_airport, a.name
FROM (
    SELECT r1.source_airport, r1.destination_airport, r1.airline, r1.airline_id
    FROM routes r1
    EXCEPT
    SELECT r2.source_airport, r2.destination_airport, r2.airline, r2.airline_id
    FROM routes r1, routes r2
    WHERE r2.source_airport=r1.destination_airport
      AND r2.destination_airport=r1.source_airport
      AND r1.airline_id=r2.airline_id
  ) fl,
  airlines a
WHERE (fl.airline=a.iata or fl.airline=icao)
  AND fl.airline_id=a.airline_id
  AND (a.IATA<>'\N' or a.ICAO<>'\N')
  AND a.callsign<>'\N'
  AND a.country<>'\N'
  AND a.Active='Y'
  AND (fl.Airline=a.IATA or fl.Airline=a.ICAO);
*/






