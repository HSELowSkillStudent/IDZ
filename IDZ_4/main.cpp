//
// Created by kr1sta1l on 13.12.22.
//
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <fstream>
#include <ctime>

#define NUMBER_OF_WORKERS 2

class Pin {
public:
    Pin() {
        /**condition может принимать следующие значения -
         * -1 - Первая группа еще не осмотрела булавку
         * 0 - Первая группа омиотрела булавку - она кривая(шанс кривой булавки = 1 - good_chance)
         * 1 - Первая группа омиотрела булавку - она не кривая(шанс не кривой булавки = good_chance)
         */
        condition_ = -1;
    }

    void setCondition(int value) {
        condition_ = value;
    }

    int getCondition() const {
        return condition_;
    }

    ~Pin() = default;

private:
    int condition_;
};

int size;
int good_chance = 75;
int inspected_chance = 80;

FILE *output = stdout;
static Pin **array;
pthread_mutex_t mutex;

// "Производитель", он же - пара первого и второго рабочих
void *workers12(void *param) {
    fprintf(output, "Workers12 is ready to work! size: %d\n", size);
    if (output != stdout) {
        fprintf(stdout, "Workers12 is ready to work! size: %d\n", size);
    }
    int i;
    // Перебираем булавки
    for (i = 0; i < size; ++i) {
        // Рабочий пытается выбрать булавку в течение какого-то времени
        int choosing_time = rand() % 5 + 1;
        fprintf(output, "Workers12 are choosing pin for %d sec(index %d)\n", choosing_time, i);
        if (output != stdout) {
            fprintf(stdout, "Workers12 are choosing pin for %d sec(index %d)\n", choosing_time, i);
        }
        // "Засыпаем" - поиск булавки требует некоторого времени
        sleep(choosing_time);
        fprintf(output, "Workers12 are going to work with Pin №%d\n", i);
        if (output != stdout) {
            fprintf(stdout, "Workers12 are going to work with Pin №%d\n", i);
        }

        // Заходим в крит. зону - работа с булавкой. Блокируем мьютекс
        pthread_mutex_lock(&mutex);
        // Выбираем i-ю булавку для дальнейшей работы с ней
        Pin *local_pin = array[i];
        // chance - вероятность, что конкретная булавка не погнута
        int chance = rand() % 100 + 1;
        if (chance < good_chance) {
            // Если булавка не погнута - то полируем её(вторым рабочим)
            local_pin->setCondition(1);
            fprintf(output, "Workers12 set condition of Pin №%d to 1\n", i);
            if (output != stdout) {
                fprintf(stdout, "Workers12 set condition of Pin №%d to 1\n", i);
            }
        } else {
            // Если булавка погнута - пропускаем её и переходим к следующей булавке
            local_pin->setCondition(0);
            fprintf(output, "Workers12 set condition of Pin №%d to 0\n", i);
            if (output != stdout) {
                fprintf(stdout, "Workers12 set condition of Pin №%d to 0\n", i);
            }
        }
        fprintf(output, "Workers12 done their job with Pin №%d\n", i);
        if (output != stdout) {
            fprintf(stdout, "Workers12 done their job with Pin №%d\n", i);
        }
        // Выходим из крит. зоны - разблокируем мьютекс
        pthread_mutex_unlock(&mutex);
    }
    // Перебрали все булавки
    fprintf(output, "Workers12 end their job\n");
    if (output != stdout) {
        fprintf(stdout, "Workers12 end their job\n");
    }
    return nullptr;
}

// "Читатель" - он же - третий рабочий
void *worker3(void *param) {
    fprintf(output, "Worker3 is ready to work! size: %d\n", size);
    if (output != stdout) {
        fprintf(stdout, "Worker3 is ready to work! size: %d\n", size);
    }
    int i;
    // Перебираем булавки
    for (i = 0; i < size; ++i) {
        bool waiting = false;
        // Рабочий пытается выбрать булавку в течение какого-то времени
        int choosing_time = rand() % 4 + 1;
        fprintf(output, "Worker3 is choosing Pin №%d for %d sec\n", i, choosing_time);
        if (output != stdout) {
            fprintf(stdout, "Worker3 is choosing Pin №%d for %d sec\n", i, choosing_time);
        }
        // "Засыпаем" - поиск булавки требует некоторого времени
        sleep(choosing_time);
        fprintf(output, "Worker3 is going to work with Pin №%d\n", i);
        if (output != stdout) {
            fprintf(stdout, "Worker3 is going to work with Pin №%d\n", i);
        }

        // Заходим в крит. зону - работа с булавкой. Блокируем мьютекс
        pthread_mutex_lock(&mutex);
        Pin *local_pin = array[i];
        // chance - вероятность, что конкретная булавка проходит контроль качества
        int chance = rand() % 100 + 1;
        if (local_pin->getCondition() == -1) {
            // Пара первого и второго рабочих еще не дошла до этой булавки. Ждем их
            fprintf(output, "Worker3 ahead of Workers12. Waiting for them...\n");
            if (output != stdout) {
                fprintf(stdout, "Worker3 ahead of Workers12. Waiting for them...\n");
            }
            waiting = true;
        } else if (local_pin->getCondition() == 1){
            // Пара первого и второго рабочих еще дошла до этой булавки. Осуществляем контроль качества
            if (chance < inspected_chance) {
                // Это хорошая булавка
                fprintf(output, "Worker3 says this is a good Pin(№%d)\n", i);
                if (output != stdout) {
                    fprintf(stdout, "Worker3 says this is a good Pin(№%d)\n", i);
                }
            } else {
                // Это плохая булавка
                fprintf(output, "Worker3 says this is a bad Pin(№%d)\n", i);
                if (output != stdout) {
                    fprintf(stdout, "Worker3 says this is a bad Pin(№%d)\n", i);
                }
            }
        } else {  // local_pin->getCondition() == 0
            // Эта булавка погнутая - её не надо проверять
            fprintf(output, "Worker3 says this is a broken Pin(№%d)\n", i);
            if (output != stdout) {
                fprintf(stdout, "Worker3 says this is a broken Pin(№%d)\n", i);
            }
        }
        pthread_mutex_unlock(&mutex);
        if (waiting) {
            i--;
            continue;
        }
        fprintf(output, "Worker3 done his job with Pin №%d\n", i);
        if (output != stdout) {
            fprintf(stdout, "Worker3 done his job with Pin №%d\n", i);
        }
    }
    fprintf(output, "Workers3 end his job\n");
    if (output != stdout) {
        fprintf(stdout, "Workers3 end his job\n");
    }
    return nullptr;
}

int scanChance(const std::string& text, int left_border, int right_border) {
    // Фукнкция заставляет пользователя вводить значения доо тех пор, пока 
    // введенное значение не будет входить в диапазон (left_border, right_border]
    std::string str;
    int result;
    while (true) {
        std::cout << text;
        std::cin >> str;
        try {
            result = std::stoi(str);
            if (left_border > result || right_border <= result) {
                continue;
            }
        } catch (std::invalid_argument e) {
            continue;
        }

        return result;
    }
}

void checker() {
    // Проверка количества булавок, шанса взять не кривую булавку 
    // и шанса прохождения обработанной булавки контроля качества
    // на удовлетворение условиям
    if (!(0 < size && 100000 >= size)) {
        printf("The number of pins must be in the range from 1 to 100000\n");
        if (output != stdout) {
            fprintf(output, "The number of pins must be in the range from 1 to 100000\n");
        }
        exit(-1);
    }
    if (!(0 < good_chance && 100 >= good_chance)) {
        printf("The number of how many of the 100 potential pins may not be curves must be in the range "
               "from 1 to 100\n");
        if (output != stdout) {
            fprintf(output, "The number of how many of the 100 potential pins may not be curves must be in the range "
               "from 1 to 100\n");
        }    
        exit(-1);
    }
    if (!(0 < inspected_chance && 100 >= inspected_chance)) {
        printf("The number of skill score for the second worker must be in the range from 1 to 100\n");
        if (output != stdout) {
            fprintf(output, "The number of skill score for the second worker must be in the range from 1 to 100\n");
        }
        exit(-1);
    }
}

void file_input(int argc, char *argv[]) {
    if (argc == 4) {
        FILE *input_file, *output_file;
        input_file = fopen(argv[2], "r");
        if (!input_file) {
            printf("Cant' open input file\n");
            exit(-1);
        }

        fscanf(input_file, "%d %d %d", &size, &good_chance, &inspected_chance);
        printf("Size: %d\n"
               "good_chance: %d\n"
               "inspected_chance: %d\n", size, good_chance, inspected_chance);

        fclose(input_file);
        output_file = fopen(argv[3], "w");
        if (!output_file) {
            printf("Cant' open output file\n");
            exit(-1);
        }
        // Очищаем файл, куда дальше будет вывод
        fclose(output_file);
        // Открываем на ввод
        output = fopen(argv[3], "a");
        checker();
    } else {
        printf("Invalid input\n");
    }
}

void arg_input(int argc, char *argv[]) {
    if (argc == 2) {
        size = atoi(argv[1]);
        if ((0 > size || 100000 <= size)) {
            printf("The number of pins should be in the range from 1 to 100000\n");
            exit(-1);
        }
    } else if (argc == 4) {
        size = atoi(argv[1]);
        good_chance = atoi(argv[2]);
        inspected_chance = atoi(argv[3]);
        checker();
    } else {
        printf("Invalid input\n");
    }
}

void random_input() {
    size = (rand() % 100000) + 1;
    // size = (rand() % 5) + 1;
    good_chance = 50 + rand() % 51;
    inspected_chance = 50 + rand() % 51;
    printf("Size: %d\n"
           "good_chance: %d\n"
           "inspected_chance: %d\n", size, good_chance, inspected_chance);
}

void default_input() {
    size = scanChance("Enter the volume of the batch of pins (number of pins must be from 1 to 100000):\n", 0, 100000);
    good_chance = scanChance("Enter how many of the 100 potential pins may not be curves(from 1 to 100):\n", 0, 100);
    inspected_chance = scanChance("Enter the skill score for the second worker(from 1 to 100). "
                                  "The higher the skill score, the more often after sharpening the pin, it will be "
                                  "in good condition:\n", 0, 100);
}

int main(int argc, char *argv[]) {
    srand(time(nullptr));
    if (argc == 1) {
        default_input();
    } else {
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
            printf("keys:\n"
                   "-h (--help)  - displays a list of keys\n"
                   "-r [int] (--random [int])  - sets a random data set. \n"
                   "-f [string] [string] (--file [string] [string])  - the program works with files entered by "
                   "the following two arguments\n");
            return(0);
        } else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--random")) {
            random_input();
        } else if (!strcmp(argv[1], "-f") || !strcmp(argv[1], "--file")) {
            file_input(argc, argv);
        } else {
            arg_input(argc, argv);
        }
    }

    // Инициализация массива
    static Pin **local_array = new Pin *[size];
    for (int i = 0; i < size; i++) {
        local_array[i] = new Pin();
    }
    // Передаем ссылку на локальный массив Pin'ов глобальной переменной
    array = local_array;

    // Инициализация массива потоков
    pthread_t threads[NUMBER_OF_WORKERS];

    // Инициализация мьютекса
    pthread_mutex_init(&mutex, nullptr);

    int workers[NUMBER_OF_WORKERS];
    int i;
    // Загружаем первую "половину" рабочих - пар первого и второго  рабочих
    for (i = 0; i < NUMBER_OF_WORKERS / 2; i++) {
        workers[i] = i + 1;
        pthread_create(&threads[i], nullptr, workers12, (void *) (workers + i));
    }
    // Загружаем вторую "половину" рабочих - третьего рабочего
    for (i = NUMBER_OF_WORKERS / 2; i < NUMBER_OF_WORKERS; i++) {
        workers[i] = i + 1;
        pthread_create(&threads[i], nullptr, worker3, (void *) (workers + i));
    }

    // Ждем, пока все потоки(все рабочие) закончат свою работу
    for (int j = 0; j < NUMBER_OF_WORKERS; j++) {
        pthread_join(threads[j], nullptr);
    }
    // уничтожение мьютекса
    pthread_mutex_destroy(&mutex);

    // Очистка памяти
    for (int j = 0; j < size; j++) {
        delete local_array[j];
    }
    delete[] local_array;
    return 0;
}

