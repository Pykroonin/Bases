/*
* Created by roberto on 3/5/21.
*/
#include <stdio.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "search.h"

void    results_search(char * from, char *to,char * date,
                       int * n_choices, char *** choices, char *** choices_info,
                       int max_length,
                       int max_rows)
   /**here you need to do your query and fill the choices array of strings
 *
 * @param from form field from
 * @param to form field to
 * @param n_choices fill this with the number of results
 * @param choices fill this with the actual results
 * @param max_length output win maximum width
 * @param max_rows output win maximum number of rows
  */
{
    int i=0;
    int t=0;
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret; /* ODBC API return status */

    SQL_TIMESTAMP_STRUCT fechita;
    unsigned short int year;
    unsigned short int month;
    unsigned short int day;
    SQLLEN num_filas;

    SQLCHAR flight_id_first[512];
    SQLCHAR scheduled_departure_first[64];
    SQLCHAR scheduled_arrival_first[64];
    SQLCHAR flight_id_second[512];
    SQLCHAR scheduled_departure_second[64];
    SQLCHAR scheduled_arrival_second[64]; 
    SQLCHAR transbordos[64];
    SQLCHAR asientos_vacios[64];
    SQLCHAR tiempo[64];
    SQLCHAR aircraft_code_first[64];
    SQLCHAR aircraft_code_second[64];

    SQLCHAR final_array[512];
    SQLCHAR final_array_info[512];
    SQLCHAR space[2] = " ";

    SQLCHAR parte1[] = "WITH vuelos_salida AS (SELECT f.flight_id, f.scheduled_departure AS fecha_sal, f.scheduled_arrival AS fecha_lleg2, f.arrival_airport, f.aircraft_code FROM flights f WHERE f.departure_airport = ? AND f.scheduled_departure >= ? AND scheduled_departure < ?::date+1),";
    SQLCHAR parte2[] = "vuelos_llegada AS (SELECT f.flight_id AS flight_id2, f.scheduled_departure AS fecha_sal2, f.scheduled_arrival AS fecha_lleg, f.departure_airport AS departure_airport2, f.aircraft_code AS aircraft_code2 FROM flights f WHERE f.arrival_airport = ? AND f.scheduled_departure > ?),";
    SQLCHAR parte3[] = "asientos_totales AS (SELECT vs.flight_id, count(s.seat_no) AS as_totales FROM seats s, vuelos_salida vs WHERE s.aircraft_code = vs.aircraft_code GROUP BY vs.flight_id UNION DISTINCT SELECT vl.flight_id2, count(s.seat_no) AS as_totales FROM seats s, vuelos_llegada vl WHERE s.aircraft_code = vl.aircraft_code2 GROUP BY vl.flight_id2),";
    SQLCHAR parte4[] = "asientos_ocupados AS (SELECT vs.flight_id, count(tf.ticket_no) AS as_ocupados FROM vuelos_salida vs, ticket_flights tf WHERE vs.flight_id = tf.flight_id GROUP BY vs.flight_id UNION DISTINCT SELECT vl.flight_id2, count(tf.ticket_no) AS as_ocupados FROM vuelos_llegada vl, ticket_flights tf WHERE vl.flight_id2 = tf.flight_id GROUP BY vl.flight_id2),";
    SQLCHAR parte5[] = "asientos_libres AS (SELECT t.flight_id, (t.as_totales - coalesce(o.as_ocupados, 0)) AS vacios FROM asientos_totales t LEFT JOIN asientos_ocupados o ON t.flight_id = o.flight_id) ";
    SQLCHAR parte6[] = "SELECT * FROM (SELECT * FROM (SELECT vs.flight_id, vs.fecha_sal, vs.fecha_lleg2, vl.flight_id2, vl.fecha_sal2, vl.fecha_lleg, '1' AS transbordos, min(al.vacios) AS asientos, (vl.fecha_lleg - vs.fecha_sal) AS tiempo, vs.aircraft_code, vl.aircraft_code2 FROM vuelos_salida vs, vuelos_llegada vl, asientos_libres al WHERE vs.arrival_airport = vl.departure_airport2 AND vs.fecha_lleg2 < vl.fecha_sal2 AND vl.fecha_lleg - vs.fecha_sal < '24:00:00.000' AND (vs.flight_id = al.flight_id OR vl.flight_id2 = ";
    SQLCHAR parte7[] = "al.flight_id) GROUP BY vs.flight_id, vs.fecha_sal, vs.fecha_lleg2, vl.flight_id2, vl.fecha_sal2, vl.fecha_lleg, vs.aircraft_code, vl.aircraft_code2) as tabla UNION ALL SELECT * FROM (SELECT f.flight_id, f.scheduled_departure, f.scheduled_arrival, f.flight_id, f.scheduled_departure, f.scheduled_arrival, '0' AS transbordos, al.vacios AS asientos, (f.scheduled_arrival - f.scheduled_departure) AS tiempo,";
    SQLCHAR parte8[] = "f.aircraft_code, f.aircraft_code FROM flights f JOIN asientos_libres al ON f.flight_id = al.flight_id WHERE f.departure_airport = ? AND f.arrival_airport = ? AND date(f.scheduled_departure) = ? AND al.vacios > '0') as resultado) as final ORDER BY tiempo;";

    SQLCHAR consulta_completa[8192]; 

    strcpy((char*)consulta_completa, (char*)parte1);
    strcat((char*)consulta_completa, (char*)parte2);
    strcat((char*)consulta_completa, (char*)parte3);
    strcat((char*)consulta_completa, (char*)parte4);
    strcat((char*)consulta_completa, (char*)parte5);
    strcat((char*)consulta_completa, (char*)parte6);
    strcat((char*)consulta_completa, (char*)parte7);
    strcat((char*)consulta_completa, (char*)parte8);



    ret = odbc_connect(&env, &dbc);

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    
    SQLPrepare(stmt, consulta_completa, SQL_NTS);
   
    sscanf(date, "%4hu_%2hu_%2hu", &year, &month, &day);
        fechita.year = year;
        fechita.month = month;
        fechita.day = day;
        fechita.hour = 0;
        fechita.minute = 0;
        fechita.second = 0;
        fechita.fraction = 0;

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, sizeof((SQLCHAR*)from), 0, (SQLCHAR*)from, sizeof((SQLCHAR*)from), NULL);
    SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &fechita, 0, NULL);
    SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &fechita, 0, NULL);
    SQLBindParameter(stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, sizeof((SQLCHAR*)to), 0, (SQLCHAR*)to, sizeof((SQLCHAR*)to), NULL);
    SQLBindParameter(stmt, 5, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &fechita, 0, NULL);
    SQLBindParameter(stmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, sizeof((SQLCHAR*)from), 0, (SQLCHAR*)from, sizeof((SQLCHAR*)from), NULL);
    SQLBindParameter(stmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, sizeof((SQLCHAR*)to), 0, (SQLCHAR*)to, sizeof((SQLCHAR*)to), NULL);
    SQLBindParameter(stmt, 8, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &fechita, 0, NULL);
    
    SQLExecute(stmt);
    

    SQLBindCol(stmt, 1, SQL_C_CHAR, flight_id_first, sizeof(flight_id_first), NULL);
    SQLBindCol(stmt, 2, SQL_C_CHAR, scheduled_departure_first, sizeof(scheduled_departure_first), NULL);
    SQLBindCol(stmt, 3, SQL_C_CHAR, scheduled_arrival_first, sizeof(scheduled_arrival_first), NULL);
    SQLBindCol(stmt, 4, SQL_C_CHAR, flight_id_second, sizeof(flight_id_second), NULL);
    SQLBindCol(stmt, 5, SQL_C_CHAR, scheduled_departure_second, sizeof(scheduled_departure_second), NULL);
    SQLBindCol(stmt, 6, SQL_C_CHAR, scheduled_arrival_second, sizeof(scheduled_arrival_second), NULL);
    SQLBindCol(stmt, 7, SQL_C_CHAR, transbordos, sizeof(transbordos), NULL);
    SQLBindCol(stmt, 8, SQL_C_CHAR, asientos_vacios, sizeof(asientos_vacios), NULL);
    SQLBindCol(stmt, 9, SQL_C_CHAR, tiempo, sizeof(tiempo), NULL);
    SQLBindCol(stmt, 10, SQL_C_CHAR, aircraft_code_first, sizeof(aircraft_code_first), NULL);
    SQLBindCol(stmt, 11, SQL_C_CHAR, aircraft_code_second, sizeof(aircraft_code_second), NULL);

    SQLRowCount(stmt, &num_filas);

    *n_choices = (int)num_filas;
    if(*n_choices<0){
        *n_choices=0;
    }
    max_rows = MIN(*n_choices, max_rows);

    while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {


        strcpy((char*)final_array, (char*)scheduled_departure_first);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)scheduled_arrival_second);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)transbordos);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)asientos_vacios);
    
        strcpy((char*)final_array_info, (char*)flight_id_first); 
        strcat((char*)final_array_info, (char*)space); 
        strcpy((char*)final_array_info, (char*)scheduled_departure_first);
        strcat((char*)final_array_info, (char*)space); 
        strcat((char*)final_array_info, (char*)scheduled_arrival_first);
        strcat((char*)final_array_info, (char*)space);
        strcat((char*)final_array_info, (char*)aircraft_code_first);
        if(atoi((char*)transbordos)==1){
            strcat((char*)final_array_info, (char*)space); 
            strcat((char*)final_array_info, (char*)flight_id_second);
            strcat((char*)final_array_info, (char*)space); 
            strcat((char*)final_array_info, (char*)scheduled_departure_second);
            strcat((char*)final_array_info, (char*)space); 
            strcat((char*)final_array_info, (char*)scheduled_arrival_second);
            strcat((char*)final_array_info, (char*)space);
            strcat((char*)final_array_info, (char*)aircraft_code_second);
        }
  

        
        t = strlen((char*)final_array)+1;
        t = MIN(t, max_length);
        strncpy((*choices)[i], (char*)final_array, t);

        t = strlen((char*)final_array_info)+1;
        t = MIN(t, max_length);
        strncpy((*choices_info)[i], (char*)final_array_info, t);
        i++;
    }

    SQLCloseCursor(stmt);

       /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);



}

