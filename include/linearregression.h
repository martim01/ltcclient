#include <algorithm>
#include <numeric>
#include <list>

using alphabeta = std::pair<double, double>;

alphabeta GetSlopeAndIntercept(const std::list<double>& X, const std::list<double>& Y)
{
    const double n = X.size();
    const double sumX = std::accumulate(X.begin(), X.end(), 0.0);
    const double sumY = std::accumulate(Y.begin(), Y.end(), 0.0);
    const double sumXX = std::inner_product(X.begin(), X.end(), X.begin(), 0.0);
    const double sumXY = std::inner_product(X.begin(), X.end(), Y.begin(), 0.0);

    double a = ((sumY*sumXX)-(sumX*sumXY)) / ((n*sumXX)-(sumX*sumX));
    double b = ((n*sumXY)- (sumX*sumY)) /  ((n*sumXX) - (sumX*sumX));

    return std::make_pair(a,b);
}

