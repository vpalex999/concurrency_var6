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
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <mutex>

#define err_exit_sysV(str){ perror(str); std::cerr << std::endl; exit(EXIT_FAILURE);}

int PA;
int MONTH;
const int ARR_SIZE = 8;
const int M = 3;
const int PT = 1000; 


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

// Семафор ядра sysV
int get_sysv_sem()
{
    const std::string f_key = "/tmp/sem.key";

    key_t key = ftok(f_key.c_str(), 1);
    if(key == -1)
        err_exit_sysV(("Cannot create key for System V IPC: " + f_key).c_str());

    

    int semid = semget(key, ARR_SIZE, IPC_CREAT|0666);
    if(semid == -1)
        err_exit_sysV("Cannot create semaphores");

    struct sembuf mysops[ARR_SIZE];

    // инициализация семафоров
    for(short j=0; j<ARR_SIZE;j++)
    {
        mysops[j].sem_num = j;
        mysops[j].sem_op = 2;
        mysops[j].sem_flg = IPC_NOWAIT;
    }

    int stat_sem = semop(semid, mysops, ARR_SIZE);
    if(stat_sem == -1)
    {
        // удаление семафоров
        semctl(semid, IPC_RMID, 0); // удаление сегмента общей памяти
        err_exit_sysV("Cannot initialize semaphore");
    }
    return semid;
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


void thread_job_var1(std::vector<Student>& arr_st, int& semid, int month)
{
    struct sembuf mybuf;

    mybuf.sem_op = -2;
	mybuf.sem_flg = IPC_NOWAIT;

    for(int j=0; j<ARR_SIZE; j++)
    {
        mybuf.sem_num = j;
        
        int stat_sem = semop(semid, &mybuf, 1);

        if(stat_sem == -1)
            continue;
        if(arr_st[j].month == month)
        {   
            std::lock_guard<std::mutex> lk(mut_cout);
            std::cout << std::this_thread::get_id() << ": ";
            arr_st[j].display();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(PT));
    }
}


void thread_job_var2(std::vector<Student>& arr_st, std::vector<MySem>& vsem, int month)
{
    for(int j=0; j<ARR_SIZE; j++)
    {
        int stat_sem = vsem[j].trywait();

        if(stat_sem == -1)
            continue;
        if(arr_st[j].month == month)
        {   
            std::lock_guard<std::mutex> lk(mut_cout);
            std::cout << std::this_thread::get_id() << ": ";
            arr_st[j].display();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(PT));
    }
}


// 1. При помощи массива из M потоков (M ≤ N), используя для синхронизации объект ядра – семафор.
void run_var1(std::vector<Student>& arr_st, int month)
{
    int semid = get_sysv_sem();

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

}


// 4. При помощи пула из M потоков (M ≤ N), моделируя его при помощи сети Петри.
void run_var4(std::vector<Student>& arr_st, int month)
{
    
}

int main(){

    std::vector<Student> arr_st = get_test_data();

    std::cout << "Choose the variant and month ([1,2,3,4] [1-12]): ";
    std::cin >> PA;
    std::cin >> MONTH;

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

    return 0;
}