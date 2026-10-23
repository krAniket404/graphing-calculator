#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <iosfwd>
#include <string>

constexpr int ROWS = 50;
constexpr int COLS = 100;

class ExpressionParser {
private:
    std::string expression;
    std::size_t pos;
    double xValue;

    void skipSpaces();
    bool match(char c);

    double parseExpression();
    double parseTerm();
    double parseUnary();
    double parsePower();
    double parsePrimary();
    std::string parseName();
    double parseFunction(const std::string&);

public:
    ExpressionParser(const std::string&, double);
    double evaluate();
};

class Graph {
private:
    char screen[ROWS][COLS];
    bool isGraph[ROWS][COLS];
    std::string expression;

public:
    Graph();

    void plot(const std::string&);
    void plot(const std::string&, double, double);

    friend std::istream& operator>>(std::istream&, Graph&);
    friend std::ostream& operator<<(std::ostream&, const Graph&);

    void initialize();
    void drawAxes();
    void putGraphPoint(int, int);

    friend void plotGraph(const std::string&, Graph&, double, double);
};

void plotGraph(const std::string&, Graph&, double, double);
void plotGraph(const std::string&, Graph&);

double evaluateExpression(const std::string&);
double evaluateExpression(const std::string&, double);

#endif