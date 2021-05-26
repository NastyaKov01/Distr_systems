#include <iostream>
#include <sys/time.h>
#include <sstream>

double fun(double x)
{
    return x / (x * x * x + x * x + 5 * x + 1);
}

double integral(double a, double b, int n)
{
    double res = 0; 
    double h = (b - a) / (2 * n);
    double x1, x2;
    for (int i = 1; i <= n; ++i) {
        x1 = a + (2 * i - 1) * h;
        x2 = a + 2 * h * i;
        res += 4 * fun(x1);
        res += 2 * fun(x2);
    }
    res += fun(b - h);
    res += fun(a) + fun(b);
    res *= (h / 3);
    return res;
}

int main(int argc, char **argv)
{
    struct timeval start, finish, diff;
    int n, num_procs, rank;

    std::stringstream s1;
    s1 << argv[1];
    s1 >> num_procs;
    std::stringstream s2;
    s2 << argv[2];
    s2 >> n;
    double a = 0, b = 200;
    gettimeofday(&start, NULL);
    std::cout << "The result is " << integral(a, b, n) << std::endl;
    gettimeofday(&finish, NULL);
    diff.tv_sec = finish.tv_sec - start.tv_sec;
    diff.tv_usec = finish.tv_usec - start.tv_usec;
    if (diff.tv_usec < 0) {
        diff.tv_sec--;
        diff.tv_usec += 1000000;
    }
    std::cout << "Time passed: " << diff.tv_sec << "." << diff.tv_usec/1000 << "." << diff.tv_usec%1000 << std::endl;
    return 0;
}
