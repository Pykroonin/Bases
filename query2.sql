with tickets_ours as (select t.passenger_name, tf.ticket_no, tf.flight_id, f.scheduled_departure, tf.fare_conditions, tf.amount from flights f natural join ticket_flights tf natural join tickets t natural join bookings b where b.book_ref = '0002F3' order by tf.ticket_no),
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
           att.aircraft_code
    FROM asientos_totales att
    LEFT JOIN asientos_ocupados aoc ON att.flight_id = aoc.flight_id AND att.asientos = aoc.asientos
    WHERE aoc.flight_id IS NULL AND aoc.asientos IS null)
select tou.passenger_name, tou.ticket_no, tou.flight_id, tou.scheduled_departure, tou.fare_conditions,tou.amount, min(al.vacios), al.aircraft_code from tickets_ours tou join asientos_libres al on tou.flight_id = al.flight_id group by tou.passenger_name, tou.scheduled_departure, tou.ticket_no, tou.flight_id,tou.fare_conditions,tou.amount,al.aircraft_code

-- insert into boarding_passes (ticket_no, flight_id, boarding_no, seat_no) values (los que sean)
