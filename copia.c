#include <stdio.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "search.h"

int main(void) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLHSTMT stmt;
    SQLRETURN ret;
    SQLCHAR arrival_airport[4];
    SQLCHAR scheduled_departure[20];
    SQLCHAR scheduled_arrival[20];
    SQL_TIMESTAMP_STRUCT timestamp;
    unsigned short int year;
    unsigned short int month;
    unsigned short int day;

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
 

    /* Allocate a statement handle */
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

    SQLPrepare(stmt, (SQLCHAR*) "select arrival_airport, scheduled_departure, scheduled_arrival from flights f where scheduled_departure > ? limit 10", SQL_NTS);

    printf("Fecha de salida (e.g., '2023-10-12'): ");
    fflush(stdout);


    while (scanf("%4hu-%2hu-%2hu", &year, &month, &day) == 3) {
        timestamp.year = year;
        timestamp.month = month;
        timestamp.day = day;
        timestamp.year = 2015;
        timestamp.month = 9;
        timestamp.day = 17;
        timestamp.hour = 0;
        timestamp.minute = 0;
        timestamp.second = 0;
        timestamp.fraction = 0;

        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &timestamp, 0, NULL);

        SQLExecute(stmt);

        SQLBindCol(stmt, 1, SQL_C_CHAR, arrival_airport, sizeof(arrival_airport), NULL);
        SQLBindCol(stmt, 2, SQL_C_CHAR, scheduled_departure, sizeof(scheduled_departure), NULL);
        SQLBindCol(stmt, 3, SQL_C_CHAR, scheduled_arrival, sizeof(scheduled_arrival), NULL);

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("Arrival Airport: %s, Scheduled Departure: %s, Scheduled Arrival: %s\n", arrival_airport, scheduled_departure, scheduled_arrival);
        }

        SQLCloseCursor(stmt);

        printf("Fecha de salida (e.g., '2023-10-12'): ");
        fflush(stdout);
    }

    /* free up statement handle */
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);

    return 0;
}
