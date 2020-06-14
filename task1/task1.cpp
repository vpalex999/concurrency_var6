#include <iostream>
#include <string>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <condition_variable>

using namespace std::chrono;

std::mutex COUT;

int NP = 1;         // Количество потоков (деталей)
int QT = 100;       // Продолжительность кванта времени, мс

struct status_work
{   private:
        int count;          // счётчик рабочих потоков
        std::mutex mut;
    public:
        status_work(int num_elements):count(num_elements){}

        // префиксное уменьшение счётчика
        void operator--()
        {
            std::lock_guard<std::mutex> lk(mut);
            --count;
        }

        // постфиксное уменьшение счётчика
        void operator--(int)
        {
            std::lock_guard<std::mutex> lk(mut);
            count--;
        }

        // получить статус обработки деталей
        bool done()
        {
            std::lock_guard<std::mutex> lk(mut);
            if(count <= 0)
                return true;
            return false;
        }

} is_work(NP);


struct Element{
    private:
        std::deque<int> machines;
    public:
        std::string name;
        int priority;       // приоритет потока
        int cpu_burst;      // количество квантов времени
        Element(std::string nm, int prt, int burst):name(nm), priority(prt), cpu_burst(burst){}

        void add_machine(int machine)
            {
                machines.push_back(machine);
            }
        
        int get_machine()
            {   
                int m = machines.front();
                machines.pop_front();
                return m;
            }
        
        bool empty()
            {
                return machines.empty();
            }
};


class Machine{
    private:
        mutable std::mutex m_current;   // мьютекс
        std::condition_variable cond;
        int name;
        int count_handlers;     // количество одновременно обрабатываемых изделий
        int processing_time;    // время обработки детали (мсек.)
        
    public:

        Machine(Machine const& other){
            std::lock_guard<std::mutex> lk(other.m_current);    // блокировка нарушения инвариантов 
            name = other.name;                                  // при пременении конструктора копирования
            count_handlers = other.count_handlers;
            processing_time = other.processing_time;
        };

        Machine& operator=(Machine const&)=delete;

        Machine(int n, int hndl, int ptime):name(n), count_handlers(hndl), processing_time(ptime){}

        int get_name(){return name;}
        int get_proc_time(){return processing_time;}
        
        void display()
        {
            std::cout << "Machine: name: " << name  << ", number of handlers: " << count_handlers << ", processing_time(msec.): " << processing_time << "\n";
        }

        void start_work()
        {
            std::unique_lock<std::mutex> lk(m_current);
            cond.wait(lk, [this]{return count_handlers > 0;});

            // обработка детали
            // std::unique_lock<std::mutex> cout_start(COUT);
            // std::cout << std::this_thread::get_id() << ": start work ...\n";
            // cout_start.unlock();

            count_handlers--;
            // lk.unlock();
            // std::this_thread::sleep_for(std::chrono::milliseconds(processing_time)); // Имитация обработки детали

            // std::unique_lock<std::mutex> cout_end(COUT);
            // std::cout << std::this_thread::get_id() << ": machine=" << name << " processing of detail... " << processing_time << " msec.\n";
            // cout_end.unlock();
        
            // end_work();
        }

        void end_work()
        {
            count_handlers++;
            cond.notify_one();

            // std::unique_lock<std::mutex> c_out(COUT);
            // std::cout << std::this_thread::get_id() << ": end work ...\n";
            // c_out.unlock();
        }
};


void element_process(Element el, std::vector<Machine>& vms)
    {   
        // цикл обработки детали на станке
        while(!el.empty()){
            int num_machine = el.get_machine(); // получить номер следующего станка
            bool status = false;                // статус обработки детали на следующем станке

            for(int j=0; j<vms.size(); j++)     // проходим по массиву станков
            {
                if(num_machine == vms[j].get_name())    // если нашли станок
                {
                    status = true;          // есть станок для обработки
                    vms[j].start_work();    // занять станок
                    is_work--;              // убрать деталь из общего счетчика деталей которые в работе

                    // обработка детали по тикам
                    for(int j=1; j<=el.cpu_burst; j++)
                    {   
                        // вывод в консоль
                        std::unique_lock<std::mutex> cout_start(COUT);
                        std::cout << std::this_thread::get_id() << ": [Tick " << j << "] work " << QT << "msec. Machine " << num_machine << "\n";
                        cout_start.unlock();
                        // заснуть на время тика
                        std::this_thread::sleep_for(std::chrono::milliseconds(QT)); // Имитация обработки детали
                    }

                    vms[j].end_work();      // освободить станок
                    break;  // выйти из цикла поиска станка
                }
            }
            if(!status) // если деталь осталась необработанной
            {
                std::cout << std::this_thread::get_id() << ": " "Not found Machine = " << num_machine << ", detail is brocken...\n";
                is_work--;  // // убрать деталь из общего счетчика деталей которые в работе
                break;  // // выйти из цикла обработки детали
            }
        }
    }


int main(){
    
    Element el1("L1", 5, 10);
    Element el2("L2", 5, 5);
    // Element el3("L3");
    // Element el4("L4");



    el1.add_machine(1);
    el1.add_machine(2);

    // el2.add_machine(1);
    // el2.add_machine(2);

    // el3.add_machine(1);
    // el3.add_machine(2);

    // el4.add_machine(1);
    // el4.add_machine(2);


    std::vector<Machine> vms;
    vms.push_back(Machine(1, 2, 1000));
    vms.push_back(Machine(2, 1, 500));

    std::thread t1(element_process, el1, std::ref(vms));
    // std::thread t2(element_process, el2, std::ref(vms));
    // std::thread t3(element_process, el3, std::ref(vms));
    // std::thread t4(element_process, el4, std::ref(vms));




    t1.join();
    // t2.join();
    // t3.join();
    // t4.join();

    int Tick = 1;

    while (!is_work.done())
    {
        std::cout << "Start Tick " << Tick << std::endl;
        std::cout << "End Tick " << Tick << std::endl;
        Tick++;
    }
    
    std::cout << "is_work done\n";



    return 0;
}

