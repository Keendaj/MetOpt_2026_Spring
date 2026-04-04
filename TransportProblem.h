#pragma once
#include <vector>
#include <string>
#include <iostream>

struct TransportCell {
    double amount = 0.0;
    bool isBasic = false;
};

class TransportProblem {
public:
    int m;
    int n;

    std::vector<double> A;
    std::vector<double> B;
    std::vector<std::vector<double>> C; 

    std::vector<std::vector<TransportCell>> plan;
    double optimalPotentialValue;

    std::vector<double> optimalSimplexPlan;
    double optimalSimplexValue;

    void input();
    void setData(const std::vector<double>& sup, const std::vector<double>& dem, const std::vector<std::vector<double>>& costs);

    bool solvePotential(int print_k = 0);
    void printPotentialPlan();
    void printTransportTable(int step, const std::vector<double>& u, const std::vector<double>& v);

    bool solveSimplex(double eps = 1e-7, int print_k = 0);
    void printSimplexPlan();
    
    
    
    void convertToOpenWithPenalties(int variant_number, const std::vector<double>& penalties);

private:
    void buildNorthWestCorner();
    bool calculatePotentials(std::vector<double>& u, std::vector<double>& v);
    bool findCycleIterative(int start_r, int start_c, bool startHorizontal, vector<pair<int, int>>& path);

    void JordanStep(std::vector<std::vector<double>>& table, std::vector<int>& basis, int pivotRow, int pivotCol);
    
    bool phase1(std::vector<std::vector<double>>& table, std::vector<int>& basis, int vars, int eqs, double eps, int print_k);
    bool phase2(std::vector<std::vector<double>>& table, std::vector<int>& basis, int vars, int eqs, double eps, int print_k);
    
    void printSimplexTable(const std::vector<std::vector<double>>& table, const std::vector<int>& basis, int vars, int eqs, int phase, int step);
};