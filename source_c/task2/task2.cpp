/*
Вариант №6

Задание 2


В работе необходимо реализовать многопоточную обработку массива
структур данных (из N элементов) четырьмя способами:

1. При помощи массива из M потоков (M ≤ N), используя для синхро-
низации объект ядра – семафор.

2. При помощи массива из M потоков (M ≤ N), используя для синхро-
низации сеть Петри, моделирующую семафор.

3. При помощи пула из M потоков (M ≤ N), используя системный пул
потоков или асинхронные потоки ввода/вывода.

4. При помощи пула из M потоков (M ≤ N), моделируя его при помощи
сети Петри.

Структура содержит анкетные данные студентов (ФИО,
группа, дата рождения, номер комнаты в общежитии).


PA - Выбранный способ обработки массива
N  - Размер массива структур данных
M  - Количество параллельных потоков (если 0, то принимается равным числу процессорных ядер в системе)
PT - Пауза после обработки каждого элемента массива, мс
... - Дополнительные входные данные (если требуются согласно индивидуальному варианту задания)
... Значения атрибутов каждой структуры данных. Если какие-то атрибуты не заданы (или заданы пустой строкой),
    то генерируются программой случайным образом

Цель:
    Требуется вывести в выходной файл данные о студентах, которые родились в заданном месяце M.

*/

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <semaphore.h>

#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <atomic>
#include <condition_variable>


#define err_exit_sysV(str){ perror(str); std::cerr << std::endl; exit(EXIT_FAILURE);}

int PA;                         // способ обработки массива
int MONTH;                      // заданный месяц
const int ARR_SIZE = 8;         // размер массива элементов
const int M = 8;                // Количество параллельных потоков (если 0, то принимается равным числу процессорных ядер в системе)
const int PT = 2000;            //Пауза после обработки каждого элемента массива, мс
const int POLL_SIZE = 8;        //размер пула потоков
const int PUSH_INTERVAL = 0;  // интервал добавления объектов в очередь мс.


std::mutex mut_cout;

// Структура Студент
struct Student{
    public:
        std::string fio;    // ФИО
        std::string group;  // группа
        int year;           // год рождения
        int month;          // месяц рождения
        int day;            // день рождения
        int room_number;    // номер комнаты в общежитии
            Student(){};
            Student(std::string f, std::string g, int y, int m, int d, int r):fio(f), group(g), year(y), month(m), day(d), room_number(r){};

        void display(){
            std::cout << fio << ", " << group << ", " << year << ", " << month << ", " << day << ", " << room_number << "\n";
    }
};

// Работа с записью студента
void work_with_student(Student& st, const int& month)
{
    if(st.month == month)
    {
        std::lock_guard<std::mutex> lk(mut_cout);   // авто-блокировка потока вывода
        std::cout << std::this_thread::get_id() << ": ";    // вывод информации на консоль
        st.display();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(PT)); // Пауза после обработки каждого элемента массива
}

// тестовые данные
std::vector<Student> get_test_data()
{
    std::vector<Student> arr_st;
    // for(int j=0; j<ARR_SIZE; j++)
    arr_st.push_back({"student1", "c-172", 1974, 3, 8, 22});
    arr_st.push_back({"student2", "c-172", 1975, 3, 29, 136});
    arr_st.push_back({"student4", "c-178", 1989, 3, 25, 47});
    arr_st.push_back({"student3", "c-172", 1987, 3, 5, 46});
    arr_st.push_back({"student5", "c-111", 1989, 3, 30, 48});
    arr_st.push_back({"student6", "c-123", 1990, 3, 2, 246});
    arr_st.push_back({"student7", "c-1345", 1995, 3, 17, 88});
    arr_st.push_back({"student8", "c-45", 1964, 3, 2, 87});

    return arr_st;
}

// получить семафор ядра systemV
int get_sysv_sem(int num_sem, int sem_op, int flag)
{
    const std::string f_key = "/tmp/sem.key";

    key_t key = ftok(f_key.c_str(), 1);
    if(key == -1)
        err_exit_sysV(("Cannot create key for System V IPC: " + f_key).c_str());

    int semid = semget(key, num_sem, IPC_CREAT|0666);
    if(semid == -1)
        err_exit_sysV("Cannot create semaphores");

    struct sembuf mysops[num_sem];

    // инициализация семафоров
    for(short j=0; j<num_sem;j++)
    {
        mysops[j].sem_num = j;
        mysops[j].sem_op = sem_op;
        mysops[j].sem_flg = flag;
    }


    int stat_sem = semop(semid, mysops, num_sem);
    if(stat_sem == -1)
    {
        // удаление семафоров
        semctl(semid, IPC_RMID, 0); // удаление сегмента общей памяти
        err_exit_sysV("Cannot initialize semaphore");
    }
    return semid;
}

// Семафор
class MySem{
    private:
        int count;
        mutable std::mutex mut;
    public:
        MySem(){count = 1;}
        MySem(MySem const&)=delete;   // конструктор копирования
        MySem& operator=(MySem const&)=delete; // присваивание объекта

        int pos()       // увеличивает счётчик
        {
            std::lock_guard<std::mutex> lk(mut);
            count++;
            return 0;
        }

        int trywait()   // неблокированное уменьшение счётчика
        {
            std::lock_guard<std::mutex> lk(mut);
            if(count == 0)
                return -1;
            else
            {
                count--;
                return 0;
            }
        }

        int getvalue()  // получить значение счётчика
        {
            std::lock_guard<std::mutex> lk(mut);
            return count;
        } 
};

// безопасная очередь
class MySafeQueue
{
    private:
        std::deque <Student> arr_st;
        mutable std::mutex mut;
    public:
        std::atomic_bool is_done;
        MySafeQueue(){}
        MySafeQueue(const std::vector<Student>& st){
            for(const auto& it: st)
                arr_st.push_back(it);
        }

        bool try_pop(Student& st)   // неблокирующее получение элемента очереди
        {
            std::lock_guard<std::mutex> lk(mut);
            if(arr_st.empty())
                return false;

            st = arr_st.front();
            arr_st.pop_front();
            return true;
        }

        void push(Student st)
        {
            std::lock_guard<std::mutex> lk(mut);
            arr_st.push_back(st);
        }

        bool empty()
        {
            std::lock_guard<std::mutex> lk(mut);
            return arr_st.empty();
        }
};


void thread_job_var1(std::vector<Student>& arr_st, int& semid, int month)
{
    // структура управления семафором
    struct sembuf mybuf;

    mybuf.sem_op = -2;
	mybuf.sem_flg = IPC_NOWAIT;

    for(int j=0; j<ARR_SIZE; j++)   // Для каждого элемента массива
    {
        mybuf.sem_num = j;  // выбрать семафор элемента
        
        int stat_sem = semop(semid, &mybuf, 1); // занять семафор элемента неблокируемым способом

        if(stat_sem == -1)
            continue;   // семафор занят, идём дальше
        else
        {  
            work_with_student(arr_st[j], month);
        }
    }
}



void thread_job_var2(std::vector<Student>& arr_st, std::vector<MySem>& vsem, int month)
{
    for(int j=0; j<ARR_SIZE; j++)   // Для каждого элемента массива
    {
        int stat_sem = vsem[j].trywait();   // занять семафор элемента неблокируемым способом

        if(stat_sem == -1)  
            continue;   // семафор занят, идём дальше
        else
        {   
            work_with_student(arr_st[j], month);
        }
    }
}


void thread_job_var3(MySafeQueue& myqueue, int& semid, int month)
{
    // структура управления семафором
    struct sembuf mybuf;

    mybuf.sem_num = 0;
	mybuf.sem_flg = 0;

    while(true)
    {
        // Проверка на завершение работы потока
        if(myqueue.is_done and myqueue.empty())
            return;
        
        mybuf.sem_op = -1;

        // занять семафор
        semop(semid, &mybuf, 1);    
        
        Student st;
        if(myqueue.try_pop(st)) // получить объект из очереди
        {
            work_with_student(st, month);
        }
        
        // освободить семафор
        mybuf.sem_op = 1;
        semop(semid, &mybuf, 1);
    }
}


void thread_job_var4(MySafeQueue& myqueue, int month)
{   
    while(true)
    {
        // Проверка на завершение работы потока
        if(myqueue.is_done and myqueue.empty())
            return;

        Student st;
        if(myqueue.try_pop(st)) // неблокируемый доступ к очереди
        {
            work_with_student(st, month);
        }
        else
        {
            std::this_thread::yield();  // заснуть
        }
    }
}

// 1. При помощи массива из M потоков (M ≤ N), используя для синхронизации объект ядра – семафор.
void run_var1(std::vector<Student>& arr_st, int month)
{
    int semid = get_sysv_sem(ARR_SIZE, 2, IPC_NOWAIT);

        std::vector<std::thread> v_th;

        for(int j=0; j<M; j++)
        {
            v_th.push_back(std::thread(thread_job_var1, std::ref(arr_st), std::ref(semid), month));
        }

        for(auto& th: v_th)
            th.join();

        // удаление семафоров
        semctl(semid, IPC_RMID, 0); // удаление сегмента общей памяти
}

// 2. При помощи массива из M потоков (M ≤ N), используя для синхронизации сеть Петри, моделирующую семафор.
void run_var2(std::vector<Student>& arr_st, int month)
{
    std::vector<MySem> vsem(ARR_SIZE);

    std::vector<std::thread> v_th;

        for(int j=0; j<M; j++)
        {
            v_th.push_back(std::thread(thread_job_var2, std::ref(arr_st), std::ref(vsem), month));
        }

        for(auto& th: v_th)
            th.join();
}

// 3. При помощи пула из M потоков (M ≤ N), используя системный пул потоков или асинхронные потоки ввода/вывода.
void run_var3(std::vector<Student>& arr_st, int month)
{
    MySafeQueue myqueue;

    int semid = get_sysv_sem(1, POLL_SIZE, 0);  // семафор пула потоков

        std::vector<std::thread> v_th;

        for(int j=0; j<M; j++)
        {
            v_th.push_back(std::thread(thread_job_var3, std::ref(myqueue), std::ref(semid), month));
        }

        // имитация заполнения очереди
        for(const auto stud:arr_st)
        {
            myqueue.push(stud);
            std::this_thread::sleep_for(std::chrono::milliseconds(PUSH_INTERVAL));
        }

        myqueue.is_done = true;

        for(auto& th: v_th)
            th.join();

        // удаление семафоров
        semctl(semid, IPC_RMID, 0); // удаление сегмента общей памяти
}



// 4. При помощи пула из M потоков (M ≤ N), моделируя его при помощи сети Петри.
void run_var4(std::vector<Student>& arr_st, int month)
{
    MySafeQueue myqueue;

    std::vector<std::thread> v_th;

    for(int j=0; j<M; j++)
    {
        v_th.push_back(std::thread(thread_job_var4, std::ref(myqueue), month));
    }

    // имитация заполнения очереди
    for(const auto stud:arr_st)
    {
        myqueue.push(stud);
        std::this_thread::sleep_for(std::chrono::milliseconds(PUSH_INTERVAL));
    }

    myqueue.is_done = true;

    for(auto& th: v_th)
        th.join();
}


int serial_processing(std::vector<Student>& arr_st, int month)
{
    auto start_time = std::chrono::steady_clock::now();
    for(auto& st: arr_st)
    {
        work_with_student(st, month);
        std::this_thread::sleep_for(std::chrono::milliseconds(PUSH_INTERVAL));
    }
    
    auto stop_time = std::chrono::steady_clock::now();
    std::cout << "TL: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count() << " msec." << std::endl;
}

int main(){

    std::vector<Student> arr_st = get_test_data();

    std::cout << "Choose the variant and month ([1,2,3,4] [1-12]): ";
    std::cin >> PA;
    std::cin >> MONTH;

    std::vector<Student> arr_st_s = get_test_data();
    serial_processing(arr_st_s, MONTH);

    auto start_time = std::chrono::steady_clock::now();

    switch (PA)
    {
    case 1:{
        run_var1(arr_st, MONTH);
        break;
    }
    case 2:{
        run_var2(arr_st, MONTH);
        break;
    }
    case 3:{
        run_var3(arr_st, MONTH);
        break;
    }
    case 4:{
        run_var4(arr_st, MONTH);
        break;
    }
    default:
        std::cout << "Selected the unknown variant\n"; 
        break;
    }

    auto stop_time = std::chrono::steady_clock::now();
    std::cout << "TP: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count() << " msec." << std::endl;


    return 0;
}