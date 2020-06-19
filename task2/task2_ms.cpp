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

Структура содержит анкетные данные студентов
(ФИО, группа, дата рождения, номер комнаты в общежитии).


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

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <fstream>


int PA;         // способ обработки массива
int MONTH;      // заданный месяц
int N;          // размер массива элементов
int M;          // Количество параллельных потоков (если 0, то принимается равным числу процессорных ядер в системе)
int PT;         //Пауза после обработки каждого элемента массива, мс


std::mutex mut_cout;

struct SemaphoreException
{
    const char* what() const {return "SemaphoreException";}
};


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
            Student(std::string str){
                std::istringstream iss(str);
                std::string w;
                std::vector<std::string> vs;

                while(!iss.eof())
                {
                    if(getline(iss,w,','))
                        vs.push_back(w);
                }

                fio = vs[0];
                group = vs[1];

                year = std::atoi(vs[2].c_str());
                if(!year)
                    year = Random(1960, 2000);
                
                month = std::atoi(vs[3].c_str());
                if(!month)
                    month = Random(1, 12);

                day = std::atoi(vs[4].c_str());
                if(!day)
                    day = Random(1, 30);

                if(vs.size() != 6)
                    room_number = Random(1, 200);
                else
                    room_number = std::atoi(vs[5].c_str());
                if(!room_number)
                    room_number = Random(1, 200);
            }

        int Random(int min, int max) {
            return min + rand() % (max - min);
        }

        void display(){
            std::cout << fio << ", " << group << ", " << year << ", " << month << ", " << day << ", " << room_number << "\n";
    }
};


// парсинг файла с входными данными
std::vector<Student> load_and_set(std::string filename)
{
    std::vector<Student> arr_st;
    std::string str;

    std::ifstream infile (filename);

    if(!infile)
    {
        std::cout << "File cannot be opened: " << filename << "\n";
        std::cout << "Error code: " <<  infile.rdstate() << "\n";
        exit(-1);
    }

    while (!infile.eof())
    {
        srand(time(0));

        infile >> str;
        if(str == "PA")
            infile >> PA;
        else if(str == "M")
        {
            infile >> M;
            if(M == 0)
                M = std::thread::hardware_concurrency();
            std::cout << "Set to hardware concurrency: " << M << "\n";
        }
        else if(str == "MONTH")
            infile >> MONTH;
        else if(str == "PT")
            infile >> PT;
        else if(str == "N")
        {
            infile >> N;
            for(int j=0; j<=N;j++)
            {
                getline(infile, str);
                if(str.size() == 0)
                    continue;
                arr_st.push_back({str});
            }
        }
    }
    infile.close();

    return arr_st;
}

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


// Семафор ПЕТРИ
class SemaphorePetry{
    private:
        int count;
        mutable std::mutex mut;
    public:
        SemaphorePetry(){count = 1;}
        SemaphorePetry(SemaphorePetry const&)=delete;   // блокировка конструктор копирования
        SemaphorePetry& operator=(SemaphorePetry const&)=delete; // блокировка оператора присваивания объекта

        void pos()       // увеличивает счётчик
        {
            std::lock_guard<std::mutex> lk(mut);
            if(count == 0)
                count++;
            else
                throw SemaphoreException(); // особый случай, если счётчик уже увеличен
        }

        int nowait()   // неблокированное уменьшение счётчика
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
        MySafeQueue(const std::vector<Student>& st){    //  конструктор, инициализирует очередь списком студентов
            for(const auto& it: st)
                arr_st.push_back(it);
        }

        bool try_pop(Student& st)   // неблокирующее получение элемента очереди через ссылку
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



void thread_job_var2(std::vector<Student>& arr_st, std::vector<SemaphorePetry>& vsem, int month)
{
    for(int j=0; j<N; j++)   // Для каждого элемента массива
    {
        int stat_sem = vsem[j].nowait();   // занять семафор элемента неблокируемым способом

        if(stat_sem == -1)  
            continue;   // семафор занят, идём дальше
        else
        {   
            work_with_student(arr_st[j], month);
        }
    }
}


void thread_job_var4(MySafeQueue& myqueue, int month)
{   

    // std::unique_lock<std::mutex> lk(mut_cout);   // авто-блокировка потока вывода
    // std::cout << std::this_thread::get_id() << ": go to job\n";    // вывод информации на консоль
    // lk.unlock();

    while(true)
    {
        // если очередь пуста
        if(myqueue.empty())
            return; // завершаем работу потока
        
        Student st;     // подготовить структуру
        if(myqueue.try_pop(st)) // активация перехода неблокируемым доступом
        {
            // переход сработал, получили, обрабатываемую структуру
            work_with_student(st, month);   
        }

    }
}

// 1. При помощи массива из M потоков (M ≤ N), используя для синхронизации объект ядра – семафор.

// 2. При помощи массива из M потоков (M ≤ N), используя для синхронизации сеть Петри, моделирующую семафор.
void run_var2(std::vector<Student>& arr_st, int month)
{
    std::cout << "Run case 2...\n";

    std::vector<SemaphorePetry> vsem(N); 

    std::vector<std::thread> v_th;

        for(int j=0; j<M; j++)
        {
            v_th.push_back(std::thread(thread_job_var2, std::ref(arr_st), std::ref(vsem), month));
        }

        for(auto& th: v_th)
            th.join();
}

// 4. При помощи пула из M потоков (M ≤ N), моделируя его при помощи сети Петри.
void run_var4(std::vector<Student>& arr_st, int month)
{
    std::cout << "Run case 4...\n";

    MySafeQueue myqueue(arr_st);    // инициализация очереди

    std::vector<std::thread> v_th;  // объявить массив потоков

    // Каждому потоку дать задачу
    for(int j=0; j<M; j++)
    {
        v_th.push_back(std::thread(thread_job_var4, std::ref(myqueue), month));
    }


    // ждём завершения работы каждого потока
    for(auto& th: v_th) 
        th.join();
}

// Последовательная обработка массива данных
void serial_processing(std::vector<Student>& arr_st, int month)
{
    std::cout << "Run serial processing...\n";

    auto start_time = std::chrono::steady_clock::now();
    for(auto& st: arr_st)
        work_with_student(st, month);
    
    auto stop_time = std::chrono::steady_clock::now();
    std::cout << "TL: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count() << " msec." << std::endl;
}


int main(int argc, char* argv[])
{
    std::vector<Student> arr_st;

    if(argc == 2)
        arr_st = load_and_set(argv[1]);
    else
    {
        std::cout << "Unknown command-line options...\n";
        exit(-1);
    }

    if(PA > 4 || PA < 1)
    {
        std::cout << "Selected the unknown variant PA = " << PA << "\n";
        std::cout << "Exit\n"; 
        exit(-1);
    }

    std::cout << "Run parallel processing...\n";

    auto start_time = std::chrono::steady_clock::now();

    switch (PA)
    {
        case 1:{
            std::cout << "Not implemented yet.";
            // run_var1(arr_st, MONTH);
            break;
        }
        case 2:{
            run_var2(arr_st, MONTH);
            break;
        }
        case 3:{
            // run_var3(arr_st, MONTH);
            std::cout << "Not implemented yet.";
            break;
        }
        case 4:{
            run_var4(arr_st, MONTH);
            break;
        }
    }

    auto stop_time = std::chrono::steady_clock::now();
    std::cout << "TP: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count() << " msec." << std::endl;

    serial_processing(arr_st, MONTH);
    return 0;
}
