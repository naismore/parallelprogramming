#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>

HANDLE mutex;

DWORD WINAPI ThreadProc(CONST LPVOID lpParam) {
    int threadNumber = *(int*)lpParam;
    const int operationsCount = 100; // ���������� �������� � ������ ������

    for (int i = 0; i < operationsCount; ++i) {
        DWORD startTime = GetTickCount(); // �������� ������� ����� � �������������
        Sleep(10); // �������� ������
        DWORD elapsedTime = GetTickCount() - startTime; // ��������� ����� ���������� ��������

        WaitForSingleObject(mutex, INFINITE); // ����������� ������� ��� ������ � ����
        std::ofstream outFile("execution_times.txt", std::ios::app);
        outFile << threadNumber << "|" << elapsedTime << std::endl; // ���������� ����� ������ � ����� ����������
        outFile.close();
        ReleaseMutex(mutex); // ����������� �������
    }

    ExitThread(0);
}

int main() {
    try {        
        std::cout << "������� Enter ��� ������..." << std::endl;
        std::cin.get(); // �������� ����� ������������

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
