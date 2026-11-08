#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

using namespace std;

bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

int main() {
    int p2c[10][2]; 
    int c2p[10][2]; 
    pid_t pids[10];

    for (int i = 0; i < 10; i++) {
        pipe(p2c[i]);
        pipe(c2p[i]);

        pids[i] = fork();
        if (pids[i] == 0) { 
            close(p2c[i][1]); 
            close(c2p[i][0]); 

            int start, end;
            read(p2c[i][0], &start, sizeof(int));
            read(p2c[i][0], &end, sizeof(int));

            vector<int> primes;
            for (int n = start; n <= end; n++) {
                if (isPrime(n)) primes.push_back(n);
            }

            int size = primes.size();
            write(c2p[i][1], &size, sizeof(int));
            if (size > 0) {
                write(c2p[i][1], primes.data(), size * sizeof(int));
            }

            close(p2c[i][0]);
            close(c2p[i][1]);
            return 0; 
        } else {
            close(p2c[i][0]);
            close(c2p[i][1]);

            int start = i * 1000 + 1;
            int end = (i + 1) * 1000;
            write(p2c[i][1], &start, sizeof(int));
            write(p2c[i][1], &end, sizeof(int));
        }
    }

    for (int i = 0; i < 10; i++) {
        int size;
        read(c2p[i][0], &size, sizeof(int));
        if (size > 0) {
            vector<int> primes(size);
            read(c2p[i][0], primes.data(), size * sizeof(int));
            
                       cout << "Procesul " << i + 1 << " a gasit numerele prime din interval: \n";
            for (int j = 0; j < size; j++) {
                cout << primes[j] << " ";
            }
            cout << "\n\n";
        }
        close(p2c[i][1]);
        close(c2p[i][0]);
        waitpid(pids[i], NULL, 0);
    }
    
    cout << "Toate procesele s-au incheiat!" << endl;
    return 0;
}