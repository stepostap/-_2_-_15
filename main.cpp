#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore.h>
#include <cmath>

using namespace std;

mutex mtx;
mutex msg_mutex;
int CustomerMade = 0;

class Customer {
    bool sex; /// Пол
    string name; /// Имя
    int days_to_stay; /// Дни до отъезда
    /**
     * Метод для генерации случайной строки, используется для создания имени посетителя
     * для более удобной визуализации программы
     * @return
     */
    string RndString() {
        char *ar[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
                      "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
        string s = "";
        for (int i = 0; i < rand() % 8 + 3; i++)
            s += ar[rand() % 25];
        return s;
    }

public:
    bool has_left = true; /// Уехал ли человек из отеля, если да, то можно перезаписывать его на другого
    int GetDays() {
        return days_to_stay;
    }
    string GetName() {
        return name;
    }
    /// Выводит информацию о человеке в формате Пол: Имя
    string Print(){
        if (sex)
            return "Male: " + name;
        else
            return "Female: " + name;
    }
    bool GetSex() {
        return sex;
    }
    Customer() {
        CustomerMade++;
        if (rand() % 2 == 1)
            sex = true;
        else sex = false;
        //srand(time(NULL) * CustomerMade * CustomerMade % (rand() % 100));
        days_to_stay = rand() % 21 + 1;
        name = RndString();
        has_left = true;

    }
};

class DoubleRoom {
public:
    Customer first;
    Customer second;
    DoubleRoom(){
        first = Customer();
        second = Customer();
    }
    /**
     * Метод, проверяющий, может ли данный посетитель заселиться в данную комнату для двоих в отеле
     * @param newCustomer
     * @return
     */
    bool CanSettle(Customer newCustomer){
        if (first.has_left && second.has_left) {
            return true;
        } else if (!first.has_left && second.has_left && first.GetSex() == newCustomer.GetSex()) {
            return true;
        } else if (first.has_left && !second.has_left && second.GetSex() == newCustomer.GetSex()) {
            return true;
        }
        return false;
    }
    /**
     * Заселение в комнату для двоих посетителя
     * @param newCustomer
     */
    void Settle(Customer newCustomer, int days) {
        if (first.has_left && second.has_left) {
            first = newCustomer;
            first.has_left = false;
            msg_mutex.lock();
            cout << "DAY " << days << ":    " << newCustomer.Print()
                << " is going to live alone in a room for two for "
                << newCustomer.GetDays() << " days." << endl;
            msg_mutex.unlock();
            return;

        } else if (!first.has_left && second.has_left && first.GetSex() == newCustomer.GetSex()) {
            second = newCustomer;
            second.has_left = false;
            msg_mutex.lock();
            cout << "DAY " << days << ":    "  << newCustomer.Print()
                << " is going to live with " << first.Print() << " in a room for two for "
                << newCustomer.GetDays() << " days." << endl;
            msg_mutex.unlock();
            return;
        } else if (first.has_left && !second.has_left && second.GetSex() == newCustomer.GetSex()) {
            first = newCustomer;
            first.has_left = false;
            msg_mutex.lock();
            cout << "DAY " << days << ":    "  << newCustomer.Print()
                << " is going to live with " << second.Print() << " in a room for two for "
                << newCustomer.GetDays() << " days." << endl;
            msg_mutex.unlock();
            return;
        }
        return;
    }
    /**
     * Выселение человека из комнаты
     * @param customer
     */
    void MoveOut(Customer customer) {
        if (first.GetName() == customer.GetName() && first.GetSex() == customer.GetSex()
        && first.GetDays() == customer.GetDays())
            first.has_left = true;
        else
            second.has_left = true;
    }
};

class Hotel {

    int roomsforone = 0;
    /**
     * Возвращает индекс одиночной комнаты, в которую можно заселиться
     * Если таких комнат нет, возвращает -1
     * @return
     */
    int GetFreeRoomIndex(){
        for (int i = 0; i < 10; ++i) {
            if (RoomsForOne[i].has_left)
                return i;
        }
        return -1;
    }
    /**
     * Возвращает индекс двойной комнаты, в которую можно заселиться
     * Если таких комнат нет, возвращает -1
     * @param cstmr
     * @return
     */
    int GetFreeDoubleRoomIndex(Customer cstmr){
        for(int i = 0; i < 15; i++){
            if (RoomsForTwo[i].CanSettle(cstmr))
                return i;
        }
        return -1;
    }
    /**
     * Метод для отладки, выводит инфрмацию о всех одиночных комнатах
     * 0 - комната свободна
     * 1 - комната занята
     */
    void PrintOneRoom(){
        string temp = "";
        for(Customer customer: RoomsForOne){
            if (customer.has_left)
                temp = temp + "0" + " ";
            else
                temp = temp + "1" + " ";
        }
        cout << temp << endl;
    }
    /**
     * Метод для отладки, выводит инфрмацию о всех двойных комнатах
     * 00 - комната свободна
     * 11 - комната занята
     * 10 (01) - есть только один жилец
     */
    void PrintTwoRoom(){
        string temp = "";
        for(DoubleRoom room: RoomsForTwo){
            if (room.first.has_left && room.second.has_left)
                temp = temp + "0" + "0" + " ";
            if (room.first.has_left && !room.second.has_left)
                temp = temp + "0" + "1" + " ";
            if (!room.first.has_left && room.second.has_left)
                temp = temp + "1" + "0" + " ";
            if (!room.first.has_left && !room.second.has_left)
                temp = temp + "1" + "1" + " ";
        }
        cout << temp << endl;
    }
public:
    vector<Customer> RoomsForOne;
    vector<DoubleRoom> RoomsForTwo;

    Hotel(){
        RoomsForOne = vector<Customer>(10);
        RoomsForTwo = vector<DoubleRoom>(15);
        for(int i = 0; i < 15; i++)
            RoomsForTwo[i] = DoubleRoom();
    }
    /**
     * Заселение человека в отель
     * @param newcomer
     */
    void NewCustomer(Customer newcomer){
        srand(time(0) * newcomer.GetDays());
        int a = rand() % 1000;
        ///Задержка для потоков, чтоб все посетители не пришли в отель в один день (один и тот же момент)
        this_thread::sleep_for(chrono::milliseconds(a) * 30);
        //this_thread::sleep_for(chrono::seconds(rand()%30));
        mtx.lock();
        ///Поиск одиночной комнаты
        int freeroom = GetFreeRoomIndex();
        ///Если поиск успешен, то проиходит заселение и выход из метода
        if (freeroom != -1){

            RoomsForOne[freeroom] = newcomer;
            RoomsForOne[freeroom].has_left = false;
            msg_mutex.lock();

            msg_mutex.unlock();
            roomsforone++;

            mtx.unlock();
            msg_mutex.lock();
            cout << "DAY " << round(a * 30 / 1000 + 0.5) << ":    " << newcomer.GetName()
                << " is going to live in a room for one person for "<< newcomer.GetDays() << " days." << endl;

            msg_mutex.unlock();
            this_thread::sleep_for(chrono::seconds(newcomer.GetDays()));
            msg_mutex.lock();
            cout << "DAY " << round(a * 30 / 1000 + 0.5) + newcomer.GetDays()
                << ":    " << newcomer.Print() << " is leaving the hotel." << endl;

            msg_mutex.unlock();

            mtx.lock();
            RoomsForOne[freeroom].has_left = true;
            roomsforone--;
            mtx.unlock();
            return;
        }
        mtx.unlock();

        mtx.lock();
        ///Поиск подходящей двойной комнаты для посетителя
        int freedoubleroom = GetFreeDoubleRoomIndex(newcomer);
        ///Если такая комната найдена, то происходит заселение
        if (freedoubleroom != -1){
            RoomsForTwo[freedoubleroom].Settle(newcomer,  round(a * 30 / 1000 + 0.5));

            mtx.unlock();
            this_thread::sleep_for(chrono::seconds(newcomer.GetDays()));

            msg_mutex.lock();
            cout << "DAY " << round(a * 30 / 1000 + 0.5) + newcomer.GetDays() << ":    "
                << newcomer.Print() << " is leaving the hotel." << endl;

            msg_mutex.unlock();
            mtx.lock();
            RoomsForTwo[freedoubleroom].MoveOut(newcomer);
            mtx.unlock();
            return;
        }
        mtx.unlock();
        ///Если программа дошла до этого момента, то все номера в отеле заняты и постеителю надо искать ночлег в другом месте
        msg_mutex.lock();
        cout << "DAY " << round(a * 30 / 1000 + 0.5)
            << ":    Hotel is full, " << newcomer.Print() << " had to leave." << endl;
        msg_mutex.unlock();

        return;
    }

};

/**
 * Ввод int значения из промежутку [lower; upper]
 * При неправильном вводе выведется соответствующее сообщение и запрос на ввод повторится
 * @param lower
 * @param upper
 * @return
 */
int GetInt(int lower, int upper){
    int res;
    cout << "Please input a number between " << lower << " and " << upper << "." << endl;
    while (!(cin >> res) || res < lower || res > upper){
        cout << "Wrong input, please, try again.";
        cin.clear();
        cin.sync();
    }
    return res;
}


int main() {
    ///Ввод числа посетителей
    cout << "Input number of customers";
    int number = GetInt(1, 750);
    Hotel hotel;
    vector<Customer> people(number);
    ///Создание посетителей
    for(int i = 0; i < number; i++){
        people[i] = Customer();
    }

    thread clients[number];
    ///Создание потока для каждого посетителя
    for (int i = 0; i < number; ++i) {
        clients[i] = thread(&Hotel::NewCustomer, ref(hotel), people[i]);
    }
    cout << "Waiting for the first customers..." << endl;
    ///Запуск потока для каждого посетителя
    for (int i = 0; i < number; i++)
        clients[i].join();
    cout << endl << "Everybody has left the Hotel.";
    return 0;
}