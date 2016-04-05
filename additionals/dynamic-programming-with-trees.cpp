#include <iostream>

using namespace std;

long long int fibonacci1(long long int n) {
    if (n == 0 || n == 1) return n;
    return fibonacci1(n - 1) + fibonacci1(n - 2);
}

long long int fibonacci(long long int n) {
    if (n == 0 || n == 1) return n;
    // alocate our memorization array
    long long int memory[n + 1];
    // initialize the base cases
    memory[0] = 0; memory[1] = 1;
    // solve all smaller sub problems until getting to our goal
    for (long long int i = 2; i <= n; ++i)
        memory[i] = memory[i - 1] + memory[i - 2];
    return memory[n];
}

int main() {
    for (long long int i = 0; i < 10000; ++i)
        cout << "n: " << i << " | fib(n): " << fibonacci(i) << endl;
    cout << sizeof(int) << endl;
    return 0;
}

