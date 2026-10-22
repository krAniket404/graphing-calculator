#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <cctype>
#include <stdexcept>

using namespace std;

const int ROWS = 50;
const int COLS = 50;
const string RED = "\033[31m";
const string RESET = "\033[0m";

class ExpressionParser {
private:
    std::string expr;
    size_t pos = 0;
    double x;

    void skipSpaces() {
        while (pos < expr.size() &&
               std::isspace(static_cast<unsigned char>(expr[pos]))) {
            ++pos;
        }
    }

    bool match(char c) {
        skipSpaces();

        if (pos < expr.size() && expr[pos] == c) {
            ++pos;
            return true;
        }

        return false;
    }

    double parseExpression() {
        double value = parseTerm();

        while (true) {
            if (match('+')) {
                value += parseTerm();
            } else if (match('-')) {
                value -= parseTerm();
            } else {
                break;
            }
        }

        return value;
    }

    double parseTerm() {
        double value = parseUnary();

        while (true) {
            if (match('*')) {
                value *= parseUnary();
            } else if (match('/')) {
                double divisor = parseUnary();

                if (divisor == 0.0) {
                    throw std::runtime_error("Division by zero");
                }

                value /= divisor;
            } else if (match('%')) {
                double divisor = parseUnary();

                if (divisor == 0.0) {
                    throw std::runtime_error("Modulus by zero");
                }

                value = std::fmod(value, divisor);
            } else {
                break;
            }
        }

        return value;
    }

    double parseUnary() {
        if (match('+')) {
            return parseUnary();
        }

        if (match('-')) {
            return -parseUnary();
        }

        return parsePower();
    }

    double parsePower() {
        double base = parsePrimary();

        if (match('^')) {
            double exponent = parseUnary();
            return checkedResult(std::pow(base, exponent));
        }

        return base;
    }

    double parseFunction() {
        skipSpaces();

        std::string name;

        while (pos < expr.size() &&
            std::isalnum(static_cast<unsigned char>(expr[pos]))) {
            name += expr[pos++];
        }

        if (name.empty()) {
            throw std::runtime_error("Expected a function name");
        }

        if (!match('(')) {
            throw std::runtime_error(
                "Expected '(' after function name: " + name
            );
        }

        double argument = parseExpression();

        if (name == "mod") {
            if (!match(',')) {
                throw std::runtime_error(
                    "Expected ',' between arguments of mod()"
                );
            }

            double secondArgument = parseExpression();

            if (!match(')')) {
                throw std::runtime_error(
                    "Missing closing parenthesis for mod()"
                );
            }

            if (secondArgument == 0.0) {
                throw std::runtime_error("Modulus by zero");
            }

            return std::fmod(argument, secondArgument);
        }

        if (!match(')')) {
            throw std::runtime_error(
                "Missing closing parenthesis for function: " + name
            );
        }

        if (name == "sin")   return std::sin(argument);
        if (name == "cos")   return std::cos(argument);
        if (name == "tan")   return std::tan(argument);

        if (name == "asin")  return std::asin(argument);
        if (name == "acos")  return std::acos(argument);
        if (name == "atan")  return std::atan(argument);

        if (name == "ln") {
            if (argument <= 0.0)
                throw std::runtime_error("ln() requires an argument greater than zero");

            return checkedResult(std::log(argument));
        }

        if (name == "log" || name == "log10") {
            if (argument <= 0.0)
                throw std::runtime_error("log() requires an argument greater than zero");

            return checkedResult(std::log10(argument));
        }

        if (name == "sqrt") {
            if (argument < 0.0)
                throw std::runtime_error("sqrt() requires a non-negative argument");

            return checkedResult(std::sqrt(argument));
        }

        if (name == "exp")   return std::exp(argument);
        if (name == "abs")   return std::abs(argument);

        throw std::runtime_error("Unknown function: " + name);
    }

    double parsePrimary() {
        skipSpaces();

        if (pos >= expr.size()) {
            throw std::runtime_error("Unexpected end of expression");
        }

        if (match('(')) {
            double value = parseExpression();

            if (!match(')')) {
                throw std::runtime_error("Missing closing parenthesis");
            }

            return value;
        }

        
        if (expr[pos] == 'x') {
            ++pos;
            return x;
        }

        if (expr.compare(pos, 2, "pi") == 0) {
            pos += 2;
            return std::acos(-1.0);
        }

        if (expr[pos] == 'e') {
            ++pos;
            return std::exp(1.0);
        }

        if (std::isalpha(static_cast<unsigned char>(expr[pos]))) {
            return parseFunction();
        }
        
        if (std::isdigit(static_cast<unsigned char>(expr[pos])) ||
            expr[pos] == '.') {
            size_t consumed = 0;

            try {
                double value = std::stod(expr.substr(pos), &consumed);
                pos += consumed;
                return value;
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid number");
            }
        }

        throw std::runtime_error(
            std::string("Unexpected character: ") + expr[pos]
        );
    }
    double checkedResult(double result) {
        if (!std::isfinite(result)) {
            throw std::runtime_error(
                "Mathematical error: result is undefined or out of range"
            );
        }

        return result;
    }
public:
    ExpressionParser(const std::string& expression, double xValue)
        : expr(expression), x(xValue) {}

    double evaluate() {
        double result = parseExpression();

        skipSpaces();

        if (pos != expr.size()) {
            throw std::runtime_error("Unexpected character in expression");
        }

        return result;
    }
};

double evaluateExpression(const std::string& expression, double x) {
    ExpressionParser parser(expression, x);
    return parser.evaluate();
}

void drawLine(char screen[ROWS][COLS],
              bool isGraph[ROWS][COLS],
              int row1, int col1,
              int row2, int col2) {

    int dx = col2 - col1;
    int dy = row2 - row1;

    int absDx = std::abs(dx);
    int absDy = std::abs(dy);

    if (absDx == 0 && absDy == 0) {
        if (row1 >= 0 && row1 < ROWS &&
            col1 >= 0 && col1 < COLS) {
            screen[row1][col1] = '/';
            isGraph[row1][col1] = true;
        }
        return;
    }

    if (absDx == 0) {
        int startRow = std::min(row1, row2);
        int endRow = std::max(row1, row2);

        for (int row = startRow; row <= endRow; row++) {
            if (row >= 0 && row < ROWS &&
                col1 >= 0 && col1 < COLS) {
                screen[row][col1] = '|';
                isGraph[row][col1] = true;
            }
        }
        return;
    }

    if (absDy == 0) {
        int startCol = std::min(col1, col2);
        int endCol = std::max(col1, col2);

        for (int col = startCol; col <= endCol; col++) {
            if (row1 >= 0 && row1 < ROWS &&
                col >= 0 && col < COLS) {
                screen[row1][col] = '_';
                isGraph[row1][col] = true;
            }
        }
        return;
    }

    int steps = std::max(absDx, absDy);

    for (int i = 0; i <= steps; i++) {
        int row = static_cast<int>(
            std::round(row1 + static_cast<double>(dy) * i / steps)
        );

        int col = static_cast<int>(
            std::round(col1 + static_cast<double>(dx) * i / steps)
        );

        if (row < 0 || row >= ROWS ||
            col < 0 || col >= COLS) {
            continue;
        }

        double slope = static_cast<double>(absDy) / absDx;

        char symbol;

        if (std::abs(slope - 1.0) < 1e-9) {
            symbol = (dy < 0) ? '/' : '\\';
        }
        else if (slope < 1.0) {
            symbol = (i % 2 == 0)
                ? ((dy < 0) ? '/' : '\\')
                : '_';
        }
        else {
            symbol = (i % 2 == 0)
                ? ((dy < 0) ? '/' : '\\')
                : '|';
        }

        screen[row][col] = symbol;
        isGraph[row][col] = true;
    }
}

double calculateY(double x, int choice) {
    switch (choice) {
        case 1:
            return x * x;

        case 2:
            return x;

        case 3:
            return -x;

        case 4:
            return x * x * x;

        default:
            return 0;
    }
}

int main() {
    string expression;
    double y;
    double yScale = 2.0;
    double xScale = 4.0;

    char screen[ROWS][COLS];
    bool isGraph[ROWS][COLS] = {};

    cout << "Enter a function of x (e.g., x*x + 2*x - 3): ";
    getline(cin, expression);

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            screen[i][j] = (j == COLS/2) ? '|' : ' ';
            if (i == ROWS/2)
            {
                screen[i][j] = '-';
            }
        }
    }

    screen[ROWS/2][COLS/2] = '+';

    bool previousValid = false;
    int previousRow = 0;
    int previousCol = 0;

    for (int col = 0; col < COLS; col++) {

        double x = (col - COLS / 2.0) / xScale;
        double y;

        try {
            y = evaluateExpression(expression, x);
        }
        catch (const std::exception&) {
            previousValid = false;
            continue;
        }

        if (!std::isfinite(y)) {
            previousValid = false;
            continue;
        }

        int row = static_cast<int>(
            std::round(ROWS / 2.0 - y * yScale)
        );

        // Ignore points outside the screen.
        if (row < 0 || row >= ROWS) {
            previousValid = false;
            continue;
        }

        if (previousValid) {
            int startRow = std::min(previousRow, row);
            int endRow = std::max(previousRow, row);

            // Fill the vertical gap in this column.
            for (int r = startRow; r <= endRow; r++) {
                screen[r][col] = '#';
                isGraph[r][col] = true;
            }
        }
        else {
            screen[row][col] = '#';
            isGraph[row][col] = true;
        }

        previousRow = row;
        previousValid = true;
    }
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (isGraph[i][j]) {
                cout << RED << screen[i][j] << RESET;
            } else {
                cout << screen[i][j];
            }
        }
        cout << '\n';
    }

    return 0;
}