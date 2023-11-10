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
    aviones_vacios
    as (select ats.flight_id,
    			ats.as_totales as vacios
    	from asientos_totales ats left join (select * from asientos_ocupados union all select * from asientos_libres) as tv on ats.flight_id = tv.flight_id where tv.flight_id is null),
    total_asientos_disponibles
    as (select * from asientos_libres union all select * from aviones_vacios),
    vuelos_directos
	as (select f.flight_id,
				f.departure_airport,
				f.arrival_airport,
				f.scheduled_departure as fecha_sal,
				f.scheduled_arrival as fecha_lleg,
				f.aircraft_code 
				from flights f where f.departure_airport = 'SVO' and f.arrival_airport = 'LED'),
	vuelos_fecha_directos
	as (select vt.flight_id, vt.departure_airport, vt.arrival_airport, vt.fecha_sal,vt.fecha_lleg, tad.vacios, '0' as transbordo, vt.aircraft_code from vuelos_directos vt natural join total_asientos_disponibles tad where DATE(vt.fecha_sal) = '2017-09-12'),
	vuelos_totales_salida
	as (select f.flight_id,f.departure_airport, f.arrival_airport, f.scheduled_departure as fecha_sal,f.scheduled_arrival as fecha_lleg, tad.vacios as asientos, f.aircraft_code  from flights f join total_asientos_disponibles tad on f.flight_id  = tad.flight_id where f.departure_airport = 'SVO' and f.arrival_airport != 'LED' and tad.vacios > '0'),
	vuelos_fecha_salida
	as (select vts.flight_id,vts.departure_airport, vts.arrival_airport, vts.fecha_sal, vts.fecha_lleg, vts.asientos,vts.aircraft_code from vuelos_totales_salida vts where DATE(vts.fecha_sal) = '2017-09-12'),
	vuelos_totales_llegada
	as (select f.flight_id, f.departure_airport, f.arrival_airport, f.scheduled_departure as fecha_sal, f.scheduled_arrival as fecha_lleg, tad.vacios as asientos, f.aircraft_code from flights f join total_asientos_disponibles tad on f.flight_id  = tad.flight_id where f.departure_airport != 'SVO' and f.arrival_airport = 'LED' and tad.vacios > '0'),
	vuelos_fecha_llegada
	as (select vtl.flight_id,vtl.departure_airport, vtl.arrival_airport,vtl.fecha_sal, vtl.fecha_lleg, vtl.asientos, vtl.aircraft_code from vuelos_totales_llegada vtl  where DATE(vtl.fecha_lleg) >= '2017-09-12' and DATE(vtl.fecha_lleg) < '2017-09-12'::DATE + 1),
	vuelos_transbordo_salida
	as (select vfs.flight_id,vfs.departure_airport, vfl.arrival_airport, vfs.fecha_sal, vfl.fecha_lleg, vfs.asientos, '1' as transbordo, vfl.aircraft_code  from vuelos_fecha_salida vfs join vuelos_fecha_llegada vfl on vfs.arrival_airport = vfl.departure_airport where vfs.asientos < vfl.asientos and vfs.fecha_lleg < vfl.fecha_sal),
	vuelos_transbordo_llegada
	as (select vfs.flight_id, vfs.departure_airport, vfl.arrival_airport,vfs.fecha_sal, vfl.fecha_lleg, vfl.asientos, '1' as transbordo, vfs.aircraft_code from vuelos_fecha_llegada vfl join vuelos_fecha_salida vfs on vfs.arrival_airport = vfl.departure_airport where vfs.asientos > vfl.asientos and vfs.fecha_lleg < vfl.fecha_sal),
	vuelos_transbordo
	as (select * from (select * from vuelos_transbordo_salida union all select * from vuelos_transbordo_llegada))
select * from (select * from vuelos_transbordo union all select * from vuelos_fecha_directos)
