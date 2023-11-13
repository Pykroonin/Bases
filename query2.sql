WITH booking_pasajeros
     AS (SELECT t.passenger_name,
                t.ticket_no,
                tf.flight_id,
                f.scheduled_departure,
                tf.fare_conditions,
                tf.amount,
                Row_number()
                  OVER (
                    partition BY tf.flight_id
                    ORDER BY t.ticket_no ) AS posicion
         FROM   flights f
                natural JOIN ticket_flights tf
                natural JOIN tickets t
                natural JOIN bookings b
         WHERE  b.book_ref = '8E6BB3'
                AND NOT EXISTS (SELECT '1'
                                FROM   boarding_passes bp
                                WHERE  bp.flight_id = tf.flight_id
                                       AND bp.ticket_no = t.ticket_no)
         ORDER  BY tf.ticket_no),
     asientos_totales
     AS (SELECT bps.flight_id,
                s.seat_no AS asientos,
                f.aircraft_code
         FROM   seats s,
                aircrafts_data ad,
                booking_pasajeros bps,
                flights f
         WHERE  s.aircraft_code = ad.aircraft_code
                AND ad.aircraft_code = f.aircraft_code
                AND f.flight_id = bps.flight_id
         GROUP  BY bps.flight_id,
                   s.seat_no,
                   f.aircraft_code
         ORDER  BY aircraft_code,
                   seat_no),
     asientos_ocupados
     AS (SELECT bps.flight_id,
                bp.seat_no AS asientos,
                f.aircraft_code
         FROM   booking_pasajeros bps,
                ticket_flights tf,
                boarding_passes bp,
                flights f
         WHERE  bps.flight_id = tf.flight_id
                AND tf.flight_id = bp.flight_id
                AND f.flight_id = bps.flight_id
         GROUP  BY bps.flight_id,
                   bp.seat_no,
                   f,
                   aircraft_code
         ORDER  BY bp.seat_no,
                   f.aircraft_code),
     asientos_libres
     AS (SELECT att.flight_id,
                att.asientos                              AS vacios,
                Row_number()
                  OVER (
                    partition BY att.flight_id
                    ORDER BY att.flight_id, att.asientos) AS posicion
         FROM   asientos_totales att
         WHERE  NOT EXISTS (SELECT '1'
                            FROM   asientos_ocupados ao
                            WHERE  ao.flight_id = att.flight_id
                                   AND ao.asientos = att.asientos))
SELECT bps.passenger_name,
       bps.flight_id,
       bps.scheduled_departure,
       al.vacios
FROM   asientos_libres al,
       booking_pasajeros bps
WHERE  bps.flight_id = al.flight_id
       AND bps.posicion = al.posicion  
