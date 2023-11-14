/*
* Created by roberto on 3/5/21.
*/
#include <stdio.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "lbpass.h"
void    results_bpass(/*@unused@*/ char * bookID,
                       int * n_choices, char *** choices,
                       int max_length,
                       int max_rows)
/**here you need to do your query and fill the choices array of strings
*
* @param bookID  form field bookId
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
    SQLRETURN ret;

    SQLCHAR passenger_name[21];
    SQLCHAR flight_id[64]; 
    SQLCHAR scheduled_departure[64]; 
    SQLCHAR seat_no[64];
    SQLLEN num_filas;
    SQLCHAR final_array[512];
    SQLCHAR space[2] = " ";

    SQLCHAR parte1[] = "WITH vuelos AS (SELECT tf.flight_id, t.ticket_no, ROW_NUMBER() OVER (PARTITION BY tf.flight_id ORDER BY t.ticket_no) AS posicion FROM tickets t, ticket_flights tf WHERE t.book_ref = ? AND t.ticket_no = tf.ticket_no AND NOT EXISTS ( SELECT 1 ";
    SQLCHAR parte2[] = "FROM boarding_passes bp WHERE bp.flight_id = tf.flight_id AND bp.ticket_no = t.ticket_no) GROUP BY tf.flight_id, t.ticket_no ORDER BY tf.flight_id, t.ticket_no ), asientos_ocupados as (select distinct bp.flight_id, bp.seat_no from boarding_passes bp, vuelos v where bp.flight_id = v.flight_id), asientos as (select distinct f.flight_id, s.seat_no from flights f, seats s, vuelos v ";
    SQLCHAR parte3[] = "where f.aircraft_code = s.aircraft_code and f.flight_id = v.flight_id), vacios AS (SELECT a.flight_id, a.seat_no, ROW_NUMBER() OVER (PARTITION BY a.flight_id ORDER BY a.flight_id, a.seat_no) AS numero FROM asientos a WHERE NOT EXISTS ( SELECT 1 FROM asientos_ocupados ao WHERE ao.flight_id = a.flight_id AND ao.seat_no = a.seat_no)) ";
    SQLCHAR parte4[] = "SELECT t.passenger_name, v.flight_id, f.scheduled_departure, av.seat_no FROM vacios av, vuelos v, tickets t, flights f ";
    SQLCHAR parte5[] = "where v.flight_id = av.flight_id and v.posicion = av.numero and t.ticket_no = v.ticket_no and f.flight_id = v.flight_id";
    SQLCHAR consulta_completa[8192];

    strcpy((char*)consulta_completa, (char*)parte1);
    strcat((char*)consulta_completa, (char*)parte2);
    strcat((char*)consulta_completa, (char*)parte3);
    strcat((char*)consulta_completa, (char*)parte4);
    strcat((char*)consulta_completa, (char*)parte5);



    ret = odbc_connect(&env, &dbc);

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);


    
    SQLPrepare(stmt, consulta_completa, SQL_NTS);
    /* SQLPrepare(stmt, (SQLCHAR*)"select* from flights limit 10", SQL_NTS);*/

    SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, sizeof((SQLCHAR*)bookID), 0, (SQLCHAR*)bookID, sizeof((SQLCHAR*)bookID), NULL);

    SQLExecute(stmt);

    SQLBindCol(stmt, 1, SQL_C_CHAR, passenger_name, sizeof(passenger_name), NULL);
    SQLBindCol(stmt, 2, SQL_C_CHAR, flight_id, sizeof(flight_id), NULL);
    SQLBindCol(stmt, 3, SQL_C_CHAR, scheduled_departure, sizeof(scheduled_departure), NULL);
    SQLBindCol(stmt, 4, SQL_C_CHAR, seat_no, sizeof(seat_no), NULL);

    SQLRowCount(stmt, &num_filas);


    *n_choices = (int)num_filas;
    if(*n_choices<0){
        *n_choices=0;
    }
    max_rows = MIN(*n_choices, max_rows);

    
 while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
    
        strcpy((char*)final_array, (char*)passenger_name); 
        if(strlen((char*)final_array)==20) final_array[21]='\0';
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)flight_id);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)scheduled_departure);
        strcat((char*)final_array, (char*)space); 
        strcat((char*)final_array, (char*)seat_no);
        
        t = strlen((char*)final_array)+1;
        t = MIN(t, max_length);
        strncpy((*choices)[i], (char*)final_array, t);
        i++;
    }

    SQLCloseCursor(stmt);

       /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);

}

