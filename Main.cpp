#include <iostream>
#include <windows.h>
#include <string>
#include <vector>

using namespace std;

bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        HANDLE hReadPipe = (HANDLE)stoull(argv[1]);
        HANDLE hWritePipe = (HANDLE)stoull(argv[2]);

        int bounds[2];
        DWORD readBytes;
        ReadFile(hReadPipe, bounds, sizeof(bounds), &readBytes, NULL);

        int start = bounds[0], end = bounds[1];
        vector<int> primes;
        for (int i = start; i <= end; i++) {
            if (isPrime(i)) primes.push_back(i);
        }

        int size = primes.size();
        DWORD writtenBytes;
        WriteFile(hWritePipe, &size, sizeof(int), &writtenBytes, NULL);
        if (size > 0) {
            WriteFile(hWritePipe, primes.data(), size * sizeof(int), &writtenBytes, NULL);
        }
        return 0;
    }

    HANDLE hReadP2C[10], hWriteP2C[10];
    HANDLE hReadC2P[10], hWriteC2P[10];
    HANDLE hProcesses[10];

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    for (int i = 0; i < 10; i++) {
        CreatePipe(&hReadP2C[i], &hWriteP2C[i], &sa, 0);
        CreatePipe(&hReadC2P[i], &hWriteC2P[i], &sa, 0);

        SetHandleInformation(hWriteP2C[i], HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(hReadC2P[i], HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        char cmd[256];

        snprintf(cmd, sizeof(cmd), "\"%s\" %llu %llu", argv[0],
            (unsigned long long)hReadP2C[i], (unsigned long long)hWriteC2P[i]);

        CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
        hProcesses[i] = pi.hProcess;
        CloseHandle(pi.hThread);

        int bounds[2] = { i * 1000 + 1, (i + 1) * 1000 };
        DWORD writtenBytes;
        WriteFile(hWriteP2C[i], bounds, sizeof(bounds), &writtenBytes, NULL);
    }

    for (int i = 0; i < 10; i++) {
        int size;
        DWORD readBytes;
        ReadFile(hReadC2P[i], &size, sizeof(int), &readBytes, NULL);

        if (size > 0) {
            vector<int> primes(size);
            ReadFile(hReadC2P[i], primes.data(), size * sizeof(int), &readBytes, NULL);

            cout << "Procesul " << i + 1 << " a gasit numerele prime din interval:\n";
            for (int j = 0; j < size; j++) {
                cout << primes[j] << " ";
            }
            cout << "\n\n";
        }

        CloseHandle(hWriteP2C[i]);
        CloseHandle(hReadP2C[i]);
        CloseHandle(hWriteC2P[i]);
        CloseHandle(hReadC2P[i]);
    }

    WaitForMultipleObjects(10, hProcesses, TRUE, INFINITE);
    for (int i = 0; i < 10; i++) CloseHandle(hProcesses[i]);

    cout << "Toate procesele s-au incheiat!" << endl;
    return 0;
}