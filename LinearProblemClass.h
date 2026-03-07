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
        
    public:
        int n = 0;
        int m = 0; 
        
        std::vector<std::string> var_names;
        std::vector<std::vector<double>> A;
        std::vector<double> b;
        std::vector<double> c;

        std::vector<ConstraintType> signs;      
        std::vector<VarSign> var_signs;
        
        std::string getSignStr(ConstraintType type);

        void input();
        
        void printForms();
        void solveSimplex(bool verbose = true);
        void solveByVertices();

        void printOriginal();
        void printDual();

        LPProblem toGeneral();
        LPProblem toCanonical();
        LPProblem toSymmetric();

        ProblemForm determineForm();
};