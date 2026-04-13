#include <functional>

struct SearchStats {
    int iters;
    int calls;
    int k_teor;
};

double uniformSearch(const std::function<double(double)>& f, double a, double b, double eps = 0.1, int m = 10, bool silent = false, SearchStats* stats = nullptr);

double fibonacciSearch(const std::function<double(double)>& f, double a, double b, double eps, int N, bool silent = false, SearchStats* stats = nullptr);