#include "menu.h"
#include <iostream>
#include <limits>

Menu::Menu(AuthService& auth,
           TripService& trip,
           ReportService& report,
           BusService& bus,
           RouteService& route)
    : authService(auth),
      tripService(trip),
      reportService(report),
      busService(bus),
      routeService(route) {}

void Menu::run() {
    std::string login;
    std::string password;

    std::cout << "===== Tourist Bureau =====\n";
    std::cout << "Логин: ";
    std::cin >> login;
    std::cout << "Пароль: ";
    std::cin >> password;

    UserSession session = authService.login(login, password);

    if (!session.authenticated) {
        std::cout << "Ошибка: неверный логин или пароль.\n";
        return;
    }

    std::cout << "Успешный вход. Роль: " << session.role << "\n";

    if (session.role == "admin") {
        adminMenu(session);
    } else if (session.role == "crew") {
        crewMenu(session);
    } else {
        std::cout << "Неизвестная роль пользователя.\n";
    }
}

void Menu::adminMenu(const UserSession&) {
    int choice = -1;

    while (choice != 0) {
        std::cout << "\n===== Меню администратора =====\n";
        std::cout << "1. Показать рейсы автобуса за период\n";
        std::cout << "2. Показать статистику по автобусу за период\n";
        std::cout << "3. Показать начисления по каждому экипажу за период\n";
        std::cout << "4. Показать сведения по наиболее дорогому маршруту\n";
        std::cout << "5. Показать автобус с наибольшим пробегом\n";
        std::cout << "6. Добавить рейс\n";
        std::cout << "7. Обновить рейс\n";
        std::cout << "8. Удалить рейс\n";
        std::cout << "9. Рассчитать и сохранить начисления экипажам\n";
        std::cout << "10. Показать начисления экипажа на дату\n";
        std::cout << "11. Показать информацию об изображении автобуса\n";
        std::cout << "12. Загрузить изображение автобуса из файла\n";
        std::cout << "13. Выгрузить изображение автобуса в файл\n";
        std::cout << "14. Показать все автобусы\n";
std::cout << "15. Добавить автобус\n";
std::cout << "16. Обновить автобус\n";
std::cout << "17. Удалить автобус\n";
std::cout << "18. Показать все маршруты\n";
std::cout << "19. Добавить маршрут\n";
std::cout << "20. Обновить маршрут\n";
std::cout << "21. Удалить маршрут\n";
        std::cout << "0. Выход\n";
        std::cout << "Выберите пункт: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1;
            continue;
        }

        if (choice == 1) {
            std::string busNumber, startDate, endDate;
            std::cout << "Номер автобуса: ";
            std::cin >> busNumber;
            std::cout << "Дата начала (YYYY-MM-DD): ";
            std::cin >> startDate;
            std::cout << "Дата конца (YYYY-MM-DD): ";
            std::cin >> endDate;
            tripService.printTripsByBusAndPeriod(busNumber, startDate, endDate);

        } else if (choice == 2) {
            std::string busNumber, startDate, endDate;
            std::cout << "Номер автобуса: ";
            std::cin >> busNumber;
            std::cout << "Дата начала (YYYY-MM-DD): ";
            std::cin >> startDate;
            std::cout << "Дата конца (YYYY-MM-DD): ";
            std::cin >> endDate;
            reportService.printBusSummary(busNumber, startDate, endDate);

        } else if (choice == 3) {
            std::string startDate, endDate;
            std::cout << "Дата начала (YYYY-MM-DD): ";
            std::cin >> startDate;
            std::cout << "Дата конца (YYYY-MM-DD): ";
            std::cin >> endDate;
            reportService.printCrewIncomeByPeriod(startDate, endDate);

        } else if (choice == 4) {
            reportService.printMostExpensiveRouteInfo();

        } else if (choice == 5) {
            reportService.printTopMileageBusInfo();

        } else if (choice == 6) {
            int busId, routeId, passengerCount;
            double ticketPrice;
            std::string departureDate, arrivalDate;

            std::cout << "bus_id: ";
            std::cin >> busId;
            std::cout << "route_id: ";
            std::cin >> routeId;
            std::cout << "Дата отправления (YYYY-MM-DD): ";
            std::cin >> departureDate;
            std::cout << "Дата прибытия (YYYY-MM-DD): ";
            std::cin >> arrivalDate;
            std::cout << "Количество пассажиров: ";
            std::cin >> passengerCount;
            std::cout << "Цена билета: ";
            std::cin >> ticketPrice;

            if (tripService.addTrip(busId, routeId, departureDate, arrivalDate, passengerCount, ticketPrice)) {
                std::cout << "Рейс успешно добавлен.\n";
            }

        } else if (choice == 7) {
            int tripId, busId, routeId, passengerCount;
            double ticketPrice;
            std::string departureDate, arrivalDate;

            std::cout << "trip_id: ";
            std::cin >> tripId;
            std::cout << "Новый bus_id: ";
            std::cin >> busId;
            std::cout << "Новый route_id: ";
            std::cin >> routeId;
            std::cout << "Новая дата отправления (YYYY-MM-DD): ";
            std::cin >> departureDate;
            std::cout << "Новая дата прибытия (YYYY-MM-DD): ";
            std::cin >> arrivalDate;
            std::cout << "Новое количество пассажиров: ";
            std::cin >> passengerCount;
            std::cout << "Новая цена билета: ";
            std::cin >> ticketPrice;

            if (tripService.updateTrip(tripId, busId, routeId, departureDate, arrivalDate, passengerCount, ticketPrice)) {
                std::cout << "Рейс успешно обновлен.\n";
            }

        } else if (choice == 8) {
            int tripId;
            std::cout << "trip_id для удаления: ";
            std::cin >> tripId;

            if (tripService.deleteTrip(tripId)) {
                std::cout << "Рейс успешно удален.\n";
            }

        } else if (choice == 9) {
            double percentRate;
            std::string startDate, endDate;

            std::cout << "Процент отчисления: ";
            std::cin >> percentRate;
            std::cout << "Дата начала (YYYY-MM-DD): ";
            std::cin >> startDate;
            std::cout << "Дата конца (YYYY-MM-DD): ";
            std::cin >> endDate;

            if (reportService.calculateCrewPayments(percentRate, startDate, endDate)) {
                std::cout << "Начисления сохранены в таблицу crew_payments.\n";
            }

        } else if (choice == 10) {
            int crewId;
            std::string dateValue;

            std::cout << "crew_id: ";
            std::cin >> crewId;
            std::cout << "Дата (YYYY-MM-DD): ";
            std::cin >> dateValue;

            reportService.printCrewPaymentOnDate(crewId, dateValue);

        } else if (choice == 11) {
            std::string busNumber;
            std::cout << "Номер автобуса: ";
            std::cin >> busNumber;
            reportService.printBusImageInfo(busNumber);

        } else if (choice == 12) {
            std::string busNumber;
            std::string filePath;

            std::cout << "Номер автобуса: ";
            std::cin >> busNumber;
            std::cout << "Путь к файлу изображения: ";
            std::cin >> filePath;

            if (reportService.saveBusImageFromFile(busNumber, filePath)) {
                std::cout << "Изображение успешно сохранено в БД.\n";
            }

        } else if (choice == 13) {
            std::string busNumber;
            std::string outputPath;

            std::cout << "Номер автобуса: ";
            std::cin >> busNumber;
            std::cout << "Путь для сохранения файла: ";
            std::cin >> outputPath;

            if (reportService.exportBusImageToFile(busNumber, outputPath)) {
                std::cout << "Изображение успешно выгружено из БД.\n";
            }
        

        }  else if (choice == 14) {
            busService.printAllBuses();

        } else if (choice == 15) {
            std::string busNumber, busName;
            double totalMileageKm;

            std::cout << "Номер автобуса: ";
            std::cin >> busNumber;
            std::cout << "Название автобуса: ";
            std::getline(std::cin >> std::ws, busName);
            std::cout << "Пробег: ";
            std::cin >> totalMileageKm;

            if (busService.addBus(busNumber, busName, totalMileageKm)) {
                std::cout << "Автобус успешно добавлен.\n";
            }

        } else if (choice == 16) {
            int busId;
            std::string busNumber, busName;
            double totalMileageKm;

            std::cout << "bus_id: ";
            std::cin >> busId;
            std::cout << "Новый номер автобуса: ";
            std::cin >> busNumber;
            std::cout << "Новое название автобуса: ";
            std::getline(std::cin >> std::ws, busName);
            std::cout << "Новый пробег: ";
            std::cin >> totalMileageKm;

            if (busService.updateBus(busId, busNumber, busName, totalMileageKm)) {
                std::cout << "Автобус успешно обновлен.\n";
            }

        } else if (choice == 17) {
            int busId;
            std::cout << "bus_id для удаления: ";
            std::cin >> busId;

            if (busService.deleteBus(busId)) {
                std::cout << "Автобус успешно удален.\n";
            }

        } else if (choice == 18) {
            routeService.printAllRoutes();

        } else if (choice == 19) {
            std::string routeName, startPoint, endPoint;
            double distanceKm;

            std::cout << "Название маршрута: ";
            std::getline(std::cin >> std::ws, routeName);
            std::cout << "Начальный пункт: ";
            std::getline(std::cin >> std::ws, startPoint);
            std::cout << "Конечный пункт: ";
            std::getline(std::cin >> std::ws, endPoint);
            std::cout << "Протяженность (км): ";
            std::cin >> distanceKm;

            if (routeService.addRoute(routeName, startPoint, endPoint, distanceKm)) {
                std::cout << "Маршрут успешно добавлен.\n";
            }

        } else if (choice == 20) {
            int routeId;
            std::string routeName, startPoint, endPoint;
            double distanceKm;

            std::cout << "route_id: ";
            std::cin >> routeId;
            std::cout << "Новое название маршрута: ";
            std::getline(std::cin >> std::ws, routeName);
            std::cout << "Новый начальный пункт: ";
            std::getline(std::cin >> std::ws, startPoint);
            std::cout << "Новый конечный пункт: ";
            std::getline(std::cin >> std::ws, endPoint);
            std::cout << "Новая протяженность (км): ";
            std::cin >> distanceKm;

            if (routeService.updateRoute(routeId, routeName, startPoint, endPoint, distanceKm)) {
                std::cout << "Маршрут успешно обновлен.\n";
            }

        } else if (choice == 21) {
            int routeId;
            std::cout << "route_id для удаления: ";
            std::cin >> routeId;

            if (routeService.deleteRoute(routeId)) {
                std::cout << "Маршрут успешно удален.\n";
            }
        } else if (choice == 0) {
            std::cout << "Выход из программы.\n";

        } else {
            std::cout << "Неверный пункт меню.\n";
        }
    }
}


void Menu::crewMenu(const UserSession& session) {
    int choice = -1;

    while (choice != 0) {
        std::cout << "\n===== Меню члена экипажа =====\n";
        std::cout << "1. Показать мои данные\n";
        std::cout << "2. Показать мои начисления на дату\n";
        std::cout << "0. Выход\n";
        std::cout << "Выберите пункт: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1;
            continue;
        }

        if (choice == 1) {
            reportService.printCrewMemberOwnInfo(session.crewMemberId);
        } else if (choice == 2) {
            std::string dateValue;
            std::cout << "Дата (YYYY-MM-DD): ";
            std::cin >> dateValue;

            int crewId = reportService.getCrewIdByMemberId(session.crewMemberId);
            if (crewId == -1) {
                std::cout << "Экипаж не найден.\n";
            } else {
                reportService.printCrewPaymentOnDate(crewId, dateValue);
            }
        } else if (choice == 0) {
            std::cout << "Выход из программы.\n";
        } else {
            std::cout << "Неверный пункт меню.\n";
        }
    }
}