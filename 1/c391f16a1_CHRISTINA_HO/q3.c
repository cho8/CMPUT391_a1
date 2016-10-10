/*
Q3 (5 pts)

Write a C program, in a file called q3.c that prints the list of top-10 countries with the most airlines flying to/from Canada.
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
      "SELECT country, count(airline_id) "\
      "FROM "\
      "( "\
      "  SELECT a.airline_id, a.name, a.country "\
      "  FROM routes r, airports p, airlines a "\
      "  WHERE r.source_airport_id=p.airport_id "\
      "    AND p.country='Canada' "\
      "    AND r.airline_id=a.airline_id "\
      "    UNION "\
      "    SELECT a.airline_id, a.name, a.country "\
      "    FROM routes r, airports p, airlines a "\
      "    WHERE r.destination_airport_id=p.airport_id "\
      "      AND p.country='Canada' "\
      "      AND r.airline_id=a.airline_id "\
      ") "\
      "GROUP BY country "\
      "ORDER BY count(airline_id) desc "\
      "LIMIT 10;";

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
SQL QUERY
SELECT country, count(airline_id)
FROM (
      SELECT a.airline_id, a.name, a.country
      FROM routes r, airports p, airlines a
      WHERE r.source_airport_id=p.airport_id
        AND p.country='Canada'
        AND r.airline_id=a.airline_id
      UNION
      SELECT a.airline_id, a.name, a.country
      FROM routes r, airports p, airlines a
      WHERE r.destination_airport_id=p.airport_id
        AND p.country='Canada'
        AND r.airline_id=a.airline_id
    )
GROUP BY country
ORDER BY count(airline_id) desc
LIMIT 10;
*/



