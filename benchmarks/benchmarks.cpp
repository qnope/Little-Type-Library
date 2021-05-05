#include <benchmark/benchmark.h>

#include <random>

#include <ltl/algos.h>
#include <ltl/functional.h>

#include <ltl/Range/Map.h>
#include <ltl/Range/Filter.h>
#include <ltl/Range/actions.h>

#include <ltl/expected.h>

using namespace ltl;

#define RANGE ->Args({10000, false})->Args({10000, true})->Args({100'000, false})->Args({100'000, true});

std::vector<std::size_t> createArray(int64_t count, bool sorted) {
    std::vector<std::size_t> a;
    for (int i = 0; i < count; ++i)
        a.push_back(i);

    if (!sorted) {
        std::mt19937 m;
        shuffle(a, m);
    }

    return a;
}

static void sum_square_normal(benchmark::State &state) {
    auto vector = createArray(state.range(0), state.range(1));

    for (auto _ : state) {
        std::size_t result = 0;
        for (auto x : vector) {
            result += x * x;
        }
        benchmark::DoNotOptimize(result);
    }
}

static void sum_square_range(benchmark::State &state) {
    auto vector = createArray(state.range(0), state.range(1));

    for (auto _ : state) {
        auto square = [](auto &&x) { return x * x; };
        benchmark::DoNotOptimize(vector | map(square) | actions::sum);
    }
}

static void sum_square_odd_normal(benchmark::State &state) {
    auto vector = createArray(state.range(0), state.range(1));

    for (auto _ : state) {
        std::size_t result = 0;
        for (auto x : vector) {
            if (x % 2 == 1)
                result += x * x;
        }
        benchmark::DoNotOptimize(result);
    }
}

static void sum_square_odd_range(benchmark::State &state) {
    auto vector = createArray(state.range(0), state.range(1));

    for (auto _ : state) {
        auto is_odd = [](auto &&x) { return x % 2 == 1; };
        auto square = [](auto &&x) { return x * x; };
        benchmark::DoNotOptimize(vector | filter(is_odd) | map(square) | actions::sum);
    }
}

static void sum_filter_single(benchmark::State &state) {
    auto vector = createArray(state.range(0), state.range(1));

    for (auto _ : state) {
        auto predicate = [](auto x) { return (x % 2 == 1) && (x % 154 == 0); };
        benchmark::DoNotOptimize(vector | filter(predicate) | actions::sum);
    }
}

static void sum_filter_single_and(benchmark::State &state) {
    auto vector = createArray(state.range(0), state.range(1));

    for (auto _ : state) {
        auto predicate1 = [](auto x) { return x % 2 == 1; };
        auto predicate2 = [](auto x) { return x % 154 == 0; };
        benchmark::DoNotOptimize(vector | filter(ltl::and_(predicate1, predicate2)) | actions::sum);
    }
}

static void sum_filter_double(benchmark::State &state) {
    auto vector = createArray(state.range(0), state.range(1));

    for (auto _ : state) {
        auto predicate1 = [](auto x) { return x % 2 == 1; };
        auto predicate2 = [](auto x) { return x % 154 == 0; };
        benchmark::DoNotOptimize(vector | filter(predicate1) | filter(predicate2) | actions::sum);
    }
}

static ltl::expected<int, const char *> fExpected(bool success) {
    if (!success)
        return "Error";
    return 10;
}

static ltl::expected<double, const char *> gExpected(bool success) {
    return fExpected(success).map([](int x) { //
        return x * 1.5;
    });
}

static void expected_result(benchmark::State &state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(gExpected(true));
    }
}

static void expected_error(benchmark::State &state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(gExpected(false));
    }
}

static int fException(bool success) {
    if (!success)
        throw "Error";
    return 10;
}

static double gException(bool success) { return fException(success) * 1.5; }

static void exception_result(benchmark::State &state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(gException(true));
    }
}

static void exception_error(benchmark::State &state) {
    for (auto _ : state) {
        try {
            benchmark::DoNotOptimize(gException(false));
        } catch (const char *) {
        }
    }
}

// Register the function as a benchmark
BENCHMARK(sum_square_normal) RANGE;
BENCHMARK(sum_square_range) RANGE;

BENCHMARK(sum_square_odd_normal) RANGE;
BENCHMARK(sum_square_odd_range) RANGE;

BENCHMARK(sum_filter_single) RANGE;
BENCHMARK(sum_filter_single_and) RANGE;
BENCHMARK(sum_filter_double) RANGE;

BENCHMARK(expected_result);
BENCHMARK(exception_result);
BENCHMARK(expected_error);
BENCHMARK(exception_error);

// Run the benchmark
BENCHMARK_MAIN();
