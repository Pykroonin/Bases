WITH vuelos_salida
AS
  (
         SELECT f.flight_id,
                f.scheduled_departure AS fecha_sal,
                f.scheduled_arrival   AS fecha_lleg2,
                f.arrival_airport,
                f.aircraft_code
         FROM   flights f
         WHERE  f.departure_airport = 'SVO'
         AND    f.scheduled_departure >= '2017-09-12'
         AND    scheduled_departure < '2017-09-12'::date+1),
  vuelos_llegada
AS
  (
         SELECT f.flight_id           AS flight_id2,
                f.scheduled_departure AS fecha_sal2,
                f.scheduled_arrival   AS fecha_lleg,
                f.departure_airport   AS departure_airport2,
                f.aircraft_code       AS aircraft_code2
         FROM   flights f
         WHERE  f.arrival_airport = 'VKO'
         AND    f.scheduled_departure > '2017-09-12'),
  asientos_totales
AS
  (
           SELECT   vs.flight_id,
                    count(s.seat_no) AS as_totales
           FROM     seats s,
                    vuelos_salida vs
           WHERE    s.aircraft_code = vs.aircraft_code
           GROUP BY vs.flight_id
           UNION
                 DISTINCT
           SELECT   vl.flight_id2,
                    count(s.seat_no) AS as_totales
           FROM     seats s,
                    vuelos_llegada vl
           WHERE    s.aircraft_code = vl.aircraft_code2
           GROUP BY vl.flight_id2),
  asientos_ocupados
AS
  (
           SELECT   vs.flight_id,
                    count(tf.ticket_no) AS as_ocupados
           FROM     vuelos_salida vs,
                    ticket_flights tf
           WHERE    vs.flight_id = tf.flight_id
           GROUP BY vs.flight_id
           UNION
                 DISTINCT
           SELECT   vl.flight_id2,
                    count(tf.ticket_no) AS as_ocupados
           FROM     vuelos_llegada vl,
                    ticket_flights tf
           WHERE    vl.flight_id2 = tf.flight_id
           GROUP BY vl.flight_id2),
  asientos_libres
AS
  (
            SELECT    t.flight_id,
                      ( t.as_totales - coalesce(o.as_ocupados, 0) ) AS vacios
            FROM      asientos_totales t
            LEFT JOIN asientos_ocupados o
            ON        t.flight_id = o.flight_id)
  SELECT   *
  FROM     (
                  SELECT *
                  FROM   (
                                  SELECT   vs.flight_id,
                                           vs.fecha_sal,
                                           vs.fecha_lleg2,
                                           vl.flight_id2,
                                           vl.fecha_sal2,
                                           vl.fecha_lleg,
                                           '1'                              AS transbordos,
                                           min(al.vacios)                   AS asientos,
                                           ( vl.fecha_lleg - vs.fecha_sal ) AS tiempo,
                                           vs.aircraft_code,
                                           vl.aircraft_code2
                                  FROM     vuelos_salida vs,
                                           vuelos_llegada vl,
                                           asientos_libres al
                                  WHERE    vs.arrival_airport = vl.departure_airport2
                                  AND      vs.fecha_lleg2 < vl.fecha_sal2
                                  AND      vl.fecha_lleg - vs.fecha_sal < '24:00:00.000'
                                  AND      (
                                                    vs.flight_id = al.flight_id
                                           OR       vl.flight_id2 = al.flight_id )
                                  GROUP BY vs.flight_id,
                                           vs.fecha_sal,
                                           vs.fecha_lleg2,
                                           vl.flight_id2,
                                           vl.fecha_sal2,
                                           vl.fecha_lleg,
                                           vs.aircraft_code,
                                           vl.aircraft_code2)
                UNION ALL
                SELECT *
                FROM   (
                              SELECT f.flight_id,
                                     f.scheduled_departure ,
                                     f.scheduled_arrival,
                                     f.flight_id,
                                     f.scheduled_departure ,
                                     f.scheduled_arrival,
                                     '0'                                             AS transbordos,
                                     al.vacios                                       AS asientos,
                                     ( f.scheduled_arrival - f.scheduled_departure ) AS tiempo,
                                     f.aircraft_code,
                                     f.aircraft_code
                              FROM   flights f
                              JOIN   asientos_libres al
                              ON     f.flight_id = al.flight_id
                              WHERE  f.departure_airport = 'SVO'
                              AND    f.arrival_airport = 'VKO'
                              AND    date(f.scheduled_departure) = '2017-09-12'
                              AND    al.vacios > '0'))
  ORDER BY tiempo
