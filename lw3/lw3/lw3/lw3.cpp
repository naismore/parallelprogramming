#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>

HANDLE mutex;

DWORD WINAPI ThreadProc(CONST LPVOID lpParam) {
    int threadNumber = *(int*)lpParam;
    const int operationsCount = 100; // Количество операций в каждом потоке

    for (int i = 0; i < operationsCount; ++i) {
        DWORD startTime = GetTickCount(); // Получаем текущее время в миллисекундах
        Sleep(10); // Имитация работы
        DWORD elapsedTime = GetTickCount() - startTime; // Вычисляем время выполнения операции

        WaitForSingleObject(mutex, INFINITE); // Захватываем мьютекс для записи в файл
        std::ofstream outFile("execution_times.txt", std::ios::app);
        outFile << threadNumber << "|" << elapsedTime << std::endl; // Записываем номер потока и время выполнения
        outFile.close();
        ReleaseMutex(mutex); // Освобождаем мьютекс
    }

    ExitThread(0);
}

int main() {
    try {        
        std::cout << "Нажмите Enter для начала..." << std::endl;
        std::cin.get(); // Ожидание ввода пользователя

        HANDLE* handles = new HANDLE[2];

        mutex = CreateMutex(NULL, FALSE, NULL);

        handles[0] = CreateThread(NULL, 0, &ThreadProc, new int(1), CREATE_SUSPENDED, NULL);
        handles[1] = CreateThread(NULL, 0, &ThreadProc, new int(2), CREATE_SUSPENDED, NULL);
        
        ResumeThread(handles[0]);
        ResumeThread(handles[1]);

        SetThreadPriority(handles[1], THREAD_PRIORITY_HIGHEST);

        WaitForMultipleObjects(2, handles, TRUE, INFINITE);

        delete[] handles;
        CloseHandle(mutex);
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
