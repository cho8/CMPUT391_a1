/*
Q4 (10 pts)

Write a C program, in a file called q4.c that prints the list of top-10 lengthiest commercial flights (measured by the geographical distance between the respective airports). See https://en.wikipedia.org/wiki/Geographical_distance.


*/

#include <stdio.h>
#include <sqlite3.h>
#include <math.h>

// The following methods are referenced from
// http://www.geodatasource.com/developers/c
// with a modification to only return distance in kilometers
#define pi 3.14159265358979323846
double deg2rad(double deg) {
  return (deg * pi / 180);
}
double rad2deg(double rad) {
  return (rad * 180 / pi);
}
double geo_distance(double lat1, double lon1, double lat2, double lon2) {
  double theta, dist;
  theta = lon1 - lon2;
  dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
  dist = acos(dist);
  dist = rad2deg(dist);
  dist = dist * 60 * 1.1515;

  dist = dist * 1.609344;
  return (dist);
}

int main(int argc, char **argv){
	sqlite3 *db; //the database
	sqlite3_stmt *stmt; //the update statement
  sqlite3_stmt *stmt_ins;

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
    
  // Create intermediate table to store distances
  char *sql_updt_distances =
    "CREATE TEMP TABLE distances( "\
    "  Airline_ID CHAR[4], "\
    "  Source_airport CHAR[4], "\
    "  Destination_airport CHAR[4], "\
    "  Distance NUMERIC);";

  rc = sqlite3_prepare_v2(db, sql_updt_distances, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
      fprintf(stderr, "Preparation failed: CREATE TABLE distances %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
  }
  if ((rc = sqlite3_step(stmt)) != SQLITE_DONE){
    fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 1;
  }
  sqlite3_finalize(stmt);

  
  // Query for all combinations of airports in routes with coordinates
  char *sql_stmt_all_flights =
    "SELECT r.airline_id, r.source_airport, r.destination_airport, a1.latitude, a1.longitude, " \
    "  a2.latitude, a2.longitude " \
    "FROM routes r, airports a1, airports a2 " \
    "WHERE r.source_airport_id=a1.airport_id " \
    "  AND r.destination_airport_id=a2.airport_id;";
  
  // Query to insert into intermediate table
  char *sql_ins =
    "INSERT INTO distances VALUES( '%s', '%s', '%s', '%s');";
  

  rc = sqlite3_prepare_v2(db, sql_stmt_all_flights, -1, &stmt, 0);
  
  if (rc != SQLITE_OK) {
      fprintf(stderr, "Preparation failed: ALL FLIGHTS %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
  }
  
  char *errorMessage;
  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &errorMessage);
  
  while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    int col;
    float lat1, lon1, lat2, lon2;
    const unsigned char *airline,*src, *dest;
    double dist = 0;
    char dist_text[50];
      
    // first get list of flights with lats and lons
    for(col=0; col<sqlite3_column_count(stmt)-1; col++) {
      
      switch (col) {
        case 0 :
          airline = sqlite3_column_text(stmt,col); break;
        case 1 :
          src = sqlite3_column_text(stmt, col); break;
        case 2 :
          dest = sqlite3_column_text(stmt, col); break;
        case 3 :
          lat1 = sqlite3_column_double(stmt,col); break;
        case 4 :
          lon1 = sqlite3_column_double(stmt,col); break;
        case 5 :
          lat2 = sqlite3_column_double(stmt,col); break;
      }
    }
    lon2 = sqlite3_column_double(stmt, col);
      
    // then calculate geo-distance
    dist = geo_distance(lat1,lon1,lat2,lon2);
    snprintf(dist_text,50,"%f", dist);
  
    char ins_text[80];
    sprintf(ins_text, sql_ins, airline, src, dest,dist_text);

    // Insert into intermediate table
    rc = sqlite3_prepare_v2(db, ins_text, -1, &stmt_ins, 0);
    if (rc != SQLITE_OK) {
      fprintf(stderr, "Preparation failed: INSERT DISTANCES %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
    }
    if ((rc = sqlite3_step(stmt_ins)) != SQLITE_DONE){
      fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return 1;
    }
    sqlite3_finalize(stmt_ins);
  }
  sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &errorMessage);
  sqlite3_finalize(stmt); //always finalize a statement
  
  // Query to retrieve top 10 longest commercial flights
  char* stmt_query =
    "SELECT d.source_airport, d.destination_airport , a.name, d.distance "\
    "FROM routes r, airlines a, distances d "\
    "WHERE r.airline_id=d.airline_id "\
    "  AND r.airline_id=a.airline_id "\
    "  AND r.source_airport=d.source_airport "\
    "  AND r.destination_airport=d.destination_airport "\
    "  AND (a.IATA<>'\\N' or a.ICAO<>'\\N') "\
    "  AND a.callsign<>'\\N' "\
    "  AND a.country<>'\\N' "\
    "ORDER BY d.distance desc "\
    "LIMIT 10;";
  
  // Retrieve top flights with longest distances
  rc = sqlite3_prepare_v2(db, stmt_query, -1, &stmt, 0);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Preparation failed: TOP 10 LONGEST %s\n", sqlite3_errmsg(db));
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

CREATE TEMP TABLE distances(
  Airline_ID CHAR[4],
  Source_airport CHAR[4],
  Destination_airport CHAR[4],
  Distance NUMERIC);

SELECT r.airline_id, r.source_airport, r.destination_airport, a1.latitude, a1.longitude, a2.latitude, a2.longitude
FROM routes r, airports a1, airports a2
WHERE r.source_airport_id=a1.airport_id
  AND r.destination_airport_id=a2.airport_id;
  
INSERT INTO distances VALUES( %s, %s, %s, %s);

SELECT d.source_airport, d.destination_airport , a.name, d.distance
FROM routes r, airlines a, distances d
WHERE r.airline_id=d.airline_id
  AND r.airline_id=a.airline_id
  AND r.source_airport=d.source_airport
  AND r.destination_airport=d.destination_airport
  AND (a.IATA<>'\N' or a.ICAO<>'\N')
  AND a.callsign<>'\N'
  AND a.country<>'\N'
ORDER BY d.distance desc
LIMIT 10;

*/

