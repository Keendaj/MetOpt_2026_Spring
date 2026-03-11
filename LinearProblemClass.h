#pragma once
#include <vector>
#include <string>

enum class ConstraintType {
    GEQ = 1,
    LEQ = -1,
    EQ = 0
};

enum class VarSign {
    POSITIVE = 1,
    NEGATIVE = -1,
    FREE = 0
};

enum class ProblemForm {
    GENERAL,   
    CANONICAL,  
    SYMMETRIC   
};

class LPProblem {
    private:
        bool isMin;

        std::string normalizeExpression(const std::string& input);
        void parseExpression(const std::string& expr, std::vector<double>& row);
        
        void parseObjectiveFunction();
        void parseConstraints();
        void parseVariableSigns();

        void normalizeVectorSizes();

        void printTerm(double coeff, const std::string& varStr, bool& isFirst);
        void printObjective();      
        void printConstraints();        
        void printVariableConstraints();
        
        void pivot(std::vector<std::vector<double>>& table,
            std::vector<int>& basis,
            int pivotRow,
            int pivotCol);

        bool phase1(std::vector<std::vector<double>>& table, 
            std::vector<int>& basis, 
            int m_canon, 
            int n_canon, 
            const LPProblem& canon,
            double eps, 
            int print_k);

        bool phase2(std::vector<std::vector<double>>& table, 
            std::vector<int>& basis, 
            int m_canon, 
            int n_canon, 
            const LPProblem& canon, 
            double eps, 
            int print_k);

        void printTableau(const std::vector<std::vector<double>>& table, 
            const std::vector<int>& basis, 
            int m_canon, 
            int n_canon, 
            int phase, 
            int step, 
            const LPProblem& canon,
            int minCol, 
            int minRow, 
            const std::vector<double>& phaseObj);
    public:
        int n = 0;
        int m = 0; 
        bool solved = false;
        
        std::vector<std::string> var_names;
        std::vector<std::vector<double>> A;
        std::vector<double> b;
        std::vector<double> c;

        std::vector<ConstraintType> signs;      
        std::vector<VarSign> var_signs;

        std::vector<double> optimalSolution; 
        double optimalValue;
        
        std::vector<double> optimalCanonicalSolution; 
        std::vector<std::string> canonicalVarNames;
        
        std::string getSignStr(ConstraintType type);

        void input();
        
        void printForms();
        bool solveSimplex(double eps = 1e-2, int print_k = 0);
        void solveByVertices();

        void printOriginal();
        void printResult();

        LPProblem toGeneral();
        LPProblem toCanonical();
        LPProblem toSymmetric();
        LPProblem toDual();

        ProblemForm determineForm();
};