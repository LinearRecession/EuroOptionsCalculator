#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
    double S; // Stock price
    double K; // Strike price
    double T; // Time to expiration (in years)
    double r; // Risk-free interest rate
    double sigma; // Volatility
    char type; // Option type c for call or p for put
} RequestOptionParameters;

typedef struct {
    double price;
    double delta;
    double gamma;
    double vega;
    double theta;
} ResponseOptionParameters;

RequestOptionParameters *init_option_parameters(double S, double K, double T, double r, double sigma, char type) {
    RequestOptionParameters *params = malloc(sizeof(RequestOptionParameters));
    if (params == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    params->S = S;
    params->K = K;
    params->T = T;
    params->r = r;
    params->sigma = sigma;
    params->type = type;
    return params;
}

// Cumulative normal distribution function  using Abramowitz and Stegun approximation
double cdf(double x) {
    double t = 1.0 / (1.0 + 0.3275911 * fabs(x));
    double y = 0.254829592 * t + 0.080788966 * t * t + 0.0003238188 * t * t * t;
    y = 0.39894228 * exp(-0.5 * x * x) * y;

    if (x >= 0.0) {
        return 0.5 + y;
    }
    return 0.5 - y;
}

double calculate_standard_normal_variable_stock(double S, double K, double T, double r, double sigma) {
    return (log(S / K) + (r + 0.5 * sigma * sigma) * T)
           / (sigma * sqrt(T));
}

double calculate_standard_normal_variable_strike(double d1, double T, double r, double sigma) {
    return d1 - sigma * sqrt(T);
}

double calculate_european_option(double d1, double d2, double S, double K, double T, double r, char type) {
    if (type == 'c') {
        return S * cdf(d1) - K * exp(-r * T) * cdf(d2);
    }

    return K * exp(-r * T) *
           cdf(-d2) - S
           * cdf(-d1);
}

double calculate_delta(double d1, char type) {
    int option_type = (type == 'c') ? 1 : -1;

    return option_type
           * cdf(option_type * d1);
}

double calculate_gamma(double d1, double S, double T, double sigma) {
    return exp(-0.5 * d1 * d1)
           / (S * sigma * sqrt(2 * M_PI * T));
}

double calculate_vega(double d1, double S, double T) {
    return S * sqrt(T) * exp(-0.5 * d1 * d1)
           / sqrt(2 * M_PI);
}

double calculate_theta(double d1, double d2, double S, double K, double T, double r, double sigma, char type) {
    int option_type = (type == 'c') ? 1 : -1;

    return -0.5 * sigma * S * exp(-0.5 * d1 * d1)
           / sqrt(2 * M_PI * T) - option_type
           * r * K * exp(-r * T) * cdf(
               option_type * d2);
}

ResponseOptionParameters *calculateOption(RequestOptionParameters *request) {
    ResponseOptionParameters *response = malloc(sizeof(ResponseOptionParameters));
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    double d1 = calculate_standard_normal_variable_stock(request->S, request->K, request->T, request->r,
                                                         request->sigma);
    double d2 = calculate_standard_normal_variable_strike(d1, request->T, request->r, request->sigma);

    response->price = calculate_european_option(d1, d2, request->S, request->K, request->T, request->r, request->type);
    response->delta = calculate_delta(d1, request->type);
    response->theta = calculate_theta(d1, d2, request->S, request->K, request->T, request->r, request->sigma,
                                      request->type);
    response->gamma = calculate_gamma(d1, request->S, request->T, request->sigma);
    response->vega = calculate_vega(d1, request->S, request->T);

    return response;
}

void displayOption(ResponseOptionParameters *response) {
    printf("\n");
    printf("Option Parameters:\n");
    printf("Price: %.2f\n", response->price);
    printf("Delta: %.2f\n", response->delta);
    printf("Gamma: %.2f\n", response->gamma);
    printf("Vega: %.2f\n", response->vega);
    printf("Theta: %.2f\n", response->theta);
    printf("\n");
}

int validate_input(const char *input, double *value) {
    char *endptr;
    *value = strtod(input, &endptr);
    return endptr != input && *endptr == '\0';
}

double get_double_input(char *message, double *value) {
    char input[100];

    while (1) {
        printf(message);
        scanf("%s", input);
        if (validate_input(input, value)) break;
        printf("Invalid input. Please enter a valid double number.\n");
    }
}

void get_char_input(char *message, char *value) {
    while (1) {
        printf("%s", message);
        scanf(" %c", value);
        if (*value == 'c' || *value == 'p') break;
        printf("Invalid input. Please enter 'c' for call or 'p' for put.\n");
    }
}

void get_user_input(double *S, double *K, double *T, double *r, double *sigma, char *type) {
    get_double_input("Enter Stock price (S): ", S);
    get_double_input("Enter Strike price (K): ", K);
    get_double_input("Enter Time to expiration (T): ", T);
    get_double_input("Enter Risk-free interest rate (r): ", r);
    get_double_input("Enter Volatility (sigma): ", sigma);
    get_char_input("Enter Option type (c for call, p for put): ", type);
}

int main() {
    while (1) {
        double S, K, T, r, sigma;
        char type;
        get_user_input(&S, &K, &T, &r, &sigma, &type);

        RequestOptionParameters *request = init_option_parameters(S, K, T, r, sigma, type);
        ResponseOptionParameters *response = calculateOption(request);

        displayOption(response);

        free(request);
        free(response);
    }
}
