with tickets_ours as (select t.passenger_name, t.ticket_no, tf.flight_id, f.scheduled_departure, tf.fare_conditions, tf.amount, row_number() over
(partition by tf.flight_id order by t.ticket_no ) as posicion from flights f natural join ticket_flights tf natural join tickets t natural join bookings b where b.book_ref = '8E6BB3' and not exists (select 1 from boarding_passes bp where bp.flight_id =tf.flight_id AND bp.ticket_no = t.ticket_no) order by tf.ticket_no),
	asientos_totales
     AS (SELECT tou.flight_id,
                s.seat_no AS asientos,
                f.aircraft_code
         FROM   seats s,
                aircrafts_data ad,
                tickets_ours tou,
                flights f
         WHERE  s.aircraft_code = ad.aircraft_code
                AND ad.aircraft_code = f.aircraft_code
                and f.flight_id = tou.flight_id
         GROUP  BY tou.flight_id, s.seat_no, f.aircraft_code
         order by aircraft_code, seat_no),
    asientos_ocupados
     AS (SELECT tou.flight_id,
                bp.seat_no AS asientos,
                f.aircraft_code
         FROM   tickets_ours tou,
                ticket_flights tf,
                boarding_passes bp,
                flights f
         WHERE  tou.flight_id = tf.flight_id
         and tf.flight_id = bp.flight_id
         and f.flight_id = tou.flight_id
         GROUP  BY tou.flight_id, bp.seat_no, f,aircraft_code  order by bp.seat_no, f.aircraft_code),
asientos_libres AS (
    SELECT att.flight_id,
           att.asientos AS vacios,
          ROW_NUMBER() OVER (PARTITION BY att.flight_id ORDER BY att.flight_id, att.asientos) AS numero
 FROM asientos_totales att
 WHERE NOT EXISTS (
   SELECT 1
   FROM asientos_ocupados ao
   WHERE ao.flight_id = att.flight_id AND ao.asientos = att.asientos))
SELECT t.passenger_name, v.flight_id, f.scheduled_departure, av.vacios
FROM asientos_libres av, tickets_ours v, tickets t, flights f
where v.flight_id = av.flight_id and v.posicion = av.numero and t.ticket_no = v.ticket_no and f.flight_id = v.flight_id
