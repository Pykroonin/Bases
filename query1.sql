WITH asientos_totales
     AS (SELECT f.flight_id,
                COUNT(s.seat_no) AS as_totales
         FROM   seats s,
                aircrafts_data ad,
                flights f
         WHERE  s.aircraft_code = ad.aircraft_code
                AND ad.aircraft_code = f.aircraft_code
         GROUP  BY f.flight_id),
     asientos_ocupados
     AS (SELECT f.flight_id,
                COUNT(tf.ticket_no) AS as_ocupados
         FROM   flights f,
                ticket_flights tf
         WHERE  f.flight_id = tf.flight_id
         GROUP  BY f.flight_id),
     asientos_libres
     AS (SELECT t.flight_id,
                ( t.as_totales - Coalesce(o.as_ocupados, 0) ) AS vacios
         FROM   asientos_totales t
                LEFT JOIN asientos_ocupados o
                       ON t.flight_id = o.flight_id),
     vuelos_directos
     AS (SELECT f.flight_id,
               f.scheduled_departure ,
               f.scheduled_arrival,
f.flight_id,
               f.scheduled_departure ,
               f.scheduled_arrival,
               0                                                 AS transbordos,
               al.vacios                                   AS asientos,
               ( f.scheduled_arrival  - f.scheduled_departure  ) AS tiempo,
               f.aircraft_code
         FROM   flights f
                JOIN asientos_libres al
                  ON f.flight_id = al.flight_id
         WHERE  f.departure_airport = 'SVO'
                AND f.arrival_airport = 'VKO'
                AND DATE(f.scheduled_departure) = '2017-09-12'
                AND al.vacios > '0'),
     vuelos_salida
     AS (SELECT f.flight_id,
                f.scheduled_departure as fecha_sal,
                f.scheduled_arrival as fecha_lleg2,
                f.arrival_airport,
                f.aircraft_code
         FROM   flights f
         WHERE  f.departure_airport = 'SVO'
                AND f.scheduled_departure >= '2017-09-12'
                AND scheduled_departure < '2017-09-12'::DATE+1),
     vuelos_llegada
     AS (SELECT f.flight_id as flight_id2,
                f.scheduled_departure as fecha_sal2,
                f.scheduled_arrival as fecha_lleg,
                f.departure_airport as departure_airport2,
                f.aircraft_code as aircraft_code2
         FROM   flights f
         WHERE  f.arrival_airport = 'VKO'
                AND f.scheduled_departure > '2017-09-12'),
     vuelos_transbordo
     AS (SELECT vs.flight_id,
               vs.fecha_sal,
               vs.fecha_lleg2,
               vl.flight_id2,
               vl.fecha_sal2,
               vl.fecha_lleg,
               1                                                 AS transbordos,
               Min(al.vacios)                                    AS asientos,
               ( vl.fecha_lleg - vs.fecha_sal ) AS tiempo,
               vs.aircraft_code
        FROM   vuelos_salida vs,
               vuelos_llegada vl,
               asientos_libres al
        WHERE  vs.arrival_airport = vl.departure_airport2
               AND vs.fecha_lleg2 < vl.fecha_sal2
               AND vl.fecha_lleg - vs.fecha_sal <
                   '24:00:00.000'
               AND ( vs.flight_id = al.flight_id
                      OR vl.flight_id2 = al.flight_id )
        GROUP  BY vs.flight_id,
                  vs.fecha_sal,
                  vs.fecha_lleg2,
                  vl.flight_id2,
                  vl.fecha_sal2,
                  vl.fecha_lleg,
                  vs.aircraft_code)
 select * from (select * from vuelos_transbordo union ALL select * from vuelos_directos) order by (fecha_sal - fecha_lleg)

