#include <iostream>
#include <future>
#include <thread>
using namespace std;

void addNumbers(promise<int> promise, int n)
{
    this_thread::sleep_for(std::chrono::seconds(4));
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += i;
    }
    promise.set_value(sum); // Set the result
}

void factorial(promise<int> promise, int n)
{
    this_thread::sleep_for(std::chrono::seconds(2));
    int fact = 1;
    for (int i = 1; i <= n; i++) // You should go up to n, not n-1
    {
        fact *= i;
    }
    promise.set_value(fact); // Set the result
}

int main()
{
    // Create promise objects
    promise<int> promise1;  
    promise<int> promise2;

    // Get futures from promises
    future<int> future1 = promise1.get_future();
    future<int> future2 = promise2.get_future();

    int n = 5;

    // Launch threads to compute sum and factorial
    thread t1(addNumbers, move(promise1), n);
    thread t2(factorial, move(promise2), n);

    // Wait for results
    int sum = future1.get();
    cout << "sum = " << sum << endl;

    cout << "waiting" << endl;  // This will print before factorial result.

    int fact = future2.get();
    cout << "factorial = " << fact << endl;

    // Wait for both threads to complete
    t1.join();
    t2.join();

    return 0;
}

