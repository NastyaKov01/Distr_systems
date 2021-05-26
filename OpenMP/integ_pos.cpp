#include <iostream>
#include <sys/time.h>

double fun(double x)
{
    return x / (x * x * x + x * x + 5 * x + 1);
}

double integral(double a, double b, int n)
{
    double res = fun(a) + fun(b);
    double h = (b - a) / (2 * n);
    double x1 = a + h, x2 = a + 2 * h;
    for (int i = 0; i < n; ++i) {
        res += 4 * fun(x1);
        res += 2 * fun(x2);
        x1 += 2 * h;
        x2 += 2 * h;
    }
    res += fun(b - h);
    res *= (h / 3);
    return res;
}

int main(void)
{
    struct timeval start, finish, diff;
    gettimeofday(&start, NULL);
    double a = 0, b = 200;
    int n = 20000;
    std::cout << integral(a, b, n) << std::endl;
    gettimeofday(&finish, NULL);
    diff.tv_sec = finish.tv_sec - start.tv_sec;
    diff.tv_usec = finish.tv_usec - start.tv_usec;
    if (diff.tv_usec < 0) {
        diff.tv_sec--;
        diff.tv_usec += 1000000;
    }
    std::cout << diff.tv_sec << "." << diff.tv_usec/1000 << "." << diff.tv_usec%1000 << std::endl;
    return 0;
}
