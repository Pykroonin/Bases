with asientos_totales
     AS (SELECT f.flight_id,
                Count(s.seat_no) AS as_totales
         FROM   seats s,
                aircrafts_data ad,
                flights f
         WHERE  s.aircraft_code = ad.aircraft_code
                AND ad.aircraft_code = f.aircraft_code
         GROUP  BY f.flight_id),
     asientos_ocupados
     AS (SELECT f.flight_id,
                Count(tf.ticket_no) AS as_ocupados
         FROM   flights f,
                ticket_flights tf
         WHERE  f.flight_id = tf.flight_id
         GROUP  BY f.flight_id),
     asientos_libres
     AS (SELECT ats.flight_id,
                ( ats.as_totales - aoc.as_ocupados ) AS vacios
         FROM   asientos_totales ats,
                asientos_ocupados aoc
         WHERE  ats.flight_id = aoc.flight_id),
    vuelos_directos
	as (select f.departure_airport,
				f.arrival_airport,
				DATE(f.scheduled_departure) as fecha_sal,
				DATE(f.scheduled_arrival) as fecha_lleg,
				al.vacios as asientos
				from flights f join asientos_libres al on f.flight_id  = al.flight_id where f.departure_airport = 'SVO' and f.arrival_airport = 'LED' and al.vacios > '0'),
	vuelos_fecha_directos
	as (select vt.departure_airport, vt.arrival_airport, vt.fecha_sal,vt.fecha_lleg, vt.asientos, '0' as transbordo from vuelos_directos vt where vt.fecha_sal = '2017-09-12'),
	vuelos_totales_salida
	as (select f.departure_airport, f.arrival_airport, DATE(f.scheduled_departure) as fecha_sal,DATE(f.scheduled_arrival) as fecha_lleg, al.vacios as asientos from flights f join asientos_libres al on f.flight_id  = al.flight_id where f.departure_airport = 'SVO' and f.arrival_airport != 'LED' and al.vacios > '0'),
	vuelos_fecha_salida
	as (select vts.departure_airport, vts.arrival_airport, vts.fecha_sal, vts.fecha_lleg, vts.asientos from vuelos_totales_salida vts where vts.fecha_sal = '2017-09-12'),
	vuelos_totales_llegada
	as (select f.departure_airport, f.arrival_airport, DATE(f.scheduled_departure) as fecha_sal, DATE(f.scheduled_arrival) as fecha_lleg, al.vacios as asientos from flights f join asientos_libres al on f.flight_id  = al.flight_id where f.departure_airport != 'SVO' and f.arrival_airport = 'LED' and al.vacios > '0'),
	vuelos_fecha_llegada
	as (select vtl.departure_airport, vtl.arrival_airport,vtl.fecha_sal, vtl.fecha_lleg, vtl.asientos from vuelos_totales_llegada vtl  where vtl.fecha_lleg >= '2017-09-12' and vtl.fecha_lleg <= '2017-09-12'::DATE + 1),
	vuelos_transbordo_salida
	as (select vfs.departure_airport, vfl.arrival_airport, vfs.fecha_sal, vfl.fecha_lleg, vfs.asientos, '1' as transbordo from vuelos_fecha_salida vfs join vuelos_fecha_llegada vfl on vfs.arrival_airport = vfl.departure_airport where vfs.asientos < vfl.asientos),
	vuelos_transbordo_llegada
	as (select vfs.departure_airport, vfl.arrival_airport,vfs.fecha_sal, vfl.fecha_lleg, vfl.asientos, '1' as transbordo from vuelos_fecha_llegada vfl join vuelos_fecha_salida vfs on vfs.arrival_airport = vfl.departure_airport where vfs.asientos > vfl.asientos),
	vuelos_transbordo
	as (select * from (select * from vuelos_transbordo_salida union all select * from vuelos_transbordo_llegada))
select * from (select * from vuelos_transbordo union all select * from vuelos_fecha_directos) order by fecha_sal, fecha_lleg
