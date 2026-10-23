#include "functions.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

const double PI = 3.14159265358979323846;

const char* RED = "\033[31m";
const char* RESET = "\033[0m";

// Helper function: check whether a screen coordinate is valid.
bool isInsideScreen(int row, int col) {
    return row >= 0 && row < ROWS &&
            col >= 0 && col < COLS;
}

// Helper function: convert a mathematical y-coordinate to a screen row.
bool toScreenRow(double y, double yScale, int centerRow, int& row) {
    double rowValue = centerRow - y * yScale;

    if (!std::isfinite(rowValue) ||
        rowValue < 0 || rowValue >= ROWS) {
        return false;
    }

    row = static_cast<int>(std::round(rowValue));

    return row >= 0 && row < ROWS;
}

// EXPRESSIONPARSER IMPLEMENTATION

ExpressionParser::ExpressionParser(const std::string& text, double x) {
    expression = text;
    pos = 0;
    xValue = x;
}

void ExpressionParser::skipSpaces() {
    while (pos < expression.size() &&
           std::isspace(
               static_cast<unsigned char>(expression[pos]))) {
        ++pos;
    }
}

bool ExpressionParser::match(char c) {
    skipSpaces();

    if (pos < expression.size() && expression[pos] == c) {
        ++pos;
        return true;
    }

    return false;
}

double ExpressionParser::parseExpression() {
    double value = parseTerm();

    while (true) {
        if (match('+')) {
            value += parseTerm();
        }
        else if (match('-')) {
            value -= parseTerm();
        }
        else {
            break;
        }

        if (!std::isfinite(value)) {
            throw std::runtime_error("Mathematical result is out of range");
        }
    }

    return value;
}

double ExpressionParser::parseTerm() {
    double value = parseUnary();

    while (true) {
        if (match('*')) {
            value *= parseUnary();
        }
        else if (match('/')) {
            double divisor = parseUnary();

            if (divisor == 0.0) {
                throw std::runtime_error("Division by zero");
            }

            value /= divisor;
        }
        else if (match('%')) {
            double divisor = parseUnary();

            if (divisor == 0.0) {
                throw std::runtime_error("Modulus by zero");
            }

            value = std::fmod(value, divisor);
        }
        else {
            break;
        }

        if (!std::isfinite(value)) {
            throw std::runtime_error("Mathematical result is out of range");
        }
    }

    return value;
}

double ExpressionParser::parseUnary() {
    if (match('+')) {
        return parseUnary();
    }

    if (match('-')) {
        return -parseUnary();
    }

    return parsePower();
}

double ExpressionParser::parsePower() {
    double base = parsePrimary();

    if (match('^')) {
        double exponent = parseUnary();
        base = std::pow(base, exponent);

        if (!std::isfinite(base)) {
            throw std::runtime_error("Invalid exponentiation");
        }
    }

    return base;
}

// Parse an identifier such as sin, sqrt, or log10.
std::string ExpressionParser::parseName() {
    skipSpaces();

    std::string name;

    while (pos < expression.size() &&
           std::isalnum(
               static_cast<unsigned char>(expression[pos]))) {
        name += expression[pos++];
    }

    return name;
}

double ExpressionParser::parseFunction(const std::string& name) {
    if (!match('(')) {
        throw std::runtime_error(
            "Expected '(' after function name: " + name);
    }

    double a = parseExpression();

    // Special case: mod accepts two arguments.
    if (name == "mod") {
        if (!match(',')) {
            throw std::runtime_error(
                "Expected ',' between mod arguments");
        }

        double b = parseExpression();

        if (!match(')')) {
            throw std::runtime_error(
                "Missing closing parenthesis in mod()");
        }

        if (b == 0.0) {
            throw std::runtime_error("Modulus by zero");
        }

        return std::fmod(a, b);
    }

    if (!match(')')) {
        throw std::runtime_error(
            "Missing closing parenthesis for " + name);
    }

    if (name == "sin") return std::sin(a);
    if (name == "cos") return std::cos(a);
    if (name == "tan") return std::tan(a);

    if (name == "asin") {
        if (a < -1 || a > 1)
            throw std::runtime_error("asin argument must be in [-1, 1]");
        return std::asin(a);
    }

    if (name == "acos") {
        if (a < -1 || a > 1)
            throw std::runtime_error("acos argument must be in [-1, 1]");
        return std::acos(a);
    }

    if (name == "atan") return std::atan(a);

    if (name == "ln") {
        if (a <= 0)
            throw std::runtime_error("ln requires a positive argument");
        return std::log(a);
    }

    if (name == "log" || name == "log10") {
        if (a <= 0)
            throw std::runtime_error("log requires a positive argument");
        return std::log10(a);
    }

    if (name == "exp") return std::exp(a);

    if (name == "sqrt") {
        if (a < 0)
            throw std::runtime_error("sqrt requires a non-negative argument");
        return std::sqrt(a);
    }

    if (name == "abs") return std::abs(a);

    throw std::runtime_error("Unknown function: " + name);
}

double ExpressionParser::parsePrimary() {
    skipSpaces();

    if (pos >= expression.size()) {
        throw std::runtime_error("Unexpected end of expression");
    }

    if (match('(')) {
        double value = parseExpression();

        if (!match(')')) {
            throw std::runtime_error("Missing closing parenthesis");
        }

        return value;
    }

    if (expression[pos] == 'x' &&
        (pos + 1 == expression.size() ||
         !std::isalnum(
             static_cast<unsigned char>(expression[pos + 1])))) {
        ++pos;
        return xValue;
    }

    if (expression.compare(pos, 2, "pi") == 0 &&
        (pos + 2 == expression.size() ||
         !std::isalnum(
             static_cast<unsigned char>(expression[pos + 2])))) {
        pos += 2;
        return PI;
    }

    if (expression[pos] == 'e' &&
        (pos + 1 == expression.size() ||
         !std::isalnum(
             static_cast<unsigned char>(expression[pos + 1])))) {
        ++pos;
        return std::exp(1.0);
    }

    if (std::isalpha(
            static_cast<unsigned char>(expression[pos]))) {
        std::string name = parseName();
        return parseFunction(name);
    }

    if (std::isdigit(
            static_cast<unsigned char>(expression[pos])) ||
        expression[pos] == '.') {
        std::size_t consumed = 0;

        double value = std::stod(
            expression.substr(pos), &consumed);

        pos += consumed;
        return value;
    }

    throw std::runtime_error(
        std::string("Unexpected character: ") + expression[pos]);
}

double ExpressionParser::evaluate() {
    double result = parseExpression();

    skipSpaces();

    if (pos != expression.size()) {
        throw std::runtime_error(
            "Unexpected text near: " + expression.substr(pos));
    }

    if (!std::isfinite(result)) {
        throw std::runtime_error("Result is not finite");
    }

    return result;
}

// FUNCTION OVERLOADS

double evaluateExpression(const std::string& expression, double x) {
    ExpressionParser parser(expression, x);
    return parser.evaluate();
}

double evaluateExpression(const std::string& expression) {
    return evaluateExpression(expression, 0.0);
}

// GRAPH IMPLEMENTATION

Graph::Graph() {
    initialize();
}

void Graph::initialize() {
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            screen[row][col] = ' ';
            isGraph[row][col] = false;
        }
    }
}

void Graph::drawAxes() {
    int centerRow = ROWS / 2;
    int centerCol = COLS / 2;

    for (int col = 0; col < COLS; ++col) {
        screen[centerRow][col] = '-';
    }

    for (int row = 0; row < ROWS; ++row) {
        screen[row][centerCol] = '|';
    }

    screen[centerRow][centerCol] = '+';
}

void Graph::putGraphPoint(int row, int col) {
    if (!isInsideScreen(row, col)) {
        return;
    }

    screen[row][col] = '#';
    isGraph[row][col] = true;
}

void Graph::plot(const std::string& function,
                 double xScale, double yScale) {
    plotGraph(function, *this, xScale, yScale);
}

void Graph::plot(const std::string& function) {
    plot(function, 4.0, 8.0);
}

// GRAPH PLOTTING HELPER

void plotGraph(const std::string& expression,
               Graph& graph,
               double xScale,
               double yScale) {
    if (xScale <= 0 || yScale <= 0) {
        throw std::invalid_argument("Scales must be positive");
    }

    graph.initialize();
    graph.expression = expression;
    graph.drawAxes();

    int centerRow = ROWS / 2;
    int centerCol = COLS / 2;

    bool previousValid = false;
    int previousRow = 0;

    for (int col = 0; col < COLS; ++col) {
        double x = (col - centerCol) / xScale;
        double y;

        try {
            y = evaluateExpression(expression, x);
        }
        catch (const std::exception&) {
            previousValid = false;
            continue;
        }

        int row = 0;

        if (!std::isfinite(y) ||
            !toScreenRow(y, yScale, centerRow, row)) {
            previousValid = false;
            continue;
        }

        if (previousValid) {
            int startRow = std::min(previousRow, row);
            int endRow = std::max(previousRow, row);

            // Do not connect large jumps across likely discontinuities.
            if (endRow - startRow <= ROWS / 4) {
                for (int r = startRow; r <= endRow; ++r) {
                    graph.putGraphPoint(r, col);
                }
            }
            else {
                graph.putGraphPoint(row, col);
            }
        }
        else {
            graph.putGraphPoint(row, col);
        }

        previousRow = row;
        previousValid = true;
    }
}

void plotGraph(const std::string& expression, Graph& graph) {
    plotGraph(expression, graph, 4.0, 8.0);
}

std::istream& operator>>(std::istream& in, Graph& graph) {
    std::string expression;

    if (std::getline(in >> std::ws, expression)) {
        graph.plot(expression);
    }

    return in;
}

std::ostream& operator<<(std::ostream& out, const Graph& graph) {
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (graph.isGraph[row][col]) {
                out << RED << graph.screen[row][col] << RESET;
            }
            else {
                out << graph.screen[row][col];
            }
        }

        out << '\n';
    }

    return out;
}