PRAGMA foreign_keys = ON;
BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "buses" (
    "bus_id" INTEGER,
    "bus_number" TEXT NOT NULL UNIQUE,
    "bus_name" TEXT NOT NULL,
    "total_mileage_km" REAL NOT NULL DEFAULT 0 CHECK("total_mileage_km" >= 0),
    "image_data" BLOB,
    PRIMARY KEY("bus_id" AUTOINCREMENT)
);
CREATE TABLE IF NOT EXISTS "crew_members" (
	"member_id"	INTEGER,
	"crew_id"	INTEGER NOT NULL,
	"surname"	TEXT NOT NULL,
	"personnel_number"	TEXT NOT NULL UNIQUE,
	"experience_years"	INTEGER NOT NULL CHECK("experience_years" >= 0),
	"category"	TEXT NOT NULL,
	"address"	TEXT NOT NULL,
	"birth_year"	INTEGER NOT NULL CHECK("birth_year" BETWEEN 1950 AND 2010),
	PRIMARY KEY("member_id" AUTOINCREMENT),
	FOREIGN KEY("crew_id") REFERENCES "crews"("crew_id") ON UPDATE CASCADE ON DELETE RESTRICT
);
CREATE TABLE IF NOT EXISTS "crew_payments" (
    "payment_id" INTEGER,
    "crew_id" INTEGER NOT NULL,
    "period_start" TEXT NOT NULL,
    "period_end" TEXT NOT NULL,
    "percent_rate" REAL NOT NULL CHECK("percent_rate" >= 0),
    "total_amount" REAL NOT NULL CHECK("total_amount" >= 0),
    "calc_date" TEXT NOT NULL DEFAULT CURRENT_DATE,
    PRIMARY KEY("payment_id" AUTOINCREMENT),
    FOREIGN KEY("crew_id") REFERENCES "crews"("crew_id") ON UPDATE CASCADE ON DELETE RESTRICT,
    CHECK(date("period_end") >= date("period_start")),
    UNIQUE("crew_id", "period_start", "period_end", "percent_rate")
);
CREATE TABLE IF NOT EXISTS "crews" (
	"crew_id"	INTEGER,
	"bus_id"	INTEGER NOT NULL UNIQUE,
	PRIMARY KEY("crew_id" AUTOINCREMENT),
	FOREIGN KEY("bus_id") REFERENCES "buses"("bus_id") ON UPDATE CASCADE ON DELETE RESTRICT
);
CREATE TABLE IF NOT EXISTS "routes" (
	"route_id"	INTEGER,
	"route_name"	TEXT NOT NULL UNIQUE,
	"start_point"	TEXT NOT NULL,
	"end_point"	TEXT NOT NULL,
	"distance_km"	REAL NOT NULL CHECK("distance_km" > 0),
	PRIMARY KEY("route_id" AUTOINCREMENT)
);
CREATE TABLE IF NOT EXISTS "trips" (
	"trip_id"	INTEGER,
	"bus_id"	INTEGER NOT NULL,
	"route_id"	INTEGER NOT NULL,
	"departure_date"	TEXT NOT NULL,
	"arrival_date"	TEXT NOT NULL,
	"passenger_count"	INTEGER NOT NULL CHECK("passenger_count" > 0),
	"ticket_price"	REAL NOT NULL CHECK("ticket_price" > 0),
	PRIMARY KEY("trip_id" AUTOINCREMENT),
	FOREIGN KEY("bus_id") REFERENCES "buses"("bus_id") ON UPDATE CASCADE ON DELETE RESTRICT,
	FOREIGN KEY("route_id") REFERENCES "routes"("route_id") ON UPDATE CASCADE ON DELETE RESTRICT,
	CHECK(date("arrival_date") >= date("departure_date"))
);
CREATE TABLE IF NOT EXISTS "users" (
	"user_id"	INTEGER,
	"login"	TEXT NOT NULL UNIQUE,
	"password"	TEXT NOT NULL,
	"role"	TEXT NOT NULL CHECK("role" IN ('admin', 'crew')),
	"crew_member_id"	INTEGER,
	PRIMARY KEY("user_id" AUTOINCREMENT),
	FOREIGN KEY("crew_member_id") REFERENCES "crew_members"("member_id") ON UPDATE CASCADE ON DELETE SET NULL
);

INSERT INTO "buses" (bus_id, bus_number, bus_name, total_mileage_km, image_data)
VALUES (1,'A101','Mercedes Tourismo',126500.0,NULL);

INSERT INTO "buses" (bus_id, bus_number, bus_name, total_mileage_km, image_data)
VALUES (2,'B202','Setra ComfortClass',98000.0,NULL);

INSERT INTO "buses" (bus_id, bus_number, bus_name, total_mileage_km, image_data)
VALUES (3,'C303','MAN Lion Coach',143500.0,NULL);

INSERT INTO "crews" VALUES (1,1);
INSERT INTO "crews" VALUES (2,2);
INSERT INTO "crews" VALUES (3,3);

INSERT INTO "routes" VALUES (1,'Золотое кольцо','Москва','Ярославль',270.0);
INSERT INTO "routes" VALUES (2,'Серебряный маршрут','Санкт-Петербург','Выборг',140.0);
INSERT INTO "routes" VALUES (3,'Исторический тур','Казань','Свияжск',65.0);
INSERT INTO "routes" VALUES (4,'Горный маршрут','Сочи','Красная Поляна',75.0);

INSERT INTO "crew_members" VALUES (1,1,'Иванов','T001',10,'A','Москва, ул. Ленина, 10',1985);
INSERT INTO "crew_members" VALUES (2,1,'Петров','T002',7,'B','Москва, ул. Мира, 12',1988);
INSERT INTO "crew_members" VALUES (3,1,'Сидоров','T003',5,'B','Москва, ул. Гагарина, 8',1992);
INSERT INTO "crew_members" VALUES (4,2,'Кузнецов','T004',12,'A','Санкт-Петербург, Невский пр., 15',1982);
INSERT INTO "crew_members" VALUES (5,2,'Смирнов','T005',8,'B','Санкт-Петербург, Лиговский пр., 20',1987);
INSERT INTO "crew_members" VALUES (6,2,'Попов','T006',6,'B','Санкт-Петербург, ул. Пушкина, 18',1991);
INSERT INTO "crew_members" VALUES (7,3,'Васильев','T007',15,'A','Сочи, ул. Курортная, 5',1980);
INSERT INTO "crew_members" VALUES (8,3,'Федоров','T008',9,'B','Сочи, ул. Морская, 7',1986);
INSERT INTO "crew_members" VALUES (9,3,'Михайлов','T009',4,'C','Сочи, ул. Центральная, 11',1995);

INSERT INTO "users" VALUES (1,'admin','admin123','admin',NULL);
INSERT INTO "users" VALUES (2,'ivanov','1234','crew',1);
INSERT INTO "users" VALUES (3,'petrov','1234','crew',2);
INSERT INTO "users" VALUES (4,'sidorov','1234','crew',3);

INSERT INTO "trips" VALUES (1,1,1,'2024-01-10','2024-01-10',35,2500.0);
INSERT INTO "trips" VALUES (2,1,1,'2024-02-15','2024-02-15',40,2600.0);
INSERT INTO "trips" VALUES (3,1,3,'2024-03-12','2024-03-12',28,1800.0);
INSERT INTO "trips" VALUES (4,2,2,'2024-01-18','2024-01-18',30,2200.0);
INSERT INTO "trips" VALUES (5,2,2,'2024-04-20','2024-04-20',32,2300.0);
INSERT INTO "trips" VALUES (6,2,4,'2024-05-05','2024-05-05',25,3200.0);
INSERT INTO "trips" VALUES (7,3,4,'2024-02-01','2024-02-01',38,3500.0);
INSERT INTO "trips" VALUES (8,3,1,'2024-06-10','2024-06-10',45,2700.0);
INSERT INTO "trips" VALUES (9,3,4,'2024-07-07','2024-07-07',33,3600.0);
INSERT INTO "trips" VALUES (10,1,2,'2024-08-01','2024-08-01',30,2400.0);
INSERT INTO "trips" VALUES (11,2,1,'2024-08-10','2024-08-10',27,2100.0);
INSERT INTO "trips" VALUES (12,3,3,'2024-09-05','2024-09-05',36,2800.0);

CREATE VIEW crew_full_info AS
SELECT
    c.crew_id,
    b.bus_number,
    b.bus_name,
    cm.member_id,
    cm.surname,
    cm.personnel_number,
    cm.experience_years,
    cm.category,
    cm.address,
    cm.birth_year
FROM crews c
JOIN buses b ON b.bus_id = c.bus_id
JOIN crew_members cm ON cm.crew_id = c.crew_id;
CREATE TRIGGER check_crew_member_limit
BEFORE INSERT ON crew_members
FOR EACH ROW
BEGIN
    SELECT CASE
        WHEN (
            SELECT COUNT(*)
            FROM crew_members
            WHERE crew_id = NEW.crew_id
        ) >= 3
        THEN RAISE(ABORT, 'В экипаже не может быть больше 3 человек')
    END;
END;
CREATE TRIGGER validate_trip_before_insert
BEFORE INSERT ON trips
FOR EACH ROW
BEGIN
    SELECT CASE
        WHEN (SELECT COUNT(*) FROM buses WHERE bus_id = NEW.bus_id) = 0
        THEN RAISE(ABORT, 'Указанный автобус не существует')
    END;

    SELECT CASE
        WHEN (SELECT COUNT(*) FROM routes WHERE route_id = NEW.route_id) = 0
        THEN RAISE(ABORT, 'Указанный маршрут не существует')
    END;

    SELECT CASE
        WHEN (
            SELECT COUNT(*)
            FROM crews
            WHERE bus_id = NEW.bus_id
        ) = 0
        THEN RAISE(ABORT, 'Для автобуса не назначен экипаж')
    END;

    SELECT CASE
        WHEN (
            SELECT COUNT(*)
            FROM crew_members cm
            JOIN crews c ON c.crew_id = cm.crew_id
            WHERE c.bus_id = NEW.bus_id
        ) <> 3
        THEN RAISE(ABORT, 'Экипаж автобуса должен состоять ровно из 3 человек')
    END;

    SELECT CASE
        WHEN date(NEW.arrival_date) < date(NEW.departure_date)
        THEN RAISE(ABORT, 'Дата прибытия не может быть раньше даты отправления')
    END;
END;


CREATE TRIGGER validate_trip_before_update
BEFORE UPDATE ON trips
FOR EACH ROW
BEGIN
    SELECT CASE
        WHEN (SELECT COUNT(*) FROM buses WHERE bus_id = NEW.bus_id) = 0
        THEN RAISE(ABORT, 'Указанный автобус не существует')
    END;

    SELECT CASE
        WHEN (SELECT COUNT(*) FROM routes WHERE route_id = NEW.route_id) = 0
        THEN RAISE(ABORT, 'Указанный маршрут не существует')
    END;

    SELECT CASE
        WHEN (
            SELECT COUNT(*)
            FROM crews
            WHERE bus_id = NEW.bus_id
        ) = 0
        THEN RAISE(ABORT, 'Для автобуса не назначен экипаж')
    END;

    SELECT CASE
        WHEN (
            SELECT COUNT(*)
            FROM crew_members cm
            JOIN crews c ON c.crew_id = cm.crew_id
            WHERE c.bus_id = NEW.bus_id
        ) <> 3
        THEN RAISE(ABORT, 'Экипаж автобуса должен состоять ровно из 3 человек')
    END;

    SELECT CASE
        WHEN date(NEW.arrival_date) < date(NEW.departure_date)
        THEN RAISE(ABORT, 'Дата прибытия не может быть раньше даты отправления')
    END;
END;
COMMIT;